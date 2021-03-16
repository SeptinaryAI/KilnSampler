#include "device_ellipsometer_new.h"
#include <QThread>
#include <QDateTime>
extern QByteArray tmpBuf;
//额外功能抽象接口
void device_ellipsometer_new::extern_fun(QString cmd){

}

void device_ellipsometer_new::start(){
    clear_readbuf();
    tmpBuf.clear();//清buf
}
static QByteArray once = QByteArray::fromHex("010300000002c40b");//测量一次的指令
//一次采集过程
void device_ellipsometer_new::sample(){
    emit serial_write(once);//发送读取指令
    QByteArray get;
    //get.append(tmpBuf);
    qint64 start = QDateTime::currentMSecsSinceEpoch();
    while(get.count() < 9 || ((uchar)get.at(1) != 0x03) || ((uchar)get.at(2) != 0x04) ){
        //超时判断
        if(QDateTime::currentMSecsSinceEpoch() - start > def_period()){
            emit IDevice::get_observer()->status_show("获取超时！");
            //emit serial_write(once);//发送读取指令
            //tmpBuf.clear();
            //tmpBuf.append(get);
            return;
        }
        //get.append(IDevice::get_serial()->readAll());
        get.append(receive_readbuf());
    }
    //emit IDevice::get_observer()->buf_show("HEX:"+get.toHex());
//    double ret = get.at(0) == 0x00 ?
//                (double)((uchar)get.at(5) * 0x100 + (uchar)get.at(6)) / 1000.0:
//                (double)(-((uchar)get.at(5) * 0x100 + (uchar)get.at(6))) / 1000.0;
    double ret = (double)((uchar)get.at(5) * 0x100 + (uchar)get.at(6)) / 1000.0;//不需要符号位
    QByteArray ret_str = QString::number(ret,'f',3).toUtf8();
    emit IDevice::get_observer()->true_data_show(ret_str);
}

void device_ellipsometer_new::end(){

}
