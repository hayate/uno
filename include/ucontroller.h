#ifndef UCONTROLLER_H
#define UCONTROLLER_H

#include "uno_global.h"

#include <QObject>

class uRequest;
class uResponse;

class UNOSHARED_EXPORT uController : public QObject
{
    Q_OBJECT
public:
    uController(QObject *parent = 0);

protected:
    uResponse *m_response;
    uRequest *m_request;
};

#endif // UCONTROLLER_H
