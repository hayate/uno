#ifndef UORM_H
#define UORM_H

#include "uno_global.h"

#include <QHash>
#include <QVariant>


class uConnectionPool;

class UNOSHARED_EXPORT uORM
{
public:
    uORM(const QHash<QString, QVariant> fields, const QString &table);

public:
    virtual bool save();
    bool setField(const QString &name, const QVariant &value);
    QVariant field(const QString &name, const QVariant &value = QVariant(QVariant::Invalid));
    QString lastError() const;
    bool isLoaded() const;
    bool findWhere(const QString &field, const QVariant &value);
    bool load(const QVariant &id = QVariant(QVariant::Invalid));
    QHash<QString, QVariant> fields() const;
    bool remove();

protected:
    virtual QString primaryField(const QVariant &id = QVariant());
    virtual bool create();
    virtual bool update();

protected:
    QString m_table;
    QString m_primary;
    QHash<QString, QVariant> m_fields;
    uConnectionPool *m_pool;
    QString m_error;
    bool m_loaded;
};

#endif // UORM_H
