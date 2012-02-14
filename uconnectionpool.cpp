#include <QUuid>
#include <QMutex>
#include <QDebug>
#include <QHostInfo>
#include <QSqlError>
#include <QMutexLocker>
#include "uutil.h"
#include "uconfig.h"
#include "urequest.h"
#include "uconnectionpool.h"


uConnectionPool::uConnectionPool(QObject *parent) :
    QObject(parent),
    m_counter(0), m_config(0)
{
    m_config = &uConfig::getInstance();
    createPool(PoolSize);
}

uConnectionPool::~uConnectionPool()
{
    foreach (QString name, m_pool)
    {
        QSqlDatabase::removeDatabase(name);
    }
}

void uConnectionPool::createPool(int size)
{
    for (int i = 0; i < size; i++)
    {
        m_pool.append(createConnection());
    }
}

QString uConnectionPool::createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase(m_config->value("database/driver", "QMYSQL").toString(), uUtil::uuid());
    db.setDatabaseName(m_config->value("database/name", "").toString());
    db.setHostName(m_config->value("database/hostname", "localhost").toString());
    db.setUserName(m_config->value("database/username", "").toString());
    db.setPassword(m_config->value("database/password", "").toString());
    return db.connectionName();
}

bool uConnectionPool::connection(QSqlDatabase &con)
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    Q_UNUSED(lock);

    int index = (m_counter % PoolSize);
    con = QSqlDatabase::database(m_pool.at(index), true);
    m_counter++;
    if (m_counter >= 1000) m_counter = 0;

    return (con.isOpen() || con.open());
}

uConnectionPool &uConnectionPool::getInstance()
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    Q_UNUSED(lock);

    static uConnectionPool instance;
    return instance;
}
