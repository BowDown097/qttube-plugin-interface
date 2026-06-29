#pragma once
#include <QNetworkReply>

class HttpReply : public QObject
{
    Q_OBJECT
    friend class HttpRequest;
public:
    using HeaderList = QList<QNetworkReply::RawHeaderPair>;

    enum Operation
    {
        UnknownOperation,
        HeadOperation,
        GetOperation,
        PutOperation,
        PostOperation,
        DeleteOperation,
    };

    // if you just want the QNetworkAccessManager that this uses, because it does have some extra goodies
    static QNetworkAccessManager* networkAccessManager();

    QVariant attribute(QNetworkRequest::Attribute code) const { return networkReply()->attribute(code); }
    QNetworkReply::NetworkError error() const { return networkReply()->error(); }
    QString errorString() const { return networkReply()->errorString(); }
    QByteArray header(const QByteArray& key) const { return networkReply()->rawHeader(key); }
    QByteArray header(QNetworkRequest::KnownHeaders header) const { return networkReply()->header(header).toByteArray(); }
    const HeaderList& headers() const { return networkReply()->rawHeaderPairs(); }
    bool isSuccessful() const { return statusCode() >= 200 && statusCode() < 300; }
    QByteArray readAll() const { return m_sink->readAll(); }
    const HeaderList& requestHeaders() const { return m_requestHeaders; }
    int statusCode() const { return networkReply()->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(); }
    const QUrl& url() const { return m_url; }

    QString getFileName() const;
    QByteArray requestHeader(const QByteArray& headerName) const;
private:
    struct HttpSink
    {
        virtual ~HttpSink() = default;
        virtual QByteArray readAll() = 0;
        virtual void write(const QByteArray& chunk) = 0;
    };

    struct ByteArraySink : HttpSink
    {
        QByteArray data;
        QByteArray readAll() override { return data; }
        void write(const QByteArray& chunk) override { data.append(chunk); }
    };

    struct IODeviceSink : HttpSink
    {
        QIODevice* device;
        explicit IODeviceSink(QIODevice* device) : device(device) {}
        QByteArray readAll() override { return device->readAll(); }
        void write(const QByteArray& chunk) override { device->write(chunk); }
    };

    HeaderList m_requestHeaders;
    std::unique_ptr<HttpSink> m_sink;
    QUrl m_url;

    explicit HttpReply(const QUrl& url, HeaderList requestHeaders, std::unique_ptr<HttpSink> sink)
        : m_requestHeaders(std::move(requestHeaders)),
          m_sink(std::move(sink)),
          m_url(url) {}
    QNetworkReply* networkReply() const { return qobject_cast<QNetworkReply*>(parent()); }
private slots:
    void onReadyRead(QNetworkReply* reply) { m_sink->write(reply->readAll()); }
signals:
    void errorOccurred(QNetworkReply::NetworkError error);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void finished(const HttpReply& reply);
};

class HttpRequest
{
public:
    HttpRequest& withAttribute(QNetworkRequest::Attribute code, const QVariant& value)
    { m_attributes.emplaceBack(code, value); return *this; }
    HttpRequest& withDiskCache(bool usingDiskCache = true)
    { m_usingDiskCache = usingDiskCache; return *this; }
    HttpRequest& withHeader(const QByteArray& name, const QByteArray& value)
    { m_headers.emplaceBack(name, value); return *this; }
    HttpRequest& withHeaders(const HttpReply::HeaderList& headers)
    { m_headers = headers; return *this; }
    HttpRequest& withUserAgentSpoofing(bool spoofUserAgent = true)
    { m_spoofUserAgent = spoofUserAgent; return *this; }
    HttpRequest& writingToIODevice(QIODevice* ioDevice)
    { m_sink.reset(new HttpReply::IODeviceSink(ioDevice)); return *this; }

    static QNetworkReply* resolveNetworkReply(
        const QNetworkRequest& request, HttpReply::Operation operation, const QByteArray& data = {});
    static QNetworkReply* resolveNetworkReply(
        const QNetworkRequest& request, const QByteArray& verb, const QByteArray& data = {});

    HttpReply* request(const QUrl& url, HttpReply::Operation operation, const QByteArray& data = {})
    { return requestVariant(url, operation, data); }
    HttpReply* request(const QUrl& url, const QByteArray& verb, const QByteArray& data = {})
    { return requestVariant(url, verb, data); }

    HttpReply* deleteResource(const QUrl& url) { return request(url, HttpReply::DeleteOperation); }
    HttpReply* get(const QUrl& url) { return request(url, HttpReply::GetOperation); }
    HttpReply* head(const QUrl& url) { return request(url, HttpReply::HeadOperation); }
    HttpReply* post(const QUrl& url, const QByteArray& data) { return request(url, HttpReply::PostOperation, data); }
    HttpReply* put(const QUrl& url, const QByteArray& data) { return request(url, HttpReply::PutOperation, data); }
private:
    QList<std::pair<QNetworkRequest::Attribute, QVariant>> m_attributes;
    HttpReply::HeaderList m_headers;
    std::unique_ptr<HttpReply::HttpSink> m_sink = std::make_unique<HttpReply::ByteArraySink>();
    bool m_spoofUserAgent{};
    bool m_usingDiskCache{};

    HttpReply* requestVariant(
        const QUrl& url,
        const std::variant<HttpReply::Operation, QByteArray>& var,
        const QByteArray& data);
};
