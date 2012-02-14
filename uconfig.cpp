#include <QMutex>
#include <QSettings>
#include "uconfig.h"

uConfig::uConfig() :
    m_settings(0)
{
}

uConfig::~uConfig()
{
    if (m_settings) delete m_settings;
}

void uConfig::createConfig(const QString &path)
{
    if (m_settings != 0)
    {
        delete m_settings;
        m_settings = 0;
    }
    m_settings = new QSettings(path, QSettings::IniFormat);
}

QVariant uConfig::value(const QString &name, const QVariant &value)
{
    if (m_settings != 0)
    {
        return m_settings->value(name, value);
    }
    return value;
}

bool uConfig::contains(const QString &name)
{
    if (m_settings != 0)
    {
        return m_settings->contains(name);
    }
    return false;
}

uConfig &uConfig::getInstance()
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    Q_UNUSED(lock);
    static uConfig instance;
    return instance;
}
