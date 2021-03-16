#ifndef SERIAL_DEV_H
#define SERIAL_DEV_H

#include <QtSerialPort/QSerialPort>

class serial_dev{

private:
    static QSerialPort* serial;

public:
    serial_dev(){}
    static QSerialPort* get_serial(){return serial;}
};



#endif // SERIAL_DEV_H
