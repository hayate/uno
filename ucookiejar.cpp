#include <QMutex>
#include <QDebug>
#include <QDateTime>
#include "uno.h"
#include "uconfig.h"
#include "ucrypto.h"
#include "urequest.h"
#include "ucookiejar.h"

const ulong uCookie::TwentyMins = 1200;
const ulong uCookie::FortyMins  = 2400;
const ulong uCookie::OneHour    = 3600;
const ulong uCookie::TwoHours   = 7200;
const ulong uCookie::OneDay     = 86400;
const ulong uCookie::OneWeek    = 604800;
const ulong uCookie::TwoWeeks   = 1209600;
const ulong uCookie::OneMonth   = 2419200;


uCookie::uCookie() :
    m_name(), m_value(), m_domain(),
    m_maxAge(uCookie::TwentyMins), m_path("/"),
    m_secure(false), m_remove(false), m_httpOnly(false)
{

}

uCookie::uCookie(const QString name, const QString &value) :
    m_name(name), m_value(value), m_domain(),
    m_maxAge(uCookie::TwentyMins), m_path("/"),
    m_secure(false), m_remove(false), m_httpOnly(false)
{

}

void uCookie::setDomain(const QString &domain)
{
    if (! domain.isEmpty())
    {
        m_domain = domain.startsWith(".") ? domain : "." + domain;
    }
}

void uCookie::setMaxAge(const ulong maxage)
{
    m_maxAge = maxage;
}

void uCookie::setName(const QString &name)
{
    m_name = name;
}

void uCookie::setPath(const QString &path)
{
    if (path.isEmpty())
    {
        m_path = "/";
    }
    else {
        m_path = path;
    }
}

void uCookie::setSecure(bool secure)
{
    m_secure = secure;
}

void uCookie::setValue(const QString &value)
{
    m_value = value;
}

void uCookie::setHttpOnly(bool httpOnly)
{
    m_httpOnly = httpOnly;
}

QString uCookie::name() const
{
    return m_name;
}

QString uCookie::value() const
{
    return m_value;
}

bool uCookie::hasValue() const
{
    return !m_value.isEmpty();
}

QString uCookie::path() const
{
    return m_path;
}

QString uCookie::domain() const
{
    return m_domain;
}

ulong uCookie::maxAge() const
{
    return m_maxAge;
}

bool uCookie::secure() const
{
    return m_secure;
}

void uCookie::clear()
{
    m_name.clear();
    m_value.clear();
}

void uCookie::remove()
{
    m_remove = true;
}

QString uCookie::toString() const
{
    if (m_name.isEmpty()) return "";

    QString cookie(m_name);
    cookie.append("=");
    cookie.append(m_value);
    cookie.append(";");

    if (! m_domain.isEmpty())
    {
        cookie.append(" Domain=");
        cookie.append(m_domain);
        cookie.append(";");
    }

    if (m_remove)
    {
        cookie.append(" Expires=Sun, 24 Jan 1971 00:09:45 GMT;");
        cookie.append(" Max-Age=0;");
    }
    else {
        if (m_maxAge > 0)
        {
            QDateTime utc = QDateTime::currentDateTimeUtc();
            QDateTime expire = QDateTime::fromTime_t(utc.toTime_t() + m_maxAge).toUTC();
            cookie.append(" Expires=");
            cookie.append(expire.toString("ddd, dd MMM yyyy hh:mm:ss "));
            cookie.append("GMT;");
        }

        cookie.append(" Max-Age=");
        cookie.append(QString::number(m_maxAge));
        cookie.append(";");
    }

    if (! m_path.isEmpty())
    {
        cookie.append(" Path=");
        cookie.append(m_path);
        cookie.append(";");
    }
    if (m_secure)
    {
        cookie.append(" Secure");
    }
    if (m_httpOnly)
    {
        if (m_secure) cookie.append(";");
        cookie.append(" HttpOnly;");
    }
    cookie.append(" Version=1");
    cookie.prepend("Set-Cookie: ");

    return cookie;
}

uCookieJar::uCookieJar() :
    m_domain(), m_path("/"),
    m_secure(false), m_maxage(uCookie::TwentyMins),
    m_encrypt(false), m_secret("")
{
    uConfig *config = &uConfig::getInstance();

    m_domain = config->value("cookie/domain", "").toString();
    if (! m_domain.isEmpty() && !m_domain.startsWith("."))
    {
        m_domain.prepend(".");
    }
    m_path = config->value("cookie/path", "/").toString();
    m_secure = config->value("cookie/secure", false).toBool();
    m_maxage = config->value("cookie/maxage", (qulonglong)uCookie::TwentyMins).toUInt();
    m_encrypt = config->value("cookie/encrypt", false).toBool();
    m_secret = config->value("secret", "").toString();

}

