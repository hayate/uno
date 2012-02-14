#include <QMutex>
#include <QDebug>
#include <QStringList>
#include "urequest.h"
#include "ucookiejar.h"
#include <netdb.h>
#include <iostream>
#include <arpa/inet.h>


uRequest::uRequest(QObject *parent) :
    QObject(parent),
    DefaultController("Time"),
    DefaultAction("indexAction"),
    m_env(m_cgi.getEnvironment()),
    m_params(),    
    m_controller(DefaultController),
    m_action(DefaultAction),
    m_dispatched(false)
{
    m_method = QString::fromStdString(m_env.getRequestMethod()).toLower();
}

void uRequest::parseRequest()
{
    parseCookies();
    parseQueryString();
    parsePostParams();
    routeRequest();

    emit requestReady();
}

QString uRequest::controller() const
{
    return m_controller;
}

QString uRequest::action() const
{
    return m_action;
}

QVariant uRequest::post(const QString &name, const QVariant &value) const
{
    return m_post.value(name, value);
}

QVariant uRequest::get(const QString &name, const QVariant &value) const
{
    return m_get.value(name, value);
}

QList<QVariant> uRequest::params() const
{
    return m_params;
}

void uRequest::routeRequest()
{
    if (! m_get.contains("q")) return;

    QString q = m_get.value("q", "").toString();
    m_get.remove("q");
    if (q.endsWith("/"))
    {
        q = q.mid(0, q.length() - 1);
    }
    if (q.startsWith("/"))
    {
        q = q.mid(1);
    }    
    QStringList route = q.split("/", QString::SkipEmptyParts);
    if (route.size() >= 2)
    {
        m_controller = route.takeFirst().toLower();
        m_controller[0] = m_controller[0].toUpper();

        m_action = route.takeFirst().toLower() + "Action";
    }
    else if (route.size() == 1)
    {
        m_controller = route.takeFirst().toLower();
        m_controller[0] = m_controller[0].toUpper();
    }
    for (int i = 0; i < route.size(); i++)
    {
        m_params.insert(i, route.at(i));
    }
    m_path = q.isEmpty() ? "/" : ("/" + q);

    emit routedRequest(m_controller, m_action);
}

void uRequest::parseCookies()
{
    uCookieJar *oo = &uCookieJar::getInstance();

    std::vector<HTTPCookie> cookies = m_env.getCookieList();
    for (std::vector<HTTPCookie>::iterator i = cookies.begin(); i != cookies.end(); i++)
    {
        if ((*i).getPath().empty())
        {
            (*i).setPath("/");
        }
        if ((*i).getDomain().empty())
        {
            (*i).setDomain(serverName().toStdString());
        }
        oo->insert((*i));
    }
    emit parsedCookies(oo);
}

void uRequest::parsePostParams()
{
    std::vector<FormEntry> params = m_cgi.getElements();
    for (std::vector<FormEntry>::iterator i = params.begin(); i != params.end(); i++)
    {
        std::string k = ((FormEntry)*i).getName();
        std::string v = ((FormEntry)*i).getStrippedValue();
        QString key = QString::fromUtf8(k.data(), k.size());
        QString value = QString::fromUtf8(v.data(), v.size());
        m_post.insert(key, value);
    }
    emit parsedPostParams(m_post);
}

void uRequest::parseQueryString()
{
    QString query = m_env.getQueryString().c_str();
    QStringList params = query.split("&", QString::SkipEmptyParts);
    foreach (QString param, params )
    {
        QList<QString> pair = param.split("=", QString::SkipEmptyParts);
        if (pair.size() == 2)
        {
            m_get.insert(pair.at(0), pair.at(1).trimmed());
        }
        if (pair.size() == 1)
        {
            m_get.insert(pair.at(0), QVariant());
        }
    }
    emit parsedQueryString(m_get);
}

QString uRequest::remoteAddress() const
{
    std::string addr = m_env.getRemoteAddr();
    return QString::fromUtf8(addr.data(), addr.size());
}

QString uRequest::serverAddress() const
{
    struct hostent *hp = 0;
    std::string host = m_env.getServerName();
    hp = gethostbyname(host.data());
    if(hp != 0 && (hp->h_addr_list[0] != 0))
    {
        return inet_ntoa(*((in_addr*)hp->h_addr_list[0]));
    }
    return host.data();
}

QString uRequest::serverName() const
{
    return QString(m_env.getServerName().c_str());
}

QString uRequest::requestPath() const
{
    return m_path;
}

QString uRequest::requestSchema() const
{
    return m_env.usingHTTPS() ? "https" : "http";
}

QString uRequest::requestUrl() const
{
    QString url(requestSchema());
    url.append("://");
    url.append(m_env.getServerName().c_str());
    if (m_env.getServerPort() != 80)
    {
        url.append(":");
        url.append(QString::number(m_env.getServerPort()));
    }
    QString script(m_env.getScriptName().c_str());
    if (! script.startsWith("/"))
    {
        url.append("/");
    }
    url.append(script);
    url.append(requestPath());
    return url;
}

void uRequest::setAction(const QString &action)
{
    if (! action.endsWith("Action"))
    {
        m_action = (action + "Action");
    }
    else {
        m_action = action;
    }
}

bool uRequest::isPost() const
{
    return ("post" == m_method);
}

bool uRequest::isGet() const
{
    return ("get" == m_method);
}

bool uRequest::isHead() const
{
    return ("head" == m_method);
}

QString uRequest::method() const
{
    return m_method;
}

const CgiEnvironment & uRequest::environment() const
{
    return m_env;
}

bool uRequest::dispatched() const
{
    return m_dispatched;
}

void uRequest::setDispatched(bool dispatched)
{
    m_dispatched = dispatched;
}

uRequest &uRequest::getInstance()
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    Q_UNUSED(lock);

    static uRequest instance;
    return instance;
}
