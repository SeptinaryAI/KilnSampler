#include "aes.h"
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
QByteArray aes::iv = "#VPIJ1t8wO6s#@$s";//向量
QByteArray aes::key = "1234567890abcdef";//密钥
/**
 * @brief char_to_QByteArray
 * @param c  char[]
 * @param len   多少字节（\0也不能忽略）
 * @return status
 */
static QByteArray char_to_QByteArray(char* c, int len){
    QByteArray ret;
    for(int i = 0 ; i < len ; ++i){
        ret.append(c[i]);
    }
    return ret;
}

/**
 * @brief aes::aes128_encrypt
 * @param in
 * @param out
 * @return status
 * @remark PKCS5Padding填充模式：数据字节数不满16的倍数，则用不满的字节数的十进制数填充至16的倍数。如果满16的倍数，则填充16个16
 * @remark 例如 "1234567890" 则在PKCS5Padding模式下填充为 "1234567890\0x06\0x06\0x06\0x06\0x06\0x06" 因为字符串大小为10  16-10=6，后面全填充 6
 * @remark 例如 "1234567890abcdef" 在PKCS5Padding模式下填充为 "1234567890abcdef\0x10\0x10\0x10\0x10\0x10\0x10\0x10\0x10\0x10\0x10\0x10\0x10\0x10\0x10\0x10\0x10"
 * @remark 因为字符串大小为16，是16的倍数，因此填充16个 16，16的Hex表示为 0x10
 */
int aes::aes128_encrypt(QByteArray in, QByteArray& out)   //加密
{
    int len = in.length();
    AES_KEY* aes = new AES_KEY();
    //先得到设置的iv，key的向量的字节数据
    auto iv_bytes = aes::iv.data();
    auto key_bytes = aes::key.data();
    uchar iv_[AES_BLOCK_SIZE];
    uchar key_[AES_BLOCK_SIZE];
    memset(iv_,'0',sizeof(iv_));//Java Web端的不满16位的向量都是用'0'字符填充
    memset(key_,' ',sizeof(key_));//Java Web端的不满16位的密钥都是用' '字符填充
    for(int i = 0 ; i < AES_BLOCK_SIZE; ++i){
        iv_[i] = i < aes::iv.length() ? (uchar)iv_bytes[i] : iv_[i];
        key_[i] = i < aes::key.length() ? (uchar)key_bytes[i] : key_[i];
    }
    if (AES_set_encrypt_key(key_, 128, aes) < 0)
        return 1;
    int in_len = AES_BLOCK_SIZE * (len / AES_BLOCK_SIZE + 1);
    char* out_ = new char[in_len];
    memset(out_,'\0',(size_t)in_len);
    char* in_ = new char[in_len];
    memset(in_,in_len - len,(size_t)in_len);
    for (int i= 0; i < len; ++i) {
        in_[i] = in.at(i);
    }
    AES_cbc_encrypt((uchar*)in_, (uchar*)out_, (size_t)in_len, aes, iv_, AES_ENCRYPT);
    out = char_to_QByteArray(out_, in_len);//openssl中aes cbc算法in out的大小相同，这里为了不忽略'\0'，直接指定大小进行拷贝
    out = out.toBase64();//转base64便于编码兼容
    delete aes;
    delete[] out_;
    delete[] in_;
    return 0;
}
/**
 * @brief aes::aes128_decrypt
 * @param in
 * @param out
 * @return status
 * @remark PKCS5Padding填充模式,直接根据最后一个Byte代表的数的大小决定删去末尾多少个Bytes
 */
