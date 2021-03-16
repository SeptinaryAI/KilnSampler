#include "mainwindow.h"
#include "ui_mainwindow.h"


//配置功能按钮对应到功能函数
void MainWindow::device_fun_init(){
    connect(ui->btn_laser_open,SIGNAL(clicked()),this,SLOT(device_fun_laser_on()));
    connect(ui->btn_laser_close,SIGNAL(clicked()),this,SLOT(device_fun_laser_off()));
}

//SLOT 额外功能按钮的SLOT 列表
//大激光：打开激光
void MainWindow::device_fun_laser_on(){
    IDevice::get_instance()->extern_fun("ON");
}
//大激光：关闭激光
void MainWindow::device_fun_laser_off(){
    IDevice::get_instance()->extern_fun("OFF");
}
