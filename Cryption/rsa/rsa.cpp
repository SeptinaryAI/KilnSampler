#include "rsa.h"

QString rsa::pkey = "-----BEGIN PUBLIC KEY-----\r\n"
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCFBu7Nd/uKm0X2AsuwmQiClQUFEULqInI8dP5HWI2emI75q6IYC2dbyHpSo4ZpFf7foqGpifeN7E+UmYgW3lKc8DYr+C37egJq0RTfltSv3QdwRJAtVZYNkh1fkiVTmLuQpPvro0wjzJbYsJSlvqRE4tJ3blelxZxCPcPEenlZtQIDAQAB\r\n"
        "-----END PUBLIC KEY-----\r\n";
/**
* @brief createRsaKey 生成秘钥对
* @param strPubKey 公钥
* @param strPriKey 私钥
* @return 成功状态
*/
bool rsa::createRsaKey (QString& strPubKey, QString& strPriKey)
{
     RSA *pRsa = RSA_generate_key(KEY_LENGTH, RSA_3, nullptr, nullptr);
     if (!pRsa){
         return false;
     }
     BIO *pPriBio = BIO_new(BIO_s_mem());
     PEM_write_bio_RSAPrivateKey(pPriBio, pRsa, nullptr, nullptr, 0, nullptr, nullptr);
     BIO *pPubBio = BIO_new(BIO_s_mem());
     PEM_write_bio_RSAPublicKey(pPubBio, pRsa);
     // 获取长度
     size_t nPriKeyLen = BIO_pending(pPriBio);
     size_t nPubKeyLen = BIO_pending(pPubBio);
     // 密钥对读取到字符串
     char* pPriKey = new char[nPriKeyLen];
     char* pPubKey = new char[nPubKeyLen];
     BIO_read(pPriBio, pPriKey, nPriKeyLen);
     BIO_read(pPubBio, pPubKey, nPubKeyLen);
     // 存储密钥对
     strPubKey = QByteArray(pPubKey, nPubKeyLen);
     strPriKey = QByteArray(pPriKey, nPriKeyLen);
     // 内存释放
     RSA_free(pRsa);
     BIO_free_all(pPriBio);
     BIO_free_all(pPubBio);
     delete[] pPriKey;
     delete[] pPubKey;
     return true;
}

/**
 * @brief rsa_pri_encrypt 私钥加密
 * @param strClearData 明文
 * @param strPriKey 私钥
 * @return 加密后数据(base64格式)
 */
QByteArray rsa::rsa_pri_encrypt_base64 (const QString& strClearData, const QString& strPriKey)
{
    QByteArray priKeyArry = strPriKey.toUtf8();
    uchar* pPriKey = (uchar*)priKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPriKey, strPriKey.length());
    if (pKeyBio == nullptr){
        return "";
    }
    RSA* pRsa = RSA_new();
    pRsa = PEM_read_bio_RSAPrivateKey(pKeyBio, &pRsa, nullptr, nullptr);
    if ( pRsa == nullptr ){
         BIO_free_all(pKeyBio);
         return "";
    }
    int nLen = RSA_size(pRsa);//rsa单次len
    int padding = nLen - 11;//如果是rsa128，则一次加密只能加密128 - 11 = 117个字符
    QByteArray clearDataArry = strClearData.toUtf8();
    int nClearDataLen = clearDataArry.length();//加密数据len
    uchar* pClearData = (uchar*)clearDataArry.data();//加密数据
    int slice = nClearDataLen / padding + (nClearDataLen%padding ? 1 : 0);//分片
    QByteArray strEncryptData = "";
    for (int i = 0; i < slice; ++i) {
        char* pEncryptBuf = new char[nLen];
        memset(pEncryptBuf, 0, nLen);
        int remain = nClearDataLen - i * padding;//剩余字节数
        int nSize = RSA_private_encrypt(remain > padding ? padding : remain,
                                       pClearData + i * padding,
                                       (uchar*)pEncryptBuf,
                                       pRsa,
                                       RSA_PKCS1_PADDING);

        if (nSize >= 0 ){
            strEncryptData += QByteArray(pEncryptBuf, nSize);
        }
        // 释放内存
        delete[] pEncryptBuf;
    }

    BIO_free_all(pKeyBio);
    RSA_free(pRsa);
    return strEncryptData.toBase64();
}

/**
 * @brief rsa_pub_decrypt 公钥解密
 * @param strDecrypt 待解密数据(base64格式)
 * @param strPubKey 公钥
 * @return 明文
 */
QByteArray rsa::rsa_pub_decrypt_base64(const QString& strDecryptData, const QString& strPubKey)
{
    QByteArray pubKeyArry = strPubKey.toUtf8();
    uchar* pPubKey = (uchar*)pubKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPubKey, strPubKey.length());
    if (pKeyBio == nullptr){
        return "";
    }

    RSA* pRsa = RSA_new();
    if ( strPubKey.contains(BEGIN_RSA_PUBLIC_KEY) ){
        pRsa = PEM_read_bio_RSAPublicKey(pKeyBio, &pRsa, nullptr, nullptr);
    }else{
        pRsa = PEM_read_bio_RSA_PUBKEY(pKeyBio, &pRsa, nullptr, nullptr);
    }

    if ( pRsa == nullptr ){
        BIO_free_all(pKeyBio);
        return "";
    }
    int nLen = RSA_size(pRsa);
    int padding = nLen;//解密时以RSA_size为单位

    //解密
    QByteArray decryptDataArry = strDecryptData.toUtf8();
    decryptDataArry = QByteArray::fromBase64(decryptDataArry);
    int nDecryptDataLen = decryptDataArry.length();
    uchar* pDecryptData = (uchar*)decryptDataArry.data();
    int slice = nDecryptDataLen / padding + (nDecryptDataLen%padding ? 1 : 0);//分片
    QByteArray strClearData = "";

    for (int i = 0; i < slice; ++i) {
        char* pClearBuf = new char[nLen];
        memset(pClearBuf, 0, nLen);
        int nSize = RSA_public_decrypt(padding,
                                        pDecryptData + i * padding,
                                        (uchar*)pClearBuf,
                                        pRsa,
                                        RSA_PKCS1_PADDING);

        if ( nSize >= 0 ){
            strClearData += QByteArray(pClearBuf, nSize);
        }
        // 释放内存
        delete[] pClearBuf;
    }

    BIO_free_all(pKeyBio);
    RSA_free(pRsa);
    return strClearData;
}

