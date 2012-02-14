#include <QDir>
#include <QMutex>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDomDocument>
#include <QSqlDatabase>
#include <QDesktopServices>
#include "uno.h"
#include "uutil.h"
#include "ucrypto.h"
#include "uconfig.h"
#include "usession.h"
#include "uresponse.h"
#include "uconnectionpool.h"


uSession::uSession() :
    m_session(0), m_engine(), m_encrypt(false), m_secret()
{
    qsrand(QDateTime::currentDateTime().toTime_t());

    uConfig *config = &uConfig::getInstance();
    m_engine = config->value("session/engine", "file").toString();
    m_encrypt = config->value("session/encrypt", false).toBool();
    m_secret = config->value("secret", "").toString();
    uint lifetime = config->value("session/lifetime", (uint)uCookie::TwentyMins).toUInt();

    if (m_engine == "cookie")
    {
        m_session = new uSessionCookie(lifetime);
    }
    else if (m_engine == "database")
    {
        m_session = new uSessionDatabase(lifetime);
    }
    else {
        m_session = new uSessionFile(lifetime);
    }
    // run session garbage collection every around 100 requests.
    if (0 == (qrand() % 100))
    {
        connect((&Uno::getInstance()), SIGNAL(shutdown()), m_session, SLOT(gc()));
    }

    m_session->open();
    connect(m_session, SIGNAL(write()), this, SLOT(write()));
    connect((&uResponse::getInstance()), SIGNAL(packingCookies(uCookieJar*)), m_session, SLOT(addSessionCookie(uCookieJar*)));
    read();
}

uSession::~uSession()
{
    delete m_session;
}

uSession &uSession::getInstance()
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    Q_UNUSED(lock);
    static uSession instance;
    return instance;
}

void uSession::insert(const QString &name, const QVariant &value)
{
    m_params.insert(name, value);
}

QVariant uSession::value(const QString &name, const QVariant &value)
{
    return m_params.value(name, value);
}

QVariant uSession::getOnce(const QString &name, const QVariant &value)
{
    if (m_params.contains(name))
    {
        return m_params.take(name);
    }
    return value;
}

void uSession::remove(const QString &name)
{
    m_params.remove(name);
}

bool uSession::contains(const QString &name)
{
    return m_params.contains(name);
}

void uSession::read()
{
    QString data = m_session->read();
    if (data.isEmpty()) return;

    if (! m_secret.isEmpty() && m_encrypt)
    {
        uCrypto crypto(m_secret);
        data = crypto.decrypt(data);
    }
    QDomDocument doc;
    doc.setContent(data);
    QDomElement root = doc.documentElement();
    QDomNodeList nodes = root.childNodes();
    for (int i = 0; i < nodes.size(); i++)
    {
        QDomElement node = nodes.at(i).toElement();
        QString key = node.nodeName();
        QByteArray ba(QByteArray::fromBase64(node.attribute("value").toUtf8()));
        QVariant value;
        {
            QDataStream out(&ba, QIODevice::ReadOnly);
            out >> value;
        }
        m_params.insert(key, value);
    }
}

void uSession::write()
{
    QDomDocument doc;
    QDomProcessingInstruction head = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    doc.appendChild(head);
    QDomElement root = doc.createElement("unosession");
    doc.appendChild(root);

    QList<QString> keys = m_params.keys();
    QByteArray ba;
    for (int i = 0; i < keys.size(); i++)
    {
        ba.clear();
        QString key = keys.at(i);
        QDomElement node = doc.createElement(key);
        {
            QDataStream in(&ba, QIODevice::WriteOnly);
            in << m_params.value(key);
        }
        node.setAttribute("value", QString(ba.toBase64()));
        root.appendChild(node);
    }

    if (! m_secret.isEmpty() && m_encrypt)
    {
        uCrypto crypto(m_secret);
        m_session->write(crypto.encrypt(doc.toString()));
    }
    else {
        m_session->write(doc.toString());
    }
    m_session->close();
}

uSessionEngine::uSessionEngine(uint lifetime)
{
    m_config = &uConfig::getInstance();
    m_name = m_config->value("session/name", "UNOSESSID").toString();
    uCookieJar *cjar = &uCookieJar::getInstance();
    m_cookie = cjar->cookie(m_name);
    // in case (re)set the name (could be a new cookie)
    m_cookie.setName(m_name);
    m_cookie.setMaxAge(lifetime);
}

void uSessionEngine::addSessionCookie(uCookieJar *cookies)
{
    if (! m_cookie.name().isEmpty())
    {
        cookies->insert(m_cookie);
    }
}

