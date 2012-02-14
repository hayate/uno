#ifndef UERROR_H
#define UERROR_H

#include "uno_global.h"

#include "uresponse.h"
#include "ucontroller.h"


class UNOSHARED_EXPORT uError : public uController
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit uError(const QString &error, const int code = 0, QObject *parent = 0);
    Q_INVOKABLE explicit uError(QObject *parent = 0);

public:
    Q_INVOKABLE void indexAction();

public:
    void setError(const QString &error, const int code = 0);
    QString error() const;
    int code() const;

private:
    QString m_error;
    int m_code;
};

#endif // UERROR_H
