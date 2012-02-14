#ifndef UUTIL_H
#define UUTIL_H

#include "uno_global.h"

#include <QString>


class UNOSHARED_EXPORT uUtil
{
public:
    static QString hostname();
    static QString uuid();
    static uint timeUTC();

protected:
    uUtil() {}
    uUtil(const uUtil &other);
    uUtil &operator =(const uUtil &other);
};

#endif // UUTIL_H
