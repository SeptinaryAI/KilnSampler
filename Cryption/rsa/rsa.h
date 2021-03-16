#ifndef RSA_H
#define RSA_H

#include <QDebug>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

// define rsa public key
#define BEGIN_RSA_PUBLIC_KEY    "BEGIN RSA PUBLIC KEY"
#define BEGIN_PUBLIC_KEY        "BEGIN PUBLIC KEY"
#define KEY_LENGTH              1024                        // 密钥长度

//记录了传给java端时，Qt的身份认证码，所有Qt采集端都使用这个码
#define CONST_AUTH              "1fc5a4b8652c0d3f"

class rsa
{
public:
    static QString pkey;//固定的公钥
/**
* @brief createRsaKey 生成秘钥对
* @param strPubKey 公钥
* @param strPriKey 私钥
* @return 状态
*/
static bool createRsaKey (QString& strPubKey, QString& strPriKey);
/**
* @brief rsa_pri_encrypt 私钥加密
* @param strClearData 明文
* @param strPriKey 私钥
* @return 加密后数据(base64格式)
*/
static QByteArray rsa_pri_encrypt_base64 (const QString& strClearData, const QString& strPriKey);
/**
* @brief rsa_pub_decrypt 公钥解密
* @param strDecrypt 待解密数据(base64格式)
* @param strPubKey 公钥
* @return 明文
*/
static QByteArray rsa_pub_decrypt_base64 (const QString& strDecryptData, const QString& strPubKey);
/**
* @brief rsa_pub_encrypt 公钥加密
* @param strClearData 明文
* @param strPubKey 私钥
* @return 加密后数据(base64格式)
*/
static QByteArray rsa_pub_encrypt_base64 (const QByteArray& strClearData, const QString& strPriKey);
static QByteArray rsa_pub_encrypt_base64 (const QString& strClearData, const QString& strPubKey);
/**
* @brief rsa_pri_decrypt 私钥解密
* @param strDecrypt 待解密数据(base64格式)
* @param strPriKey 私钥
* @return 明文
*/
static QByteArray rsa_pri_decrypt_base64(const QByteArray& strDecryptData, const QString& strPriKey);
static QByteArray rsa_pri_decrypt_base64 (const QString& strDecryptData, const QString& strPriKey);
/**< 测试 */
static void test ();
};
#endif // RSA_H
