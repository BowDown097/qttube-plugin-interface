#include "webauthroutine.h"
#include <QMessageBox>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineView>

namespace QtTubePlugin
{
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
        if (auto it = std::ranges::find_if(m_searchCookies, [&](const auto& p) { return p.first == cookie; });
            it != m_searchCookies.end())
        {
            it->second = cookie.value();
            onNewCookie(cookie.name(), cookie.value());
        }

        checkAndEmitSuccess();
    }

    bool WebAuthRoutine::nothingToSearch() const
    {
        #define ALL_FOUND(map) std::ranges::none_of(map, [](const auto& p) { return p.second.isEmpty(); })
    #if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        return ALL_FOUND(m_searchCookies) && ALL_FOUND(m_searchHeaders.asKeyValueRange());
    #else
        return ALL_FOUND(m_searchCookies);
    #endif
    }

    QHash<QByteArray, QByteArray> WebAuthRoutine::searchCookies() const
    {
        QHash<QByteArray, QByteArray> out;
        out.reserve(m_searchCookies.size());

        for (const auto& [searchCookie, value] : m_searchCookies)
            out.emplace(searchCookie.name, value);

        return out;
    }

    QHash<QByteArray, QByteArray> WebAuthRoutine::searchHeaders() const
    {
    #if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        return m_searchHeaders;
    #else
        return {};
    #endif
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

    #if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        QList<QByteArray> headerKeys;
        headerKeys.reserve(m_searchHeaders.size());
        for (auto it = m_searchHeaders.begin(); it != m_searchHeaders.end(); ++it)
            headerKeys.append(it.key());

        m_interceptor = new WebAuthRequestInterceptor(headerKeys, this);
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
            it.value() = value;
            onNewHeader(name, value);
        }

        checkAndEmitSuccess();
    }

    void WebAuthRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info)
    {
        if (!m_searchHeaders.isEmpty())
        {
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
    }
#endif
}
