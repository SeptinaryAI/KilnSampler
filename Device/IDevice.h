#ifndef IDEVICE_H
#define IDEVICE_H
#include "Serial/serial_dev.h"
#include <QString>

//设备接口
class IDevice : public QObject{
    Q_OBJECT
private:
    static QSerialPort* serial;
    static IDevice* observer;//全局唯一常驻稳定不变的IDevice对象，用于发送信号SIGNAL，
    static IDevice* instance;//全局唯一常驻可变的IDevice对象，可被替换为各个具体设备子类对象，
    static QMap<int, IDevice*> dev_map;
public:
    //默认配置
    virtual QString get_name() {return "base";}
    virtual QSerialPort::BaudRate def_baudrate(){return QSerialPort::BaudRate::Baud9600;}
    virtual QSerialPort::StopBits def_stopbits(){return QSerialPort::StopBits::OneStop;}
    virtual QSerialPort::DataBits def_databits(){return QSerialPort::DataBits::Data8;}
    virtual QSerialPort::Parity def_parity(){return QSerialPort::Parity::NoParity;}
    virtual int def_period(){return 10;}//默认采样周期
    //单例
    static QSerialPort* get_serial(){return serial;}
    static IDevice* get_instance(){return instance;}
    static IDevice* get_observer(){return observer;}
    //构造析构
    explicit IDevice();
    virtual ~IDevice(){}
    //功能
    static void dev_map_clear();
    static void dev_map_add(int, IDevice*);
    static IDevice* get_tab_device(int i);
    static void device_change(IDevice* dev);//可使用其他设备替换单例

    virtual void extern_fun(QString cmd);//额外功能抽象接口
    virtual void start();//采集开始
    virtual void sample();//一次采集过程
    virtual void end();//采集结束

    static void clear_readbuf();//清空读取缓存，保活设备缓存和软件缓存
    static QByteArray receive_readbuf();//获取缓冲区所有数据
signals:
    void buf_show(QByteArray buf);//缓冲区信号
    void true_data_show(QByteArray data);//实际数据
    void status_show(QString msg);//发送到状态栏的信息
    void serial_write(QByteArray bytes);
public slots:
    void serial_write_slot(QByteArray bytes);
    void serial_read_slot();

};

#endif
