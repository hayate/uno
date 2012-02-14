#include <QDebug>
#include <QMetaMethod>
#include <QStringList>
#include "uno.h"
#include "uerror.h"
#include "urequest.h"
#include "uresponse.h"
#include "ucontroller.h"
#include "udispatcher.h"


uDispatcher::uDispatcher(QObject *parent) :
    QObject(parent),
    m_request(0)
{
    m_request = &uRequest::getInstance();
    registerController<uError>();
}

void uDispatcher::dispatch()
{
    m_request->setDispatched(true);

    QPair<uController*, int> handler = findController();
    QMetaMethod action = handler.first->metaObject()->method(handler.second);
    action.invoke(handler.first);
    handler.first->deleteLater();
}

QPair<uController*, int> uDispatcher::findController()
{
    QString name = m_request->controller();

    if (m_controllers.contains(name))
    {
        const QMetaObject *meta = m_controllers.value(name);
        uController *controller = static_cast<uController*>(meta->newInstance());
        int method = findAction(controller);
        if (method >= 0)
        {
            return QPair<uController*, int>(controller, method);
        }
        else {
            delete controller;
        }
    }
    // if we are here we can't find a controller or the action
    // Lets check if the user has and Error controller
    // if not we use our own
    const QMetaObject *meta = m_controllers.value("Error", 0);
    uError *controller = 0;
    if (meta)
    {
        controller = static_cast<uError*>(meta->newInstance());
    }
    else {
        controller = new uError();
    }
    controller->setError(tr("Request url: %1 Not Found").arg(m_request->requestUrl()), 404);
    m_request->setAction("index");
    int method = findAction(controller);
    return QPair<uController*, int>(controller, method);
}

int uDispatcher::findAction(const uController *controller)
{
    QString action = m_request->action();
    int defaultAction = -1;

    for (int i = 0; i < controller->metaObject()->methodCount(); i++)
    {
        QMetaMethod method = controller->metaObject()->method(i);
        if (method.access() == QMetaMethod::Public)
        {
            if (QMetaObject::normalizedSignature(method.signature()).startsWith(action.toAscii()))
            {
                return i;
            }
            else if (QMetaObject::normalizedSignature(method.signature()).startsWith(m_request->DefaultAction.toAscii()))
            {
                defaultAction = i;
            }
        }
    }
    return defaultAction;
}

