#include "httprequest.h"
#include <QNetworkDiskCache>
#include <QStandardPaths>

// https://tools.ietf.org/html/rfc6266
QString HttpReply::getFileName() const
{
    if (QNetworkReply* reply = qobject_cast<QNetworkReply*>(parent());
        reply && reply->hasRawHeader("Content-Disposition"))
    {
        QString fileName;

        QByteArray disposition = reply->rawHeader("Content-Disposition");
        constexpr std::array keys = { QLatin1String("filename="), QLatin1String("filename*=UTF-8''") };

        for (const QLatin1String& key : keys)
        {
            if (int index = disposition.indexOf(key); index >= 0)
            {
                disposition = disposition.mid(index + key.size());
                if (disposition.startsWith('"') || disposition.startsWith('\''))
                    disposition = disposition.mid(1);
                if ((index = disposition.lastIndexOf('"')) > 0)
                    disposition = disposition.left(index);
                else if ((index = disposition.lastIndexOf('\'')) > 0)
                    disposition = disposition.left(index);
                fileName = QUrl::fromPercentEncoding(disposition);
            }
        }

        return fileName;
    }

    return {};
}

QNetworkAccessManager* HttpReply::networkAccessManager()
{
    static thread_local QNetworkAccessManager* nam = [] {
        QNetworkAccessManager* nam = new QNetworkAccessManager;
        nam->setAutoDeleteReplies(true);
        nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

        QNetworkDiskCache* diskCache = new QNetworkDiskCache(nam);
        diskCache->setCacheDirectory(
            QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/http/"));
        nam->setCache(diskCache);

        return nam;
    }();

    return nam;
}

QByteArray HttpReply::requestHeader(const QByteArray& headerName) const
{
    QByteArray result;
    const char* separator = "";

    for (const auto& [name, value] : m_requestHeaders)
    {
        if (name.compare(headerName, Qt::CaseInsensitive) == 0)
        {
            result.append(separator);
            result.append(value);
            separator = "\n";
        }
    }

    return result;
}

HttpReply* HttpRequest::requestVariant(
    const QUrl& url,
    const std::variant<HttpReply::Operation, QByteArray>& var,
    const QByteArray& data)
{
    HttpReply* result = new HttpReply(url, std::move(m_headers), std::move(m_sink));

    QNetworkRequest req(result->m_url);
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    req.setAttribute(QNetworkRequest::CacheSaveControlAttribute, m_usingDiskCache);

    if (m_spoofUserAgent)
        req.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/148.0.0.0 Safari/537.36");

    for (const auto& [code, value] : std::as_const(m_attributes))
        req.setAttribute(code, value);
    for (const auto& [name, value] : std::as_const(result->m_requestHeaders))
        req.setRawHeader(name, value);

    if (QNetworkReply* reply = std::visit([&](auto&& v) { return resolveNetworkReply(req, v, data); }, var))
    {
        result->setParent(reply);
        QObject::connect(reply, &QNetworkReply::downloadProgress, result, &HttpReply::downloadProgress);
        QObject::connect(reply, &QNetworkReply::errorOccurred, result, &HttpReply::errorOccurred);
        QObject::connect(reply, &QNetworkReply::finished, result, [=] { emit result->finished(*result); });
        QObject::connect(reply, &QNetworkReply::readyRead, result, [=] { result->onReadyRead(reply); });
    }

    return result;
}

QNetworkReply* HttpRequest::resolveNetworkReply(
    const QNetworkRequest& request, HttpReply::Operation operation, const QByteArray& data)
{
    switch (operation)
    {
    case HttpReply::HeadOperation:
        return HttpReply::networkAccessManager()->head(request);
    case HttpReply::GetOperation:
        return HttpReply::networkAccessManager()->get(request);
    case HttpReply::PutOperation:
        return HttpReply::networkAccessManager()->put(request, data);
    case HttpReply::PostOperation:
        return HttpReply::networkAccessManager()->post(request, data);
    case HttpReply::DeleteOperation:
        return HttpReply::networkAccessManager()->deleteResource(request);
    default:
        throw std::runtime_error("Making request with invalid operation (value: " + std::to_string(operation));
    }
}

QNetworkReply* HttpRequest::resolveNetworkReply(
    const QNetworkRequest& request, const QByteArray& verb, const QByteArray& data)
{
    if (verb.compare("GET", Qt::CaseInsensitive) == 0)
        return HttpReply::networkAccessManager()->get(request);
    else if (verb.compare("POST", Qt::CaseInsensitive) == 0)
        return HttpReply::networkAccessManager()->post(request, data);
    else if (verb.compare("HEAD", Qt::CaseInsensitive) == 0)
        return HttpReply::networkAccessManager()->head(request);
    else if (verb.compare("DELETE", Qt::CaseInsensitive) == 0)
        return HttpReply::networkAccessManager()->deleteResource(request);
    else if (verb.compare("PUT", Qt::CaseInsensitive) == 0)
        return HttpReply::networkAccessManager()->put(request, data);
    else
        return HttpReply::networkAccessManager()->sendCustomRequest(request, verb, data);
}