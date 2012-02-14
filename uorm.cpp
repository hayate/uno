#include <QDebug>
#include <QSqlField>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringList>
#include <QSqlDatabase>
#include "uorm.h"
#include "uconnectionpool.h"


uORM::uORM(const QHash<QString, QVariant> fields, const QString &table) :
    m_table(table),
    m_primary("id"),
    m_fields(fields),
    m_error(),
    m_loaded(false)
{
    m_pool = &uConnectionPool::getInstance();
}

bool uORM::load(const QVariant &id)
{
    QSqlDatabase con;
    if (! m_pool->connection(con))
    {
        m_error = con.lastError().text();
        qDebug() << m_error << Q_FUNC_INFO;
        return false;
    }
    QString primary = primaryField(id);
    QString q("SELECT * FROM %1 WHERE %2=? LIMIT 1");
    q = q.arg(m_table).arg(primary);
    QSqlQuery query(con);
    query.prepare(q);

    QVariant primaryKey = id.isValid() ? id : m_fields.value(primary);
    query.bindValue(0, primaryKey);
    if (! query.exec())
    {
        m_error = query.lastError().text();
        qDebug() << m_error << Q_FUNC_INFO;
        qDebug() << query.lastQuery();
        return false;
    }
    if (! query.next())
    {
        m_loaded = false;
        return true;
    }
    m_loaded = true;
    QSqlRecord record = query.record();
    QStringList fields = m_fields.keys();
    for (int i = 0; i < fields.size(); i++)
    {
        QSqlField field = record.field(fields.at(i));
        m_fields.insert(fields.at(i), field.value());
    }
    return true;
}

bool uORM::findWhere(const QString &field, const QVariant &value)
{
    QSqlDatabase con;
    if (! m_pool->connection(con))
    {
        m_error = con.lastError().text();
        return false;
    }
    QString q("SELECT * FROM %1 WHERE %2=? LIMIT 1");
    q = q.arg(m_table).arg(field);
    QSqlQuery query(con);
    query.prepare(q);
    query.bindValue(0, value);
    if (! query.exec())
    {
        m_error = query.lastError().text();
        return false;
    }
    if (! query.next())
    {
        m_loaded = false;
        return true;
    }
    m_loaded = true;
    QSqlRecord record = query.record();
    QStringList fields = m_fields.keys();
    for (int i = 0; i < fields.size(); i++)
    {
        QSqlField field = record.field(fields.at(i));
        m_fields.insert(fields.at(i), field.value());
    }
    return true;
}

bool uORM::isLoaded() const
{
    return m_loaded;
}

bool uORM::setField(const QString &name, const QVariant &value)
{
    if (m_fields.contains(name))
    {
        m_fields.insert(name, value);
        return true;
    }
    qDebug() << QObject::trUtf8("Field %1 does not exists in table: %2").arg(name).arg(m_table);
    return false;
}

QVariant uORM::field(const QString &name, const QVariant &value)
{
    return m_fields.value(name, value);
}

QHash<QString, QVariant> uORM::fields() const
{
    return m_fields;
}

bool uORM::save()
{
    m_error.clear();

    QVariant pk = m_fields.value(primaryField(), QVariant(QVariant::Invalid));
    if (pk.isValid())
    {
        if (pk.canConvert(QVariant::Int))
        {
            bool ok = false;
            long id = pk.toInt(&ok);
            if (ok)
            {
                if (id == 0)
                {
                    return create();
                }
                else {
                    return update();
                }
            }
        }
        if (pk.canConvert(QVariant::String))
        {
            QString id = pk.toString();
            if (id.isEmpty())
            {
                return create();
            }
            else {
                return update();
            }
        }

    }
    return create();
}

bool uORM::create()
{
    QSqlDatabase con;
    if (! m_pool->connection(con))
    {
        m_error = con.lastError().text();
        return false;
    }
    QStringList fields = m_fields.keys();
    // remove the primary key
    for (int i = 0; i < fields.size(); i++)
    {
        if (fields.at(i) == m_primary)
        {
            fields.removeAt(i);
            break;
        }
    }
    // construct query string
    QString q("INSERT INTO %1 (");
    q = q.arg(m_table);
    foreach (QString field, fields)
    {
        q.append(field);
        q.append(",");
    }
    q = q.mid(0, q.length() -1);
    q.append(") VALUES (");
    for (int i = 0; i < fields.size(); i++)
    {
        q.append("?,");
    }
    q = q.mid(0, q.length() -1);
    q.append(")");

    QSqlQuery query(con);
    query.prepare(q);
    for (int i = 0; i < fields.size(); i++)
    {
        query.bindValue(i, m_fields.value(fields.at(i)));
    }
    if (! query.exec())
    {
        m_error = query.lastError().text();
        return false;
    }
    m_fields.insert(primaryField(), query.lastInsertId());
    return true;
}

bool uORM::update()
{
    QSqlDatabase con;
    if (! m_pool->connection(con))
    {
        m_error = con.lastError().text();
        return false;
    }
    QStringList fields = m_fields.keys();
    // remove the primary key
    for (int i = 0; i < fields.size(); i++)
    {
        if (fields.at(i) == m_primary)
        {
            fields.removeAt(i);
            break;
        }
    }
    // construct query string
    QString q("UPDATE %1 SET ");
    foreach (QString field, fields)
    {
        q.append(field);
        q.append("=?,");
    }
    q = q.mid(0, q.length() -1);
    q.append(" WHERE %2=?");
    q = q.arg(m_table).arg(primaryField());

    QSqlQuery query(con);
    query.prepare(q);
    int i;
    for (i = 0; i < fields.size(); i++)
    {
        query.bindValue(i, m_fields.value(fields.at(i)));
    }
    query.bindValue(i, m_fields.value(primaryField()));
    if (! query.exec())
    {
        m_error = query.lastError().text();
        return false;
    }
    return true;
}

bool uORM::remove()
{
    // the primary key value
    QVariant id = m_fields.value(primaryField(), QVariant(QVariant::Invalid));
    if (! id.isValid())
    {
        qDebug() << QObject::tr("Cannot delete row in %1 without knowing the primary key.").arg(m_table);
        return false;
    }
    QSqlDatabase con;
    if (! m_pool->connection(con))
    {
        m_error = con.lastError().text();
        return false;
    }
    QString q("DELETE FROM %1 WHERE %2=?");
    q = q.arg(m_table).arg(primaryField());

    QSqlQuery query(con);
    query.prepare(q);
    query.bindValue(0, id);
    if (! query.exec())
    {
        m_error = query.lastError().text();
        return false;
    }
    foreach (QString key, m_fields.keys())
    {
        m_fields.insert(key, QVariant(QVariant::Invalid));
    }
    m_loaded = false;
    return true;
}

QString uORM::primaryField(const QVariant &id)
{
    Q_UNUSED(id);
    return m_primary;
}

QString uORM::lastError() const
{
    return m_error;
}
