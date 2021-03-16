#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMutex>
#include "serial_bool.h"

//饿汉
QSerialPort* serial_bool::serial = new QSerialPort();

/**
 * @brief MainWindow::serial_bool_update 刷新串口
 */
void MainWindow::serial_bool_update(){
    QComboBox* now = ui->bool_portname_set;
    QString get = now->currentText();
    now->clear();//清空候选项
    //更新候选项
    for(QSerialPortInfo port : QSerialPortInfo::availablePorts()){
        now->addItem(port.portName());
    }
    now->addItem("刷新串口");
    now->setCurrentText(get);
}

/**
 * @brief serial_bool_mutex 霍尔串口连接/断开
 */
static QMutex serial_bool_mutex;
void MainWindow::serial_bool_onoff(){
    QPushButton* btn = ui->bool_onoff;
    btn->setEnabled(false);
    //serial_bool_mutex.lock();
    if(!serial_bool_mutex.tryLock(0)){
        return;
    }
    auto ser = serial_bool::get_serial();
    if(ser->isOpen()){
        ser->close();
        //断开后选项可用
        ui->bool_portname_set->setEnabled(true);
        ui->bool_baudrate_set->setEnabled(true);
        ui->bool_databits_set->setEnabled(true);
        ui->bool_stopbits_set->setEnabled(true);
        ui->bool_parity_set->setEnabled(true);
        ui->bool_onoff->setText("连接");
    } else{
        ser->setPortName(ui->bool_portname_set->currentText());
        bool valid = false;
        for(QSerialPortInfo port : QSerialPortInfo::availablePorts()){
            if(ser->portName() == port.portName())
                valid = true;
        }
        if(valid == false){
            serial_bool_mutex.unlock();
            btn->setEnabled(true);
            return;
        }
        ser->setBaudRate(ui->bool_baudrate_set->currentData().value<QSerialPort::BaudRate>());
        ser->setDataBits(ui->bool_databits_set->currentData().value<QSerialPort::DataBits>());
        ser->setStopBits(ui->bool_stopbits_set->currentData().value<QSerialPort::StopBits>());
        ser->setParity(ui->bool_parity_set->currentData().value<QSerialPort::Parity>());
        bool ret = ser->open(QIODevice::ReadWrite);
        if(ret == false){
            serial_bool_mutex.unlock();
            btn->setEnabled(true);
            return;
        }
        //连接后选项不可用
        ui->bool_portname_set->setEnabled(false);
        ui->bool_baudrate_set->setEnabled(false);
        ui->bool_databits_set->setEnabled(false);
        ui->bool_stopbits_set->setEnabled(false);
        ui->bool_parity_set->setEnabled(false);
        ui->bool_onoff->setText("断开");
    }
    serial_bool_mutex.unlock();
    btn->setEnabled(true);
}
