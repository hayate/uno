#include <QDebug>
#include "uview.h"
#include "uerror.h"


uError::uError(QObject *parent) :
    uController(parent),
    m_error(), m_code(0)
{

}

uError::uError(const QString &error, const int code, QObject *parent) :
    uController(parent), m_error(error), m_code(code)
{
}

void uError::indexAction()
{
    if (m_response->mimeType().startsWith("text/html"))
    {
        uView view(":/error.tpl");
        view.insert("error", m_error);
        m_response->setBody(view.fetch());
    }
    else {
        m_response->setBody(m_error);
    }
    m_response->render();
}

void uError::setError(const QString &error, const int code)
{
    m_error = error;
    m_code = code;
}

QString uError::error() const
{
    return m_error;
}

int uError::code() const
{
    return m_code;
}
