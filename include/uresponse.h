#ifndef URESPONSE_H
#define URESPONSE_H

#include "uno_global.h"
#include <QVariant>
#include <cgicc/HTTPResponseHeader.h>


class uRequest;
class uCookieJar;

class UNOSHARED_EXPORT uResponse : public QObject
{
    Q_OBJECT
public:
    static uResponse &getInstance();
    ~uResponse();

signals:
    void packingCookies(uCookieJar *cookies);
    void sendingHeaders(cgicc::HTTPResponseHeader *headers, QString &body);
    void sentBody(const QString &body);
    void sentHeaders(const cgicc::HTTPResponseHeader *headers);

public slots:
    void render();
    void refresh();
    void redirect(const QString &location);
    void sendHeaders();
    void sendResponse();

public:
    void setBody(const QString &body);
    void setMimeType(const QString &mime);
    void setStatusCode(int code, const QString &msg);
    QString mimeType() const;

private:
    QString m_body;
    QString m_mime;
    QPair<int, QString> m_status;
    uRequest *m_request;
    uCookieJar *m_cookies;
    cgicc::HTTPResponseHeader *m_headers;

private:
    uResponse();
};

#endif // URESPONSE_H
