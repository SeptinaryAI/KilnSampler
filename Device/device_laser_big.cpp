#include "device_laser_big.h"
#include "Logs/logs.h"
extern QByteArray tmpBuf;
static bool first_find_flag = false;//第一次找到符号
//额外功能抽象接口
void device_laser_big::extern_fun(QString cmd){
    IDevice::extern_fun("");
    if(cmd == "ON"){
        QByteArray bytes = QByteArray::fromHex("4c4f0d");
        emit serial_write(bytes);
        logs::write_log("发送‘打开激光’指令");
        emit IDevice::get_observer()->status_show("已发送‘打开激光’指令");
    } else if(cmd == "OFF"){
        QByteArray bytes = QByteArray::fromHex("4c460d");
        emit serial_write(bytes);
        logs::write_log("发送‘关闭激光’指令");
        emit IDevice::get_observer()->status_show("已发送‘关闭激光’指令");
    }
}

void device_laser_big::start(){
    clear_readbuf();
    tmpBuf.clear();
    first_find_flag = false;
    //50HZ连续测量开始 : 'D','X',0x0d
    QByteArray bytes = QByteArray::fromHex("44580d");
    IDevice::get_serial()->write(bytes);
}

//一次采集过程
void device_laser_big::sample(){
    //QByteArray get = IDevice::get_serial()->readAll();
    QByteArray get = receive_readbuf();
    //处理buf
    int i = 0;
    int size = get.count();
    if(size < 1) return;
    //emit IDevice::get_observer()->buf_show("HEX:"+get.toHex());
    int tmp_buf_size = 0;
    while(i < size){
        tmpBuf.append(get.at(i));
        tmp_buf_size = tmpBuf.size();
        if(tmp_buf_size>1 && (uchar)tmpBuf.at(tmp_buf_size-1) == 0x0a && (uchar)tmpBuf.at(tmp_buf_size-2) == 0x0d){
            if(first_find_flag == false){
                first_find_flag = true;
            } else {
                emit IDevice::get_observer()->true_data_show(tmpBuf);
            }
            tmpBuf.clear();
        }
        ++i;
    }
}

void device_laser_big::end(){
    //结束测量 : 0x1b
    QByteArray bytes = QByteArray::fromHex("1b");
    emit serial_write(bytes);
}