/**
 * @brief rsa_pub_encrypt 公钥加密
 * @param strClearData 明文
 * @param strPubKey 私钥
 * @return 加密后数据(base64格式)
 */
QByteArray rsa::rsa_pub_encrypt_base64 (const QString& strClearData, const QString& strPubKey)
{
    QByteArray pubKeyArry = strPubKey.toUtf8();
    uchar* pPubKey = (uchar*)pubKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPubKey, pubKeyArry.length());
    if (pKeyBio == nullptr){
        return "";
    }
    RSA* pRsa = RSA_new();
    if (strPubKey.contains(BEGIN_RSA_PUBLIC_KEY) ){
        pRsa = PEM_read_bio_RSAPublicKey(pKeyBio, &pRsa, nullptr, nullptr);
    }else{
        pRsa = PEM_read_bio_RSA_PUBKEY(pKeyBio, &pRsa, nullptr, nullptr);
    }
    if (pRsa == nullptr ){
        BIO_free_all(pKeyBio);
        return "";
    }

    int nLen = RSA_size(pRsa);//rsa单次len
    int padding = nLen - 11;//如果是rsa128，则一次加密只能加密128 - 11 = 117个字符
    QByteArray clearDataArry = strClearData.toUtf8();
    int nClearDataLen = clearDataArry.length();//加密数据len
    uchar* pClearData = (uchar*)clearDataArry.data();//加密数据
    int slice = nClearDataLen / padding + (nClearDataLen%padding ? 1 : 0);//分片

    QByteArray strEncryptData = "";
    for (int i = 0; i < slice; ++i) {
        char* pEncryptBuf = new char[nLen];
        memset(pEncryptBuf, 0, nLen);
        int remain = nClearDataLen - i * padding;//剩余字节数
        int nSize = RSA_public_encrypt(remain > padding ? padding : remain,
                                       pClearData + i * padding,
                                       (uchar*)pEncryptBuf,
                                       pRsa,
                                       RSA_PKCS1_PADDING);

        if (nSize >= 0 ){
            strEncryptData += QByteArray(pEncryptBuf, nSize);
        }
        // 释放内存
        delete[] pEncryptBuf;
    }

    BIO_free_all(pKeyBio);
    RSA_free(pRsa);
    return strEncryptData.toBase64();
}
QByteArray rsa::rsa_pub_encrypt_base64 (const QByteArray& strClearData, const QString& strPriKey){
    return rsa_pub_encrypt_base64(QString::fromUtf8(strClearData), strPriKey);
}
/**
 * @brief rsa_pri_decrypt 私钥解密
 * @param strDecrypt 待解密数据(base64格式)
 * @param strPriKey 私钥
 * @return 明文
 */
QByteArray rsa::rsa_pri_decrypt_base64(const QString& strDecryptData, const QString& strPriKey)
{
    QByteArray priKeyArry = strPriKey.toUtf8();
    uchar* pPriKey = (uchar*)priKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPriKey, priKeyArry.length());
    if (pKeyBio == nullptr){
        return "";
    }
    RSA* pRsa = RSA_new();
    pRsa = PEM_read_bio_RSAPrivateKey(pKeyBio, &pRsa, nullptr, nullptr);
    if (pRsa == nullptr ){
        BIO_free_all(pKeyBio);
        return "";
    }
    int nLen = RSA_size(pRsa);
    int padding = nLen;//解密时以RSA_size为单位

    //解密
    QByteArray decryptDataArry = strDecryptData.toUtf8();
    decryptDataArry = QByteArray::fromBase64(decryptDataArry);
    int nDecryptDataLen = decryptDataArry.length();
    uchar* pDecryptData = (uchar*)decryptDataArry.data();
    int slice = nDecryptDataLen / padding + (nDecryptDataLen%padding ? 1 : 0);//分片
    QByteArray strClearData = "";

    for (int i = 0; i < slice; ++i) {
        char* pClearBuf = new char[nLen];
        memset(pClearBuf, 0, nLen);
        int nSize = RSA_private_decrypt(padding,
                                        pDecryptData + i * padding,
                                        (uchar*)pClearBuf,
                                        pRsa,
                                        RSA_PKCS1_PADDING);

        if ( nSize >= 0 ){
            strClearData += QByteArray(pClearBuf, nSize);
        }
        // 释放内存
        delete[] pClearBuf;
    }


    BIO_free_all(pKeyBio);
    RSA_free(pRsa);
    return strClearData;
}
QByteArray rsa::rsa_pri_decrypt_base64 (const QByteArray& strDecryptData, const QString& strPriKey){
    return rsa_pri_decrypt_base64(QString::fromUtf8(strDecryptData), strPriKey);
}

void rsa::test()
{
    
}
