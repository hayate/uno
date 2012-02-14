#ifndef UVIEW_H
#define UVIEW_H

#include "uno_global.h"

#include <QFile>
#include <QHash>
#include <QRegExp>
#include <QMetaType>
#include <QStringMatcher>

class uConfig;


class UNOSHARED_EXPORT uView
{
public:
    uView();
    explicit uView(const QString &tpl);
    uView(const uView &other);

public:
    uView &operator =(const uView &other);
    void insert(const QString &name, const QVariant &value);
    void insert(const QString &name, const uView &view);
    bool render();
    QString fetch();
    QString lastError() const;
    void setTemplate(const QString &tmplpath);

private:
    QString extract(const QString &tmpl);
    QString load(const QString &tmplpath);

private:
    QHash<QString, QVariant> m_params;
    QString m_error;
    QString m_tmplpath;
    QStringMatcher m_oTag; // open tag
    QStringMatcher m_cTag; // close tag
    const QChar Sep;
    uConfig *m_config;
    QRegExp m_regex;
};

Q_DECLARE_METATYPE(uView)

#endif // UVIEW_H
