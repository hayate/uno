#ifndef UCONNECTIONPOOL_H
#define UCONNECTIONPOOL_H

#include "uno_global.h"

#include <QObject>
#include <QSqlDatabase>

class uConfig;


class UNOSHARED_EXPORT uConnectionPool : public QObject
{
    Q_OBJECT
public:
    static uConnectionPool &getInstance();
    ~uConnectionPool();

public:
    static const int PoolSize = 5;

signals:

public:
    bool connection(QSqlDatabase &con);

public slots:

private:
    void createPool(int size);
    QString createConnection();

private:
    QList<QString> m_pool;
    quint16 m_counter;
    uConfig *m_config;

private:
    uConnectionPool(QObject *parent = 0);
};

#endif // UCONNECTIONPOOL_H
