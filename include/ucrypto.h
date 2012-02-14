#ifndef UCRYPTO_H
#define UCRYPTO_H

#include "uno_global.h"
#include <QString>


class UNOSHARED_EXPORT uCrypto
{
public:
    explicit uCrypto(const QString &secret = QString());

public:
    QString encrypt(const QString &data, const QString &secret = QString());
    QString decrypt(const QString &data, const QString &secret = QString());
    QString lastError() const;

protected:

private:
    QString m_error;
    QString m_secret;
};

#endif // UCRYPTO_H
