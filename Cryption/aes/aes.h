#ifndef AES_H
#define AES_H

#include <QDebug>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <openssl/aes.h>

class aes
{
public:
    static QByteArray iv;//向量
    static QByteArray key;//密钥
    /**
    * @brief aes128_encrypt aes对称加密： 128位cbc模式加密,PKCS5Padding
    * @param in 明文
    * @param out 加密后的密文
    * @param key 密钥
    * @return 运行状态  0:ok  1:err
    */
    static int aes128_encrypt(QByteArray in, QByteArray& out);
    /**
    * @brief aes128_decrypt aes对称加密： 128位cbc模式解密,PKCS5Padding
    * @param in 密文
    * @param out 解密后的明文
    * @param key 密钥
    * @return 运行状态 0:ok  1:err
    */
    static int aes128_decrypt(QByteArray in, QByteArray& out);
    /**
    * @brief 测试函数
    */
    static void test();
};
#endif // RSA_H
