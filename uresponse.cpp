#include <QDebug>
#include <QMutex>
#include "uview.h"
#include "urequest.h"
#include "uresponse.h"
#include "ucookiejar.h"
#include <cgicc/Cgicc.h>
#include <cgicc/HTMLClasses.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTTPPlainHeader.h>
#include <cgicc/HTTPStatusHeader.h>
#include <cgicc/HTTPRedirectHeader.h>
#include <cgicc/HTTPResponseHeader.h>
#include <stdio.h>
#include <iostream>

using namespace cgicc;
using namespace std;

uResponse::uResponse() :
    m_body(""),
    m_mime("text/html; charset=UTF-8"),
    m_status(200, "OK"),
    m_request(0),
    m_cookies(0), m_headers(0)
{
    m_request = &uRequest::getInstance();
    m_cookies = &uCookieJar::getInstance();
}

uResponse::~uResponse()
{
    if (m_headers) delete m_headers;
}

void uResponse::render()
{
    char date[30] = {0};
    time_t now = time(&now);
    strftime(date, 30, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));

    m_headers = new HTTPResponseHeader("HTTP/1.1", m_status.first, m_status.second.toStdString());
    m_headers->addHeader("Date", date);
    m_headers->addHeader("Server", m_request->environment().getServerSoftware());
}

void uResponse::redirect(const QString &location)
{
    char date[30] = {0};
    time_t now = time(&now);
    strftime(date, 30, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));

    m_headers = new HTTPResponseHeader("HTTP/1.1", 302, "Found");
    m_headers->addHeader("Date", date);
    m_headers->addHeader("Server", m_request->environment().getServerSoftware());

    QString url(location);
    if (! url.startsWith("http"))
    {
        if (! url.startsWith("/"))
        {
            url.prepend("/");
        }
        url.prepend(m_request->serverName());
        url.prepend("://");
        url.prepend(m_request->requestSchema());
    }
    m_headers->addHeader("Location", url.toStdString());

    if (! m_request->isHead())
    {
        uView view(":/redirect.tpl");
        view.insert("url", url);
        view.insert("server", m_request->environment().getServerSoftware().c_str());
        view.insert("hostname", m_request->serverName());
        view.insert("port", QString::number(m_request->environment().getServerPort()));
        m_body = view.fetch();
        m_headers->addHeader("Connection", "close");
    }
}

void uResponse::sendHeaders()
{   
    emit packingCookies(m_cookies);
    QList<uCookie> cookies = m_cookies->cookies();
    foreach (uCookie cookie, cookies)
    {
        m_headers->addHeader(cookie.toString().toStdString());
    }
    m_headers->addHeader("Vary", "*");

    emit sendingHeaders(m_headers, m_body);

    m_headers->addHeader("Content-Length", QString::number(m_body.size()).toStdString());
    m_headers->addHeader("Content-Type", m_mime.toStdString());
    m_headers->render(cout);

    emit sentHeaders(m_headers);
}

void uResponse::sendResponse()
{
    if (! m_request->isHead() && !m_body.isEmpty())
    {
        cout << m_body.toStdString();
        emit sentBody(m_body);
    }
}

void uResponse::setBody(const QString &body)
{
    m_body = body;
}

void uResponse::setMimeType(const QString &mime)
{
    m_mime = mime;
}

void uResponse::setStatusCode(int code, const QString &msg)
{
    m_status.first = code;
    m_status.second = msg;
}

QString uResponse::mimeType() const
{
    return m_mime;
}

void uResponse::refresh()
{
    redirect(m_request->requestUrl());
}

uResponse &uResponse::getInstance()
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    Q_UNUSED(lock);
    static uResponse instance;
    return instance;
}