void uCookieJar::insert(const QString &name, const QVariant &value)
{
    if (value.type() == QVariant::String)
    {
        QString data;
        if (m_encrypt)
        {
            uCrypto crypto(m_secret);
            data = crypto.encrypt(value.toString());
        }
        else {
            data = value.toString();
        }
        uCookie cookie(name, data);
        cookie.setDomain(m_domain);
        cookie.setMaxAge(m_maxage);
        cookie.setPath(m_path);
        cookie.setSecure(m_secure);       
        m_cookies.insert(name, cookie);
    }
    else {
        QByteArray ba;
        {
            QDataStream in(&ba, QIODevice::WriteOnly);
            in.setVersion(QDataStream::Qt_4_7);
            in << (QVariant)value;
        }
        QString data;
        data.append("uno64:");
        data.append(ba.toBase64());

        if (m_encrypt)
        {
            uCrypto crypto(m_secret);
            data = crypto.encrypt(data);
        }
        uCookie cookie(name, data);
        cookie.setDomain(m_domain);
        cookie.setMaxAge(m_maxage);
        cookie.setPath(m_path);
        cookie.setSecure(m_secure);
        m_cookies.insert(name, cookie);
    }
}

void uCookieJar::insert(const uCookie &cookie)
{
    m_cookies.insert(cookie.name(), cookie);
}

void uCookieJar::insert(const cgicc::HTTPCookie &cookie)
{
    uCookie value(QString::fromStdString(cookie.getName()), QString::fromStdString(cookie.getValue()));
    value.setDomain(QString::fromStdString(cookie.getDomain()));
    value.setMaxAge(cookie.getMaxAge());
    value.setPath(QString::fromStdString(cookie.getPath()));
    m_cookies.insert(value.name(), value);
}

bool uCookieJar::remove(const QString &name)
{
    uCookie cookie = this->cookie(name);
    if (cookie.name() == name)
    {
        cookie.remove();
        return true;
    }
    return false;
}

QVariant uCookieJar::getOnce(const QString &name, const QVariant &value)
{
    QVariant ret = this->value(name, value);
    this->remove(name);
    return ret;
}

QVariant uCookieJar::value(const QString &name, const QVariant &value)
{
    if (! m_cookies.contains(name))
    {
        return value;
    }

    uCookie cookie = m_cookies.value(name);
    QString str(cookie.value());
    if (m_encrypt)
    {
        uCrypto crypto(m_secret);
        str = crypto.decrypt(str);
    }
    if (! str.startsWith("uno64:"))
    {
        return str;
    }
    QVariant ret;
    QByteArray ba;
    ba.append(str.mid(6));
    ba = QByteArray::fromBase64(ba);
    {
        QDataStream out(&ba, QIODevice::ReadOnly);
        out.setVersion(QDataStream::Qt_4_7);
        out >> ret;
    }
    return ret;
}

uCookie uCookieJar::cookie(const QString &name) const
{
    return m_cookies.value(name, uCookie());
}

QList<uCookie> uCookieJar::cookies() const
{
    return m_cookies.values();
}

void uCookieJar::setDefaultDomain(const QString &domain)
{
    m_domain = domain;
    if (! m_domain.isEmpty() && !m_domain.startsWith("."))
    {
        m_domain.prepend(".");
    }
}

void uCookieJar::setDefaultPath(const QString &path)
{
    m_path = path;
}

void uCookieJar::setDefaultSecure(bool secure)
{
    m_secure = secure;
}

void uCookieJar::setDefaultMaxAge(ulong maxage)
{
    m_maxage = maxage;
}

void uCookieJar::setEncrypt(bool encrypt)
{
    m_encrypt = encrypt;
}

void uCookieJar::setSecret(const QString &secret)
{
    m_secret = secret;
}

QString uCookieJar::defaultDomain() const
{
    return m_domain;
}

QString uCookieJar::defaultPath() const
{
    return m_path;
}

bool uCookieJar::defaultSecure() const
{
    return m_secure;
}

ulong uCookieJar::defaultMaxAge() const
{
    return m_maxage;
}

bool uCookieJar::encrypt() const
{
    return m_encrypt;
}

QString uCookieJar::secret() const
{
    return m_secret;
}

uCookieJar &uCookieJar::getInstance()
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    Q_UNUSED(lock);
    static uCookieJar instance;
    return instance;
}
