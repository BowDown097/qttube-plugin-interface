#pragma once
#include "authroutine.h"
#include <QHashFunctions>
#include <QMutex>
#include <QNetworkCookie>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QWebEngineUrlRequestInterceptor>
#endif

namespace QtTubePlugin
{
    class SearchCookie
    {
    public:
        QByteArray name;
        QString domain;
        QString path;

        friend bool operator==(const SearchCookie& lhs, const SearchCookie& rhs)
        { return lhs.name == rhs.name && match(lhs.domain, rhs.domain) && match(lhs.path, rhs.path); }
        friend bool operator==(const SearchCookie& lhs, const QNetworkCookie& rhs)
        { return lhs.name == rhs.name() && match(lhs.domain, rhs.domain()) && match(lhs.path, rhs.path()); }
    private:
        template<typename T>
        static bool match(const T& lhs, const T& rhs) { return lhs.isEmpty() || lhs == rhs; }
    };

    class WebAuthRoutine : public AuthRoutine
    {
        friend class WebAuthRequestInterceptor;
        Q_OBJECT
    public:
        explicit WebAuthRoutine(AuthStoreBase* authStore);

        /** Get all found cookies that were given to this routine. */
        std::unordered_map<QByteArray, QByteArray> searchCookies() const;

        /** Set cookies to search for in all web requests made in this routine. */
        void setSearchCookies(const QList<SearchCookie>& cookies);

        /**
         * @brief Called when a new search cookie is found.
         * @return Whether or not to reject the given cookie and continue searching.
         */
        virtual bool onNewCookie(const QByteArray& name, const QByteArray& value) { return true; }

        /**
         * @brief Get all found headers that were given to this routine.
         * @note It is not possible to search for headers in Qt versions less than 6.5.
         */
        const std::unordered_map<QByteArray, QByteArray>& searchHeaders() const { return m_searchHeaders; }

        /**
         * @brief Set headers to search for in all web requests made in this routine.
         * @note It is not possible to search for headers in Qt versions less than 6.5.
         */
        void setSearchHeaders(const QList<QByteArray>& headers);

        /**
         * @brief Called when a new search header is found.
         * @return Whether or not to reject the given header and continue searching.
         * @note It is not possible to search for headers in Qt versions less than 6.5.
         */
        virtual bool onNewHeader(const QByteArray& name, const QByteArray& value) { return true; }

        /** Set the CSS query for the login button to be clicked on page load, if needed. */
        void setLoginButton(const QString& loginButton);

        void setUrl(const QUrl& url) { m_url = url; }
        void start() override;
    protected:
        QString m_loginButton;
        std::vector<std::pair<SearchCookie, QByteArray>> m_searchCookies;
        std::unordered_map<QByteArray, QByteArray> m_searchHeaders;
        QUrl m_url;
    private:
        QWidget* m_loginWindow{};
        QMutex m_mutex;
        bool m_authDebug;
        bool m_successEmitted{};

        void checkAndEmitSuccess();
        bool nothingToSearch() const;
    private slots:
        void cookieAdded(const QNetworkCookie& cookie);

    // Qt does not provide any way to intercept headers until Qt 6.5
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    private:
        class WebAuthRequestInterceptor* m_interceptor;
    private slots:
        void foundHeader(const QByteArray& name, const QByteArray& value);
    };

    class WebAuthRequestInterceptor : public QWebEngineUrlRequestInterceptor
    {
        Q_OBJECT
    public:
        explicit WebAuthRequestInterceptor(WebAuthRoutine* routine)
            : QWebEngineUrlRequestInterceptor(routine), m_routine(routine) {}
        void interceptRequest(QWebEngineUrlRequestInfo& info) override;
    private:
        WebAuthRoutine* m_routine;
    signals:
        void foundHeader(const QByteArray& key, const QByteArray& value);
    };
#else
    };
#endif
}