uSessionFile::uSessionFile(uint lifetime) :
    uSessionEngine(lifetime), m_file(), m_basepath()
{
    m_basepath.append(QDesktopServices::storageLocation(QDesktopServices::TempLocation));
    if (m_basepath.isEmpty())
    {
        m_basepath.append(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
        if (m_basepath.isEmpty())
        {
            m_basepath.append(m_config->value("session/location", "").toString());
            if (m_basepath.isEmpty())
            {
                qDebug() << tr("Cannot find a valid location to store session files");
                qDebug() << tr("please set a valid location in the configuration file.");
                qDebug() << tr("Trying /tmp.....");
                m_basepath.append("/tmp");
            }
        }
    }
    m_basepath.append(m_basepath.endsWith("/") ? "" : "/");
    QString path(m_basepath);
    path.append("uno_");

    if (m_cookie.hasValue())
    {
        path.append(m_cookie.value());
        QFileInfo sesfile(path);
        if (sesfile.exists())
        {
            // if the file was NOT accessed within "lifetime"
            // or is older than 24 hours its expired so delete it
            uint now = QDateTime::currentDateTime().toTime_t();
            // yesterday
            QDateTime ieri = QDateTime::fromTime_t(now - 86400);
            QDateTime then = QDateTime::fromTime_t(now - lifetime);

            if ((sesfile.created() < ieri) || (sesfile.lastRead() < then))
            {
                QFile::remove(path);
                m_cookie.setValue(uUtil::uuid());
                path.clear();
                path.append(m_basepath);
                path.append("uno_");
                path.append(m_cookie.value());
            }
        }
    }
    else {
        m_cookie.setValue(uUtil::uuid());
        path.append(m_cookie.value());
    }
    m_file.setFileName(path);

    // here we let know when our write method should be called
    connect((&Uno::getInstance()), SIGNAL(shutdown()), this, SIGNAL(write()));
}

void uSessionFile::open()
{
    m_file.open(QFile::ReadWrite);
}

void uSessionFile::close()
{
    m_file.close();
}

QString uSessionFile::read()
{
    return m_file.readAll();
}

void uSessionFile::write(const QString &data)
{
    m_file.seek(0);
    if (m_file.write(data.toUtf8()) < 0)
    {
        qDebug() << Q_FUNC_INFO << m_file.errorString();
    }
}

void uSessionFile::remove()
{
    if (m_file.exists())
    {
        m_file.remove();
    }
}

void uSessionFile::gc()
{
    QDir dir(m_basepath);
    QStringList filter("uno_*");
    QFileInfoList files = dir.entryInfoList(filter, QDir::Files);
    // time 24 hours from now
    QDateTime ieri = QDateTime::fromTime_t(QDateTime::currentDateTime().toTime_t() - 86400);
    foreach (QFileInfo file, files)
    {
        if (file.created() < ieri)
        {
            QFile::remove(file.absoluteFilePath());
        }
    }
}

uSessionCookie::uSessionCookie(uint lifetime) :
    uSessionEngine(lifetime)
{
    connect((&uResponse::getInstance()), SIGNAL(packingCookies(uCookieJar*)), this, SIGNAL(write()));
}

void uSessionCookie::open() {}

QString uSessionCookie::read()
{
    return QByteArray::fromBase64(m_cookie.value().toUtf8());
}

void uSessionCookie::write(const QString &data)
{
    QByteArray buf;
    buf.append(data);
    m_cookie.setValue(buf.toBase64());
}

void uSessionCookie::close() {}

void uSessionCookie::remove()
{
    m_cookie.remove();
}

void uSessionCookie::gc() {}

/***
 * Session ORM class
 */
uSessionORM::uSessionORM() :
    uORM(setFields(), "sessions")
{

}

QHash<QString, QVariant> uSessionORM::setFields()
{
    QHash<QString, QVariant> fields;
    fields.insert("id", QVariant(QVariant::Invalid));
    fields.insert("session_id", "");
    fields.insert("access", 0);
    fields.insert("data", "");
    return fields;
}

bool uSessionORM::save()
{
    m_fields.insert("access", uUtil::timeUTC());
    return uORM::save();
}

/***
 * Database Session
 */
uSessionDatabase::uSessionDatabase(uint lifetime) :
    uSessionEngine(lifetime)
{
    if (m_cookie.hasValue())
    {
        if ( ! m_ses.findWhere("session_id", m_cookie.value()))
        {
            m_cookie.setValue("");
            qDebug() << m_ses.lastError();
            return;
        }
        if (m_ses.isLoaded())
        {
            QDateTime then = QDateTime::fromTime_t(uUtil::timeUTC() - lifetime);
            QDateTime access = QDateTime::fromTime_t(m_ses.field("access").toUInt());
            if (access < then)
            {
                m_cookie.setValue("");
                m_ses.remove();
            }
        }
    }
    else {
        m_cookie.setValue(uUtil::uuid());
    }
    connect((&Uno::getInstance()), SIGNAL(shutdown()), this, SIGNAL(write()));
}

void uSessionDatabase::open() {}

QString uSessionDatabase::read()
{
    return m_ses.isLoaded() ? m_ses.field("data", "").toString() : "";
}

void uSessionDatabase::write(const QString &data)
{
    if (! data.isEmpty())
    {
        m_ses.setField("session_id", m_cookie.value());
        m_ses.setField("data", data);
        m_ses.save();
    }
}

void uSessionDatabase::remove()
{
    if (m_ses.isLoaded()) m_ses.remove();
}

void uSessionDatabase::close() {}

void uSessionDatabase::gc()
{
    uConnectionPool *pool = &uConnectionPool::getInstance();
    QSqlDatabase con;
    if (! pool->connection(con))
    {
        qDebug() << Q_FUNC_INFO;
        qDebug() << con.lastError().text();
        return;
    }
    QSqlQuery query(con);
    query.prepare("DELETE FROM sessions WHERE access <= ?");
    query.bindValue(0, (uint)(uUtil::timeUTC() - uCookie::OneDay));
    if (! query.exec())
    {
        qDebug() << query.lastError().text();
    }
}
