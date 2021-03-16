#include "IDevice.h"
#include <QMessageBox>
#include <QMap>
#include "device_ellipsometer_classical.h"
#include "device_ellipsometer_new.h"
#include "device_laser_big.h"
#include "device_laser_small.h"
#include <QMutex>
#include "global.h"
//部分设备使用的缓冲区
QByteArray tmpBuf;
//读取缓冲区
QByteArray readBuf;
//饿汉
IDevice* IDevice::instance = new IDevice();//可变的IDevice对象，会被替换为各个具体子类对象指针
IDevice* IDevice::observer = new IDevice();//稳定不变的一个IDevice对象，专门用于emit信号
QSerialPort* IDevice::serial = nullptr;
QMap<int, IDevice*> IDevice::dev_map = QMap<int, IDevice*>();

//QMap<QString, QList<QString>>* IDevice::def_target = new QMap<QString, QList<QString>>();//默认target候选项表，对应到各个设备
//QMap<QString, QList<QString>>* IDevice::def_location = new QMap<QString, QList<QString>>();//默认location候选项表，对应到各个设备

IDevice::IDevice(){
    serial = serial_dev::get_serial();//该设备使用的串口
    connect(this,SIGNAL(serial_write(QByteArray)),this,SLOT(serial_write_slot(QByteArray)));//通过信号槽避免多线程的跨线程问题
    if(this->get_serial() != nullptr)
        connect(this->get_serial(),SIGNAL(readyRead()), this, SLOT(serial_read_slot()));//读取到数据就会触发readyRead
}

void IDevice::dev_map_clear(){
    dev_map.clear();
}

void IDevice::dev_map_add(int i, IDevice* dev){
    dev_map.insert(i,dev);
}

IDevice* IDevice::get_tab_device(int i){
    //字典填充,新增设备请修改此处，和选项卡顺序对应
    if(dev_map.empty()){
        Global::device_choose_init();//填充可选设备，全局
    }
    if(dev_map.contains(i))
        return dev_map[i];
    return NULL;
}

void IDevice::extern_fun(QString cmd){
    if(!serial->isOpen()){
        QMessageBox::warning(nullptr,"Warning","设备串口未打开！",QMessageBox::Ok);
        return;
    }
}
void IDevice::start(){}
void IDevice::sample(){}
void IDevice::end(){}

void IDevice::device_change(IDevice* dev){
    instance = dev;//改变单例指针，指向新的设备
}
//slot 告诉串口发送指令，用于多线程避免跨线程
void IDevice::serial_write_slot(QByteArray bytes){
    serial->write(bytes);
}

void IDevice::serial_read_slot(){
    readBuf.append(serial->readAll());
}

//接受缓冲，并且删掉缓冲
QByteArray IDevice::receive_readbuf(){
    QByteArray tmp;
    if(readBuf.isEmpty()){
        return tmp;
    }
    tmp = std::move(readBuf);
    emit IDevice::get_observer()->buf_show("HEX: "+tmp.toHex());
    readBuf.clear();
    return tmp;
}
//清空读取缓存，保活设备缓存和软件缓存
void IDevice::clear_readbuf(){
    serial->readAll();//读取串口设备所有数据，相当于清除读取缓冲区
    serial->clear();
    readBuf.clear();//清空软件设置的读取缓存区
}
