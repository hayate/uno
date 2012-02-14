#ifndef UREQUEST_H
#define UREQUEST_H

#include "uno_global.h"

#include <QObject>
#include <QVariant>
#include <cgicc/Cgicc.h>

class uCookieJar;


using namespace cgicc;

class UNOSHARED_EXPORT uRequest : public QObject
{
    Q_OBJECT
public:
    static uRequest &getInstance();

signals:
    void parsedCookies(uCookieJar *cookies);
    void parsedQueryString(QHash<QString, QVariant> &query);
    void parsedPostParams(QHash<QString, QVariant> &params);
    void routedRequest(QString &controller, QString &action);
    void requestReady();

public slots:
    void parseRequest();

public:
    const QString DefaultController;
    const QString DefaultAction;

public:
    bool isPost() const;
    bool isGet() const;
    bool isHead() const;
    QString method() const;
    QString controller() const;
    QString action() const;
    void setAction(const QString &action);
    QVariant post(const QString &name, const QVariant &value = QVariant(QVariant::Invalid)) const;
    QVariant get(const QString &name, const QVariant &value = QVariant(QVariant::Invalid)) const;
    bool hasPost(const QString &name) const;
    bool hasGet(const QString &name) const;
    QList<QVariant> params() const;
    QString remoteAddress() const;
    QString serverAddress() const;
    QString serverName() const;
    QString requestPath() const;
    QString requestSchema() const;
    QString requestUrl() const;
    const CgiEnvironment & environment() const;
    void setDispatched(bool dispatched);
    bool dispatched() const;

private:
    void parseCookies();
    void parseQueryString();
    void parsePostParams();
    void routeRequest();

private:
    Cgicc m_cgi;
    const CgiEnvironment &m_env;
    QList<QVariant> m_params;
    QHash<QString, QVariant> m_post;
    QHash<QString, QVariant> m_get;    
    QString m_controller;
    QString m_action;
    QString m_method;
    QString m_path;
    bool m_dispatched;

private:
    uRequest(QObject *parent = 0);
};

#endif // UREQUEST_H
