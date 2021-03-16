#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::device_init(){
    device_choose();
}

//改变串口设备选择
void MainWindow::device_choose(){
    int index = ui->tab_dev->currentIndex();
    IDevice* get = IDevice::get_tab_device(index);
    if(get != nullptr){
        IDevice::device_change(get);
        QString dev_name = IDevice::get_instance()->get_name();
        logs::write_log("当前选择的设备是: "+dev_name);
        ui->statusBar->showMessage("当前选择的设备是: "+dev_name);
        //关闭打开的设备串口
        if(serial_dev::get_serial()->isOpen())
            serial_dev_onoff();
        //读取串口设备的默认配置串口参数配置，并写入串口设置下拉框中
        for(int i = 0 ; i < ui->dev_baudrate_set->count() ; ++i){
            if(ui->dev_baudrate_set->itemData(i).value<QSerialPort::BaudRate>() == IDevice::get_instance()->def_baudrate())
                ui->dev_baudrate_set->setCurrentIndex(i);
        }
        for(int i = 0 ; i < ui->dev_databits_set->count() ; ++i){
            if(ui->dev_databits_set->itemData(i).value<QSerialPort::DataBits>() == IDevice::get_instance()->def_databits())
                ui->dev_databits_set->setCurrentIndex(i);
        }
        for(int i = 0 ; i < ui->dev_stopbits_set->count() ; ++i){
            if(ui->dev_stopbits_set->itemData(i).value<QSerialPort::StopBits>() == IDevice::get_instance()->def_stopbits())
                ui->dev_stopbits_set->setCurrentIndex(i);
        }
        for(int i = 0 ; i < ui->dev_parity_set->count() ; ++i){
            if(ui->dev_parity_set->itemData(i).value<QSerialPort::Parity>() == IDevice::get_instance()->def_parity())
                ui->dev_parity_set->setCurrentIndex(i);
        }
    }
}

