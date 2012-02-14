#ifndef UNO_H
#define UNO_H

#include "uno_global.h"

#include <QObject>
#include "udispatcher.h"
#include <cgicc/HTTPResponseHeader.h>

class uRequest;
class uResponse;


class UNOSHARED_EXPORT Uno : public QObject
{
    Q_OBJECT
public:
    static Uno &getInstance();
    ~Uno();

signals:
    void startup();
    void shutdown();

public:
    void setConfig(const QString &path);
    void setDispatcher(const uDispatcher *dispatcher);    
    void run();

private:
    uDispatcher *m_dispatcher;
    uRequest *m_request;
    uResponse *m_response;

private:
    Uno();
};

#endif // UNO_H
