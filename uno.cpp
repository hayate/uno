#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include "uno.h"
#include "uconfig.h"
#include "urequest.h"
#include "uresponse.h"
#include "udispatcher.h"


Uno::Uno() :
    m_dispatcher(0),
    m_request(0),
    m_response(0)
{
    m_request = &uRequest::getInstance();
    connect(this, SIGNAL(startup()), m_request, SLOT(parseRequest()));
}

Uno::~Uno()
{
    if (m_dispatcher)
    {
        delete m_dispatcher;
    }
}

void Uno::setConfig(const QString &path)
{
    (&uConfig::getInstance())->createConfig(path);
}

void Uno::setDispatcher(const uDispatcher *dispatcher)
{
    m_dispatcher = const_cast<uDispatcher*>(dispatcher);
}

void Uno::run()
{
    if (m_dispatcher == 0)
    {
        qDebug() << QObject::tr("Uno cannot run without a valid dispatcher.");
        return;
    }
    emit startup();

    do {
        m_dispatcher->dispatch();

    } while (! m_request->dispatched());

    m_response = &uResponse::getInstance();
    m_response->sendHeaders();
    m_response->sendResponse();

    emit shutdown();
}

Uno &Uno::getInstance()
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    static Uno instance;
    return instance;
}
