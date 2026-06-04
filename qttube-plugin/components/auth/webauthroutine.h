#pragma once
#include "authroutine.h"
#include <QHash>
#include <QMutex>
#include <QNetworkCookie>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QWebEngineUrlRequestInterceptor>
#endif

namespace QtTubePlugin
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    class WebAuthRequestInterceptor : public QWebEngineUrlRequestInterceptor
    {
        Q_OBJECT
    public:
        WebAuthRequestInterceptor(const QList<QByteArray>& searchHeaders, QObject* parent = nullptr)
            : QWebEngineUrlRequestInterceptor(parent), m_searchHeaders(searchHeaders) {}
        void interceptRequest(QWebEngineUrlRequestInfo& info) override;
    private:
        QList<QByteArray> m_searchHeaders;
    signals:
        void foundHeader(const QByteArray& key, const QByteArray& value);
    };
#endif

    struct SearchCookie
    {
        QByteArray name;
        QString domain;
        QString path;

        bool operator==(const QNetworkCookie& rhs) const;
    };

    class WebAuthRoutine : public AuthRoutine
    {
        Q_OBJECT
    public:
        using AuthRoutine::AuthRoutine;

        std::unordered_map<QByteArray, QByteArray> searchCookies() const;
        void setSearchCookies(const QList<SearchCookie>& cookies);
        virtual void onNewCookie(const QByteArray& name, const QByteArray& value) {}

        // header functions are no-op until Qt 6.5
        const std::unordered_map<QByteArray, QByteArray>& searchHeaders() const;
        void setSearchHeaders(const QList<QByteArray>& headers);
        virtual void onNewHeader(const QByteArray& name, const QByteArray& value) {}

        // Set the CSS query for the login button to be clicked on page load, if needed.
        void setLoginButton(const QString& loginButton);

        void setUrl(const QUrl& url) { m_url = url; }
        void start() override;
    protected:
        QString m_loginButton;
        QList<std::pair<SearchCookie, QByteArray>> m_searchCookies;
        std::unordered_map<QByteArray, QByteArray> m_searchHeaders;
        QUrl m_url;
    private:
        QWidget* m_loginWindow{};
        QMutex m_searchCheckMutex;
        bool m_successEmitted{};

        void checkAndEmitSuccess();
        bool nothingToSearch() const;
    private slots:
        void cookieAdded(const QNetworkCookie& cookie);

    // Qt does not provide any way to intercept headers until Qt 6.5
    #if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    private:
        WebAuthRequestInterceptor* m_interceptor;
    private slots:
        void foundHeader(const QByteArray& name, const QByteArray& value);
    #endif
    };
}
