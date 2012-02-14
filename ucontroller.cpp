#include "urequest.h"
#include "uresponse.h"
#include "ucontroller.h"


uController::uController(QObject *parent) :
    QObject(parent), m_response(0), m_request(0)
{
    m_response = &uResponse::getInstance();
    m_request = &uRequest::getInstance();
}
