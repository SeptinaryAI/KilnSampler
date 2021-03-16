#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMutex>
#include "serial_dev.h"

//饿汉
QSerialPort* serial_dev::serial = new QSerialPort();

/**
 * @brief MainWindow::serial_dev_update 刷新串口
 */
void MainWindow::serial_dev_update(){
    QComboBox* now = ui->dev_portname_set;
    QString get = now->currentText();
    now->clear();//清空候选项
    //更新候选项
    for(QSerialPortInfo port : QSerialPortInfo::availablePorts()){
        now->addItem(port.portName());
    }
    now->addItem("刷新串口");
    now->setCurrentText(get);
}

//各仪器串口连接/断开
static QMutex serial_dev_mutex;
void MainWindow::serial_dev_onoff(){
    QPushButton* btn = ui->dev_onoff;
    btn->setEnabled(false);
    //serial_dev_mutex.lock();
    if(!serial_dev_mutex.tryLock(0)){
        return;
    }
    auto ser = serial_dev::get_serial();
    if(ser->isOpen()){
        ser->close();
        //断开后选项可用
        ui->dev_portname_set->setEnabled(true);
        ui->dev_baudrate_set->setEnabled(true);
        ui->dev_databits_set->setEnabled(true);
        ui->dev_stopbits_set->setEnabled(true);
        ui->dev_parity_set->setEnabled(true);
        ui->dev_onoff->setText("连接");
    } else{
        ser->setPortName(ui->dev_portname_set->currentText());
        bool valid = false;
        for(QSerialPortInfo port : QSerialPortInfo::availablePorts()){
            if(ser->portName() == port.portName())
                valid = true;
        }
        if(valid == false){
            serial_dev_mutex.unlock();
            btn->setEnabled(true);
            return;
        }
        ser->setBaudRate(ui->dev_baudrate_set->currentData().value<QSerialPort::BaudRate>());
        ser->setDataBits(ui->dev_databits_set->currentData().value<QSerialPort::DataBits>());
        ser->setStopBits(ui->dev_stopbits_set->currentData().value<QSerialPort::StopBits>());
        ser->setParity(ui->dev_parity_set->currentData().value<QSerialPort::Parity>());
        bool ret = ser->open(QIODevice::ReadWrite);
        if(ret == false){
            serial_dev_mutex.unlock();
            btn->setEnabled(true);
            return;
        }
        //连接后选项不可用
        ui->dev_portname_set->setEnabled(false);
        ui->dev_baudrate_set->setEnabled(false);
        ui->dev_databits_set->setEnabled(false);
        ui->dev_stopbits_set->setEnabled(false);
        ui->dev_parity_set->setEnabled(false);
        ui->dev_onoff->setText("断开");
    }
    serial_dev_mutex.unlock();
    btn->setEnabled(true);
}