int aes::aes128_decrypt(QByteArray in, QByteArray& out)   //解密
{
    in = QByteArray::fromBase64(in);
    int len = in.length();
    if(len < 1) len = 1;//保证len不会小于1
    AES_KEY *aes = new AES_KEY();//防止cbc模式解密受到前一次的干扰，每次生成新对象
    //先得到设置的iv，key的向量的字节数据
    auto iv_bytes = aes::iv.data();
    auto key_bytes = aes::key.data();
    uchar iv_[AES_BLOCK_SIZE];
    uchar key_[AES_BLOCK_SIZE];
    memset(iv_,'0',sizeof(iv_));//Java Web端的不满16位的向量都是用'0'字符填充
    memset(key_,' ',sizeof(key_));//Java Web端的不满16位的密钥都是用' '字符填充
    for(int i = 0 ; i < AES_BLOCK_SIZE ; ++i){
        iv_[i] = i < aes::iv.length() ? (uchar)iv_bytes[i] : iv_[i];
        key_[i] = i < aes::key.length() ? (uchar)key_bytes[i] : key_[i];
    }
    if (AES_set_decrypt_key(key_, 128, aes) < 0)
        return 1;
    char* out_ = new char[len];
    memset(out_,'\0',(size_t)len);
    AES_cbc_encrypt((uchar*)in.data(), (uchar*)out_, (size_t)len, aes, iv_, AES_DECRYPT);
    out = char_to_QByteArray(out_, len);//openssl中aes cbc算法in out的大小相同，这里为了不忽略'\0'，直接指定大小进行拷贝
    int to_del = (uint)out.at(out.length()-1);//要删除几个byte
    int start_del = out.length() - to_del;//开始删除的位置
    out = out.remove(start_del,to_del);
    delete aes;
    delete[] out_;
    return 0;
}

void aes::test(){
    QByteArray get2;
    aes::key = "1234567890abcdef";
    QByteArray mi = "wv4oHUvMyMZy7NTDsGdc/NsXmP9OK2TPtrDPmH6z1mAFpZHVmsQstKR/x/5VLFOE8sFVkNZZZHpiDIcaX6xZU1Nn5X0zbF5A8rutP2PdIz+PvFw6tat0s4CAx3vbdA2qy127defM97FjTUhVsoQPxoNFBeSEzop5t3VNa1+vB/ecZHLhgxLw9eREysNtkZ984veYzEuBmGRpp5p+ObtikeTnXwm9f2/FFb4pSAhhpWBAdmYNwMd18IqrtY7Biowzh7zcKvQ8Za7lDaFCOlnWiUMdeHo3+r1xbpkAaRQizP2avCtCULuJOXpwHz3R1Tcfe65G4L3JQFt31+1S80g5hccSr/2tvFpR1+wbTYGmsxPn7pHeM2gjsD1ZFQ/CIdeQLVEGEU1WcU0DXe/vJM6Y1NIrHGGfVEiz9bYs4kq1/FVIR+AwEhoQzzBhrFj1/A6xN+eVhG1W3RMcjEWanDvQzvJtAWwn850ysXWGiCnfs9rq+G/3JbV61aTmLcjxqZoLJsUvewMhciPXOsmvcUMXGakXrtg968ktjTSgUx7R3OjWr7fiSLsr5tEwFYHKuqNUv4PfsLFdlMeM9oA0M2XRPZwG/WdvynBiP85hw0/TSqq0HqcevWtJKkM9odUycm+T+YZxyL+4X33n5ihHs0+u62JOnxS1CeOv5hf+LviGIIEGKjEABRAq6q7KbC9e6sDGrOobbHNzTE19Vns+wSG4HwARSrdTSJ4WZ1AWYJZyxha+bTTzpj5MPiy1uFJLWgq+pZyVIpDruKiGC+PwftqlTeo8oDFbmKs5ObMGZkeVGBNdYwDAXhY+Dwoiksem/6vdGc7s3nLjJGhvdo1PZ8kYqeYeUVWJCG25QT8lne4zWh/gAeaiLuNJkp12zu1quige";
    aes128_decrypt(mi,get2);
    qDebug()<<"out:     "<< QString(get2)<<endl;
    qDebug()<<"out:     "<< QString::fromUtf8(get2)<<endl;
    qDebug()<<"out hex:     "<< QString::fromUtf8(get2.toHex())<<endl;

}
