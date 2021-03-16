#include "device_ellipsometer_classical.h"
extern QByteArray tmpBuf;
//额外功能抽象接口
void device_ellipsometer_classical::extern_fun(QString cmd){

}
static bool first_find_flag = false;//第一次找到符号
void device_ellipsometer_classical::start(){
    clear_readbuf();
    tmpBuf.clear();//清buf
    first_find_flag = false;
}

//一次采集过程
//经典款椭圆仪不以\r\n表示一次数据结束，直接发送表示数据的字符串
//格式形如"-0043.23"
//以"-" "+"符号分割数据来进行数据截取
void device_ellipsometer_classical::sample(){
    //QByteArray get = IDevice::get_serial()->readAll();
    QByteArray get = receive_readbuf();
    int i = 0;
    int size = get.count();
    if(size < 1) return;
    //找到符号
    while(i < size){
        if(get.at(i) == '-' || get.at(i) == '+'){
            if(first_find_flag == false){
                first_find_flag = true;
            } else {
                emit IDevice::get_observer()->true_data_show(tmpBuf);
            }
            //emit IDevice::get_observer()->buf_show("HEX:"+tmpBuf.toHex());
            tmpBuf.clear();
            ++i;
            continue;//不需要符号位
        }
        tmpBuf.append(get.at(i));
        ++i;
    }
}

void device_ellipsometer_classical::end(){

}
