#include "webauthroutine.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include <unordered_set>

struct QNetworkCookieHash
{
    size_t operator()(const QNetworkCookie& v) const
    { return qHashMulti(0, v.name(), v.domain(), v.path()); }
};

struct QNetworkCookieEqual
{
    bool operator()(const QNetworkCookie& lhs, const QNetworkCookie& rhs) const
    { return lhs.hasSameIdentifier(rhs); }
};

namespace QtTubePlugin
{
    WebAuthRoutine::WebAuthRoutine(AuthStoreBase* authStore)
        : AuthRoutine(authStore),
          m_authDebug(QCoreApplication::arguments().contains("--auth-debug")) {}

    void WebAuthRoutine::checkAndEmitSuccess()
    {
        m_mutex.lock();
        if (!m_successEmitted && nothingToSearch())
        {
            m_successEmitted = true;
            m_mutex.unlock();
            m_loginWindow->hide();
            m_loginWindow->deleteLater();
            emit success();
        }
        else
        {
            m_mutex.unlock();
        }
    }

    void WebAuthRoutine::cookieAdded(const QNetworkCookie& cookie)
    {
        if (m_authDebug)
        {
            static std::unordered_set<QNetworkCookie, QNetworkCookieHash, QNetworkCookieEqual> seen;
            if (auto [_, inserted] = seen.insert(cookie); inserted)
            {
                qDebug().noquote().nospace() << "Cookie: "
                    << "name=" << cookie.name() << ' '
                    << "domain=" << cookie.domain() << ' '
                    << "path=" << cookie.path();
            }
        }

        auto it = std::ranges::find_if(m_searchCookies, [&](const auto& p) { return p.first == cookie; });
        if (it != m_searchCookies.end())
        {
            it->second = cookie.value();
            onNewCookie(cookie.name(), cookie.value());
        }

        checkAndEmitSuccess();
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    void WebAuthRoutine::foundHeader(const QByteArray& name, const QByteArray& value)
    {
        if (auto it = m_searchHeaders.find(name); it != m_searchHeaders.end())
        {
            it->second = value;
            onNewHeader(name, value);
        }

        checkAndEmitSuccess();
    }
#endif

    bool WebAuthRoutine::nothingToSearch() const
    {
        #define ALL_FOUND(map) std::ranges::none_of(map, [](const auto& p) { return p.second.isEmpty(); })
        return ALL_FOUND(m_searchCookies) && ALL_FOUND(m_searchHeaders);
    }

    std::unordered_map<QByteArray, QByteArray> WebAuthRoutine::searchCookies() const
    {
        std::unordered_map<QByteArray, QByteArray> out;
        out.reserve(m_searchCookies.size());
        for (const auto& [cookie, value] : m_searchCookies)
            out.emplace(cookie.name, value);
        return out;
    }

    void WebAuthRoutine::setLoginButton(const QString& loginButton)
    {
        m_loginButton = loginButton;
        m_loginButton.replace("\\", "\\\\").replace("\"", "\\\"");
    }

    void WebAuthRoutine::setSearchCookies(const QList<SearchCookie>& cookies)
    {
        m_searchCookies.reserve(cookies.size());
        for (const SearchCookie& cookie : cookies)
            m_searchCookies.emplace_back(cookie, QByteArray());
    }

    void WebAuthRoutine::setSearchHeaders(const QList<QByteArray>& headers)
    {
    #if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        m_searchHeaders.reserve(headers.size());
        for (const QByteArray& header : headers)
            m_searchHeaders.emplace(header, QByteArray());
    #endif
    }

    void WebAuthRoutine::start()
    {
        if (m_url.isEmpty() || nothingToSearch())
        {
            QMessageBox::critical(nullptr, "Cannot Authenticate", "Authentication routine is misconfigured. Either a URL has not been set or there is nothing to capture.");
            return;
        }

        m_loginWindow = new QWidget;
        m_loginWindow->setFixedSize(m_loginWindow->size());
        m_loginWindow->setWindowTitle("Login Window");
        m_loginWindow->show();

        QWebEngineView* view = new QWebEngineView(m_loginWindow);
        view->setFixedSize(m_loginWindow->size());

        QWebEngineProfile* profile = new QWebEngineProfile(view);
        QWebEnginePage* page = new QWebEnginePage(profile, view);
        view->setPage(page);

        view->load(m_url);
        view->show();

        if (!m_loginButton.isEmpty())
        {
            connect(view, &QWebEngineView::loadFinished, this, [this, page](bool ok) {
                if (!ok)
                    return;

                page->runJavaScript(QStringLiteral(R"(
                    const selector = "%1";

                    function isClickable(el) {
                        if (!el || el.disabled)
                            return false;

                        const style = window.getComputedStyle(el);
                        return style.display !== "none" && style.visibility !== "hidden" && parseFloat(style.opacity) > 0;
                    }

                    const interval = setInterval(() => {
                        const el = document.querySelector(selector);
                        if (isClickable(el)) {
                            clearInterval(interval);
                            el.click();
                        }
                    }, 500);

                    setTimeout(() => clearInterval(interval), 30000);
                )").arg(m_loginButton));
            });
        }

    #if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        if (!m_searchHeaders.empty())
        {
            m_interceptor = new WebAuthRequestInterceptor(this);
            profile->setUrlRequestInterceptor(m_interceptor);
            connect(m_interceptor, &WebAuthRequestInterceptor::foundHeader,
                    this, &WebAuthRoutine::foundHeader,
                    Qt::QueuedConnection);
        }
    #endif

        connect(profile->cookieStore(), &QWebEngineCookieStore::cookieAdded,
                this, &WebAuthRoutine::cookieAdded,
                Qt::QueuedConnection);
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    void WebAuthRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info)
    {
        QHash<QByteArray, QByteArray> httpHeaders = info.httpHeaders();
        std::unordered_map<QByteArray, QByteArray>& searchHeaders = m_routine->m_searchHeaders;

        if (m_routine->m_authDebug)
        {
            static std::unordered_set<QByteArray> seen;
            for (const auto& [name, value] : httpHeaders.asKeyValueRange())
                if (auto [_, inserted] = seen.insert(name); inserted)
                    qDebug().noquote().nospace() << "Header: " << name << '=' << value;
        }

        for (const auto& [name, value] : searchHeaders)
            if (auto it = httpHeaders.find(name); it != httpHeaders.end())
                emit foundHeader(it.key(), it.value());
    }
#endif
}
