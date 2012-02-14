#include <time.h>
#include <QUuid>
#include <QDebug>
#include <QDateTime>
#include <QHostInfo>
#include "uutil.h"

#ifdef Q_OS_WIN
#include <Winsock2.h>
#else
#include <unistd.h>
#endif


QString uUtil::hostname()
{
    // hostnames cannot be longer than 255 characters + NL
    char buf[256] = {"\0"};
    if (0 == gethostname(buf, 256))
    {
        return buf;
    }
    return QHostInfo::localHostName();
}

QString uUtil::uuid()
{
    QString uuid = QUuid::createUuid().toString();
    return uuid.mid(1, uuid.size() - 2); // remove curly brackets
}

uint uUtil::timeUTC()
{
    time_t rawtime;    
    struct tm *ptm;
    time(&rawtime);
    ptm = gmtime(&rawtime);
    return mktime(ptm);
}
