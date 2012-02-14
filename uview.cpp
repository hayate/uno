#include <QDir>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <QSettings>
#include <QScriptEngine>
#include <iostream>
#include "uno.h"
#include "uview.h"
#include "uconfig.h"


uView::uView() :
    m_error(), m_tmplpath(), Sep(0x1D), m_config(0),
    m_regex("^\\s*include\\s+\\\"?([^\\\"]*)\\\"?")
{
    m_config = &uConfig::getInstance();
    m_oTag.setPattern(m_config->value("openTag", "<%").toString());
    m_cTag.setPattern(m_config->value("closeTag", "%>").toString());
}

uView::uView(const QString &tpl) :
    m_error(), m_tmplpath(tpl), Sep(0x1D), m_config(0),
    m_regex("^\\s*include\\s+\\\"?([^\\\"]*)\\\"?")
{
    m_config = &uConfig::getInstance();
    m_oTag.setPattern(m_config->value("openTag", "<%").toString());
    m_cTag.setPattern(m_config->value("closeTag", "%>").toString());
}

uView::uView(const uView &other) :
    m_params(other.m_params), m_error(other.m_error),
    m_tmplpath(other.m_tmplpath),
    m_oTag(other.m_oTag), m_cTag(other.m_cTag), Sep(0x1D), m_config(0),
    m_regex("^\\s*include\\s+\\\"?([^\\\"]*)\\\"?")
{
    m_config = &uConfig::getInstance();
}

void uView::insert(const QString &name, const QVariant &value)
{
    m_params.insert(name, value);
}

void uView::insert(const QString &name, const uView &view)
{
    QVariant v;
    v.setValue(view);
    m_params.insert(name, v);
}

bool uView::render()
{
    std::cout << fetch().toStdString();
    return m_error.isEmpty();
}

QString uView::fetch()
{
    m_error.clear();

    QString js("(function() { with(this) { var s='';\n");
    js.append(extract(load(m_tmplpath))).append(" return s;} })");
    if (! m_error.isEmpty()) return "";

    QScriptEngine engine;

    QScriptValue script = engine.evaluate(js);
    if (! script.isFunction())
    {
        if (engine.hasUncaughtException())
        {
            m_error = QString("%1 At line %2").arg(engine.uncaughtException().toString()).arg(engine.uncaughtExceptionLineNumber());
        }
        else {
            m_error = QString("Error parsing template %1").arg(m_tmplpath);
        }
        qDebug() << m_error << js;
        return "";
    }
    QScriptValue vars = engine.newObject();
    QList<QString> keys = m_params.keys();
    foreach (QString key, keys)
    {
        QVariant value = m_params.value(key);
        if (strcmp(value.typeName(), "UView") == 0)
        {
            uView view(value.value<uView>());
            vars.setProperty(key, view.fetch());
        }
        else {
            vars.setProperty(key, m_params.value(key).toString());
        }
    }
    QScriptValue page = script.call(vars);
    if (engine.hasUncaughtException())
    {
        m_error = QString("%1 At line %2").arg(engine.uncaughtException().toString()).arg(engine.uncaughtExceptionLineNumber());
        qDebug() << m_error << js;
        return "";
    }
    return page.toString().replace(Sep, "\n");
}

QString uView::extract(const QString &tmpl)
{
    int pos = 0;
    int next = 0;
    bool in = false;

    QString fragment;

    while (pos < tmpl.size())
    {
        if (! in)
        {
            next = m_oTag.indexIn(tmpl, pos);
            if (next == -1) next = tmpl.size();

            QString escaped = tmpl.mid(pos, (next - pos));
            escaped.replace('\n', Sep);
            if (escaped != "")
            {
                fragment += "s += '" + escaped + "';\n";
            }
            pos = next + 2;
            in = true;
        }
        else {
            next = m_cTag.indexIn(tmpl, pos);
            if (next < 0)
            {
                qDebug() << QObject::tr("Missing closing tag");
                m_error = QObject::tr("Missing closing tag");
                return "";
            }
            if (tmpl[pos] == '=')
            {
                pos++;
                fragment += "s += " + tmpl.mid(pos, (next - pos)) + "\n";
            }
            else {
                QString snip = tmpl.mid(pos, (next - pos));
                if (m_regex.indexIn(snip) != -1)
                {
                    fragment += extract(load(m_regex.cap(1).trimmed())) + "\n";
                }
                else {
                    fragment += snip + "\n";
                }
            }
            pos = next + 2;
            in = false;
        }
    }
    return fragment;
}

QString uView::load(const QString &tmplpath)
{
    QFileInfo info;
    if (tmplpath.startsWith(":")) // resource template
    {
        info.setFile(tmplpath);
    }
    else {
        QDir templates(m_config->value("templates").toString());
        info.setFile(templates, tmplpath);
    }
    if (info.exists())
    {
        QFile file(info.absoluteFilePath());
        file.open(QFile::ReadOnly);
        QString tmpl(file.readAll());
        file.close();
        return tmpl;
    }
    else {
        qDebug() << QObject::tr("Template: %1 could not be found.").arg(info.absoluteFilePath());
        m_error = QObject::tr("Template: %1 is missing.").arg(info.absoluteFilePath());
    }
    return "";
}

QString uView::lastError() const
{
    return m_error;
}

void uView::setTemplate(const QString &tmplpath)
{
    m_tmplpath = tmplpath;
}

uView &uView::operator =(const uView &other)
{
    if (this == &other) return *this;

    m_params = other.m_params;
    m_error = other.m_error;
    m_tmplpath = other.m_tmplpath;
    m_oTag.setPattern(other.m_oTag.pattern());
    m_cTag.setPattern(other.m_cTag.pattern());
    m_config = other.m_config;
    m_regex.setPattern(other.m_regex.pattern());

    return *this;
}
