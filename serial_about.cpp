#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Chart/IChart.h"
#include "Project/fake_map.h"
void MainWindow::serial_init(){
    fake_map<QString, QSerialPort::BaudRate> m_baudrate;
    m_baudrate.insert("1200",QSerialPort::Baud1200);
    m_baudrate.insert("2400",QSerialPort::Baud2400);
    m_baudrate.insert("4800",QSerialPort::Baud4800);
    m_baudrate.insert("9600",QSerialPort::Baud9600);
    m_baudrate.insert("19200",QSerialPort::Baud19200);
    m_baudrate.insert("38400",QSerialPort::Baud38400);
    m_baudrate.insert("57600",QSerialPort::Baud57600);
    m_baudrate.insert("115200",QSerialPort::Baud115200);
    fake_map<QString, QSerialPort::DataBits> m_databits;
    m_databits.insert("5",QSerialPort::Data5);
    m_databits.insert("6",QSerialPort::Data6);
    m_databits.insert("7",QSerialPort::Data7);
    m_databits.insert("8",QSerialPort::Data8);
    fake_map<QString, QSerialPort::StopBits> m_stopbits;
    m_stopbits.insert("1",QSerialPort::OneStop);
    m_stopbits.insert("2",QSerialPort::TwoStop);
    m_stopbits.insert("1.5",QSerialPort::OneAndHalfStop);
    fake_map<QString, QSerialPort::Parity> m_parity;
    m_parity.insert("无",QSerialPort::Parity::NoParity);
    m_parity.insert("奇",QSerialPort::Parity::OddParity);
    m_parity.insert("偶",QSerialPort::Parity::EvenParity);
    auto itor = m_baudrate.begin();
    while(itor!=m_baudrate.end()){
        ui->bool_baudrate_set->addItem(itor->first, itor->second);
        ui->dev_baudrate_set->addItem(itor->first, itor->second);
        ++itor;
    }
    ui->bool_baudrate_set->setCurrentIndex(3);
    ui->dev_baudrate_set->setCurrentIndex(3);
    auto itor2 = m_databits.begin();
    while(itor2!=m_databits.end()){
        ui->bool_databits_set->addItem(itor2->first, itor2->second);
        ui->dev_databits_set->addItem(itor2->first, itor2->second);
        ++itor2;
    }
    ui->bool_databits_set->setCurrentIndex(3);
    ui->dev_databits_set->setCurrentIndex(3);
    auto itor3 = m_stopbits.begin();
    while(itor3!=m_stopbits.end()){
        ui->bool_stopbits_set->addItem(itor3->first, itor3->second);
        ui->dev_stopbits_set->addItem(itor3->first, itor3->second);
        ++itor3;
    }
    ui->bool_stopbits_set->setCurrentIndex(0);
    ui->dev_stopbits_set->setCurrentIndex(0);
    auto itor4 = m_parity.begin();
    while(itor4!=m_parity.end()){
        ui->bool_parity_set->addItem(itor4->first, itor4->second);
        ui->dev_parity_set->addItem(itor4->first, itor4->second);
        ++itor4;
    }
    ui->bool_parity_set->setCurrentIndex(2);
    ui->dev_parity_set->setCurrentIndex(2);
    //更新串口资源
    serial_bool_update();
    serial_dev_update();
}

void MainWindow::buf_show_slot(QByteArray bytes){
    bytes = bytes.replace("\r","").replace("\n","");//去掉换行
    ui->data_serialbuf->appendPlainText(bytes);
}

//读取真实数据的槽，需要处理换行问题
void MainWindow::true_data_show_slot(QByteArray data){
    data = data.replace("\r","").replace("\n","").replace(" ","");//去掉换行和空格
    //不需要空数据
    if(data.isEmpty())
        return;
    ui->data_switch->appendPlainText(data);
    QString data_str = data;
    //取数
    bool success = false;
    double data_double = data_str.toDouble(&success);
    if(!success){//转换失败
        IChart::get_instance()->chart_add_point(-1);//负数表示数据转换失败，一般是没采到数据，例如大激光返回"E18"
        return;
    }
    IChart::get_instance()->chart_add_point(data_double);
    ui->num_data_switch->setValue(ui->num_data_switch->value()+1);//有效数据加1


}
