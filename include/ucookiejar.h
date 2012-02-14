#ifndef UCOOKIE_H
#define UCOOKIE_H

#include "uno_global.h"

#include <QVariant>
#include <map>
#include <vector>
#include <cgicc/HTTPCookie.h>

class UNOSHARED_EXPORT uCookie
{
public:
    uCookie();
    uCookie(const QString name, const QString &value);

public:
    void setName(const QString &name);
    void setValue(const QString &value);
    void setDomain(const QString &domain);
    void setMaxAge(const ulong maxage);
    void setPath(const QString &path);
    void setSecure(bool secure);
    void setHttpOnly(bool httpOnly);

    QString name() const;
    QString value() const;
    bool hasValue() const;
    QString domain() const;
    ulong maxAge() const;
    QString path() const;
    bool secure() const;
    void clear();
    // will expire this cookie
    void remove();

    QString toString() const;
    void render(std::ostream &out);

public:
    static const ulong TwentyMins; // 1200;
    static const ulong FortyMins;  // 2400;
    static const ulong OneHour;    // 3600;
    static const ulong TwoHours;   // 7200;
    static const ulong OneDay;     // 86400;
    static const ulong OneWeek;    // 604800;
    static const ulong TwoWeeks;   // 1209600;
    static const ulong OneMonth;   // 2419200;

private:
    QString m_name;
    QString m_value;
    QString m_domain;
    ulong m_maxAge;
    QString m_path;
    bool m_secure;
    bool m_remove;
    bool m_httpOnly;
};


class UNOSHARED_EXPORT uCookieJar
{
public:
    static uCookieJar &getInstance();

public:
    void insert(const QString &name, const QVariant &value);
    void insert(const uCookie &cookie);
    void insert(const cgicc::HTTPCookie &cookie);
    bool remove(const QString &name);
    QVariant value(const QString &name, const QVariant &value = QVariant(QVariant::String));
    uCookie cookie(const QString &name) const;
    QList<uCookie> cookies() const;
    QVariant getOnce(const QString &name, const QVariant &value = QVariant(QVariant::String));
    bool contains(const QString &name) const;

    void setDefaultDomain(const QString &domain);
    void setDefaultPath(const QString &path);
    void setDefaultMaxAge(ulong maxage);
    void setDefaultSecure(bool secure);
    void setEncrypt(bool encrypt);
    void setSecret(const QString &secret);

    QString defaultDomain() const;
    QString defaultPath() const;
    ulong defaultMaxAge() const;
    bool defaultSecure() const;

    bool encrypt() const;
    QString secret() const;

private:
    QHash<QString, uCookie> m_cookies;
    QString m_domain;
    QString m_path;
    bool m_secure;
    ulong m_maxage;
    bool m_encrypt;
    QString m_secret;

private:
    uCookieJar();
    uCookieJar(const uCookieJar &);
    uCookieJar &operator=(const uCookieJar &);
};

#endif // UCOOKIE_H
