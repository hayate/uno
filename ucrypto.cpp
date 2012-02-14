#include <QDebug>
#include <QObject>
#include "ucrypto.h"

#ifndef QT_NO_OPENSSL
#include <openssl/evp.h>
#endif

uCrypto::uCrypto(const QString &secret) :
    m_error(), m_secret(secret)
{

}

QString uCrypto::encrypt(const QString &data, const QString &secret)
{    
#ifndef QT_NO_OPENSSL

    if (m_secret.isEmpty())
    {
        if (secret.isEmpty())
        {
            m_error = QObject::tr("Missing a valid secret to encrypt the data with.");
            return "";
        }
        m_secret = secret;
    }
    int count = 3;
    unsigned char key[32] = {0};
    unsigned char iv[32] = {0};
    const unsigned char *s = (const unsigned char*)m_secret.toStdString().c_str();

    if (0 == EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), NULL, s, m_secret.size(), count, key, iv))
    {
        m_error = QObject::tr("Could not extract a valid IV and Key from the passed secret.");
        return "";
    }
    EVP_CIPHER_CTX cctx;
    EVP_CIPHER_CTX_init(&cctx);
    EVP_EncryptInit_ex(&cctx, EVP_aes_256_cbc(), NULL, key, iv);

    int size = EVP_CIPHER_CTX_block_size(&cctx) + data.size();
    unsigned char *cipher = (unsigned char*)malloc(size);
    memset(cipher, 0, size);
    int ol = 0;
    const unsigned char *in = (const unsigned char*)data.toStdString().c_str();
    size = 0;

    EVP_EncryptUpdate(&cctx, &cipher[ol], &size, &in[ol], data.size());
    ol += size;

    if (0 != EVP_EncryptFinal_ex(&cctx, &cipher[ol], &size))
    {
        ol += size;
    }
    EVP_CIPHER_CTX_cleanup(&cctx);
    QByteArray buf((const char*)cipher, ol);
    free(cipher);    
    return buf.toBase64().trimmed();

#else
    m_error = QObject::tr("Crypto class requires Qt with OpenSSL enabled");
    return "";
#endif
}

QString uCrypto::decrypt(const QString &data, const QString &secret)
{
#ifndef QT_NO_OPENSSL

    if (m_secret.isEmpty())
    {
        if (secret.isEmpty())
        {
            m_error = QObject::tr("Missing a valid secret to encrypt the data with.");
            return "";
        }
        m_secret = secret;
    }
    int count = 3;
    unsigned char key[32] = {0};
    unsigned char iv[32] = {0};
    const unsigned char *s = (const unsigned char*)m_secret.toStdString().c_str();

    if (0 == EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), NULL, s, m_secret.size(), count, key, iv))
    {
        m_error = QObject::tr("Could not extract a valid IV and Key from the passed secret.");
        return "";
    }
    EVP_CIPHER_CTX cctx;
    EVP_CIPHER_CTX_init(&cctx);
    EVP_DecryptInit_ex(&cctx, EVP_aes_256_cbc(), NULL, key, iv);

    QByteArray cipher(QByteArray::fromBase64(data.toAscii()));

    int size = cipher.size() + EVP_CIPHER_CTX_block_size(&cctx);
    unsigned char *plaintext = (unsigned char*)malloc(size);
    memset(plaintext, 0, size);
    const unsigned char *cf = (const unsigned char*)cipher.constData();

    int len = 0;
    int ol = 0;
    EVP_DecryptUpdate(&cctx, &plaintext[ol], &len, &cf[ol], cipher.size());
    ol += len;

    if (0 != EVP_DecryptFinal_ex(&cctx, &plaintext[ol], &len))
    {
        ol += len;
    }
    EVP_CIPHER_CTX_cleanup(&cctx);

    QByteArray pt((const char*)plaintext, ol);
    free(plaintext);
    return pt.trimmed();

#else
    qDebug() << QObject::tr("Crypto class requires Qt with OpenSSL enabled");
    return "";
#endif
}
