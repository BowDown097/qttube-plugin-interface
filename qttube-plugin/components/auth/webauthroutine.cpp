#include "webauthroutine.h"
#include <QMessageBox>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineView>

namespace QtTubePlugin
{
    bool SearchCookie::operator==(const QNetworkCookie& rhs) const
    {
        return (name == rhs.name()) &&
               (domain.isEmpty() || domain == rhs.domain()) &&
               (path.isEmpty() || path == rhs.path());
    }

    void WebAuthRoutine::checkAndEmitSuccess()
    {
        m_searchCheckMutex.lock();
        if (!m_successEmitted && nothingToSearch())
        {
            m_successEmitted = true;
            m_searchCheckMutex.unlock();
            m_loginWindow->hide();
            m_loginWindow->deleteLater();
            emit success();
        }
        else
        {
            m_searchCheckMutex.unlock();
        }
    }

    void WebAuthRoutine::cookieAdded(const QNetworkCookie& cookie)
    {
        auto it = std::ranges::find_if(m_searchCookies, [&](const auto& p) { return p.first == cookie; });
        if (it != m_searchCookies.end())
        {
            it->second = cookie.value();
            onNewCookie(cookie.name(), cookie.value());
        }

        checkAndEmitSuccess();
    }

    bool WebAuthRoutine::nothingToSearch() const
    {
        #define ALL_FOUND(map) std::ranges::none_of(map, [](const auto& p) { return p.second.isEmpty(); })
        return ALL_FOUND(m_searchCookies) && ALL_FOUND(m_searchHeaders);
    }

    std::unordered_map<QByteArray, QByteArray> WebAuthRoutine::searchCookies() const
    {
        std::unordered_map<QByteArray, QByteArray> out;
        out.reserve(m_searchCookies.size());
        for (const auto& [searchCookie, value] : m_searchCookies)
            out.emplace(searchCookie.name, value);
        return out;
    }

    const std::unordered_map<QByteArray, QByteArray>& WebAuthRoutine::searchHeaders() const
    {
        return m_searchHeaders;
    }

    void WebAuthRoutine::setLoginButton(const QString& loginButton)
    {
        m_loginButton = loginButton;
        m_loginButton.replace("\\", "\\\\").replace("\"", "\\\"");
    }

    void WebAuthRoutine::setSearchCookies(const QList<SearchCookie>& cookies)
    {
        for (const SearchCookie& searchCookie : cookies)
            m_searchCookies.emplaceBack(searchCookie, QByteArray());
    }

    void WebAuthRoutine::setSearchHeaders(const QList<QByteArray>& headers)
    {
    #if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
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
        auto keys = m_searchHeaders | std::views::keys;
        m_interceptor = new WebAuthRequestInterceptor(QList(keys.begin(), keys.end()), this);
        profile->setUrlRequestInterceptor(m_interceptor);
        connect(m_interceptor, &WebAuthRequestInterceptor::foundHeader,
                this, &WebAuthRoutine::foundHeader,
                Qt::QueuedConnection);
    #endif

        connect(profile->cookieStore(), &QWebEngineCookieStore::cookieAdded,
                this, &WebAuthRoutine::cookieAdded,
                Qt::QueuedConnection);
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

    void WebAuthRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info)
    {
        if (m_searchHeaders.isEmpty())
            return;

        QHash<QByteArray, QByteArray> headers = info.httpHeaders();
        for (auto it = m_searchHeaders.begin(); it != m_searchHeaders.end();)
        {
            if (auto it2 = headers.find(*it); it2 != headers.end())
            {
                emit foundHeader(*it, it2.value());
                it = m_searchHeaders.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
#endif
}
