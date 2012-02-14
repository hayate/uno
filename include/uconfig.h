#ifndef UCONFIG_H
#define UCONFIG_H

#include "uno_global.h"
#include <QVariant>

class QSettings;

class UNOSHARED_EXPORT uConfig
{
public:
    static uConfig &getInstance();
    virtual ~uConfig();

public:
    QVariant value(const QString &name, const QVariant &value = QVariant(QVariant::String));
    bool contains(const QString &name);

public:
    void createConfig(const QString &path);

private:
    QSettings *m_settings;

private:
    uConfig();
    uConfig(const uConfig &);
    uConfig &operator = (const uConfig &);
};

#endif // UCONFIG_H
