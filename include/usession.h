#ifndef USESSION_H
#define USESSION_H

#include "uno_global.h"

#include <QFile>
#include <QSqlDatabase>
#include "uorm.h"
#include "ucookiejar.h"

class uConfig;
class uSessionEngine;


class UNOSHARED_EXPORT uSession : public QObject
{
    Q_OBJECT
public:
    static uSession &getInstance();
    ~uSession();

public:
    void insert(const QString &name, const QVariant &value);
    bool contains(const QString &name);
    QVariant value(const QString &name, const QVariant &value = QVariant(QVariant::String));
    QVariant getOnce(const QString &name, const QVariant &value = QVariant(QVariant::String));
    void remove(const QString &name);

private slots:
    void write();
    void read();

private:
    uSessionEngine *m_session;
    QString m_engine;
    bool m_encrypt;
    QString m_secret;
    QHash<QString, QVariant> m_params;

private:
    uSession();
};

class UNOSHARED_EXPORT uSessionEngine : public QObject
{
    Q_OBJECT
public:
    uSessionEngine(uint lifetime);

signals:
    void write();

public slots:
    virtual void open() = 0;
    virtual void close() = 0;
    virtual QString read() = 0;
    virtual void write(const QString &data) = 0;
    virtual void remove() = 0;
    virtual void gc() = 0;

public slots:
    void addSessionCookie(uCookieJar *cookies);

protected:
    QString m_name;
    uCookie m_cookie;
    uConfig *m_config;
};

class UNOSHARED_EXPORT uSessionFile : public uSessionEngine
{
    Q_OBJECT
public:
    uSessionFile(uint lifetime = uCookie::TwentyMins);

public slots:
    void open();
    void close();
    QString read();
    void write(const QString &data);
    void remove();
    void gc();

private:
    QFile m_file;
    QString m_basepath;
};

class UNOSHARED_EXPORT uSessionCookie : public uSessionEngine
{
    Q_OBJECT
public:
    uSessionCookie(uint lifetime = uCookie::TwentyMins);

public slots:
    void open();
    void close();
    QString read();
    void write(const QString &data);
    void remove();
    void gc();
};

class UNOSHARED_EXPORT uSessionORM : public uORM
{
public:
    uSessionORM();

public:
    bool save();

protected:
    static QHash<QString, QVariant> setFields();
};

class UNOSHARED_EXPORT uSessionDatabase : public uSessionEngine
{
    Q_OBJECT
public:
    uSessionDatabase(uint lifetime = uCookie::TwentyMins);

public slots:
    void open();
    void close();
    QString read();
    void write(const QString &data);
    void remove();
    void gc();

private:
    uSessionORM m_ses;
};


#endif // USESSION_H
