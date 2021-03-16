#ifndef SERIAL_BOOL_H
#define SERIAL_BOOL_H

#include <QtSerialPort/QSerialPort>

class serial_bool{

private:
    static QSerialPort* serial;

public:
    serial_bool(){}
    static QSerialPort* get_serial(){return serial;}
};



#endif // SERIAL_BOOL_H
