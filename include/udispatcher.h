#ifndef UDISPATCHER_H
#define UDISPATCHER_H

#include "uno_global.h"

#include <QPair>
#include <QHash>
#include <QObject>
#include <memory>
#include "ucontroller.h"

class uRequest;

class UNOSHARED_EXPORT uDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit uDispatcher(QObject *parent = 0);

public slots:
    void dispatch(); 

protected:
    virtual void registerControllers() = 0;
    template<class T> inline void registerController()
    {
        m_controllers.insert(T::staticMetaObject.className(), &(T::staticMetaObject));
    }

private:
    QPair<uController*, int> findController();
    int findAction(const uController *controller);

private:
    uRequest *m_request;
    QHash<QString, const QMetaObject*> m_controllers;
};

#endif // UDISPATCHER_H
