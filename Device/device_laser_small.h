#ifndef DEVICE_LASER_SMALL_H
#define DEVICE_LASER_SMALL_H
#include "IDevice.h"

class device_laser_small : public IDevice{

public:
    //默认配置
    virtual QSerialPort::BaudRate def_baudrate() override {return QSerialPort::BaudRate::Baud38400;}
    virtual QSerialPort::StopBits def_stopbits() override {return QSerialPort::StopBits::OneStop;}
    virtual QSerialPort::DataBits def_databits() override {return QSerialPort::DataBits::Data8;}
    virtual QSerialPort::Parity def_parity() override {return QSerialPort::Parity::NoParity;}
    virtual int def_period() override {return 10;}//默认采样周期
    virtual QString get_name() override {return "小激光";}
    explicit device_laser_small() : IDevice(){}
    ~device_laser_small(){}
    virtual void extern_fun(QString cmd) override;//额外功能抽象接口
    virtual void start() override;//采样开始
    virtual void sample() override;//一次采集过程
    virtual void end() override;//采样结束

};

#endif
