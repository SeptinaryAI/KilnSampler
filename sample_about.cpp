#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Timer/bool_timer.h"
#include "Timer/dev_timer.h"
#include "Timer/count_timer.h"
#include "Serial/serial_bool.h"
#include "Device/IDevice.h"
#include "Thread/sample_task.h"
#include <QMessageBox>
#include <QTime>
#include <QThreadPool>
#include "Chart/IChart.h"

//采样状态枚举:采样中 、 结束
enum sample_status{
    RUN,
    END,
};

//霍尔开关触发会引起的布尔信号灯状态变化
static bool bool_light_flag = false;
void bool_light_change(Ui::MainWindow* ui){
    bool_light_flag = !bool_light_flag;
    if(bool_light_flag){
        ui->bool_flag->setStyleSheet("QLabel{background:#282eea;}");
    }else{
        ui->bool_flag->setStyleSheet("QLabel{background:#ea6728;}");
    }
}

//采样状态变换会引起的界面空间状态变换
void ui_switch(Ui::MainWindow* ui, sample_status status){
    if(status == sample_status::RUN){
        ui->btn_start->setEnabled(false);
        ui->btn_end->setEnabled(true);
        ui->tab_dev->setEnabled(false);
        ui->data_serialbuf->clear();
        ui->data_switch->clear();
        ui->num_data_switch->setValue(0);//从0开始计数
        ui->data_serialbuf->setEnabled(false);
        ui->data_switch->setEnabled(false);
        ui->data_serialbuf->setStyleSheet("QPlainTextEdit{color:black;}");
        ui->data_switch->setStyleSheet("QPlainTextEdit{background:#d2daf7;color:black;}");
        ui->bool_used->setEnabled(false);
        ui->check_timer->setEnabled(false);
        ui->check_autosave->setEnabled(false);
        ui->sample_time->setEnabled(false);
        ui->bool_start->setEnabled(false);
        ui->bool_end->setEnabled(false);
        ui->text_company->setEnabled(false);
        ui->text_kilnnum->setEnabled(false);
        ui->times->setEnabled(false);
        ui->btn_choosepath->setEnabled(false);
        ui->btn_savedata->setEnabled(false);
        ui->dev_onoff->setEnabled(false);
        ui->bool_onoff->setEnabled(false);
        ui->btn_update_chart->setEnabled(false);
        ui->menuBar->setEnabled(false);
    } else{
        ui->btn_start->setEnabled(true);
        ui->btn_end->setEnabled(false);
        ui->tab_dev->setEnabled(true);
        ui->data_serialbuf->setEnabled(true);
        ui->data_switch->setEnabled(true);
        ui->data_serialbuf->setStyleSheet("QPlainTextEdit{color:black;}");
        ui->data_switch->setStyleSheet("QPlainTextEdit{background:white;color:black;}");
        ui->bool_used->setEnabled(true);
        ui->check_timer->setEnabled(true);
        ui->check_autosave->setEnabled(true);
        ui->sample_time->setEnabled(true);
        ui->bool_start->setEnabled(true);
        ui->bool_end->setEnabled(true);
        ui->text_company->setEnabled(true);
        ui->text_kilnnum->setEnabled(true);
        ui->times->setEnabled(true);
        ui->btn_choosepath->setEnabled(true);
        ui->btn_savedata->setEnabled(true);
        ui->dev_onoff->setEnabled(true);
        ui->bool_onoff->setEnabled(true);
        ui->btn_update_chart->setEnabled(true);
        ui->menuBar->setEnabled(true);
    }
}

//采样状态转换函数
void status_switch(Ui::MainWindow* ui, sample_status status){
    if(status == sample_status::RUN){
        if(!serial_dev::get_serial()->isOpen()){
            QMessageBox::warning(nullptr,"Warning","设备串口未打开！",QMessageBox::Ok);
            return;
        }
        if(ui->bool_used->isChecked()){
            if(!serial_bool::get_serial()->isOpen()){
                QMessageBox::warning(nullptr,"Warning","霍尔开关串口未打开！",QMessageBox::Ok);
                return;
            }
            ui_switch(ui,sample_status::RUN);
            serial_bool::get_serial()->readAll();//清空接受缓冲区
            //交给布尔控制
            bool_timer::get_timer()->start(10);
            return;
        }
        //开始信号
        ui_switch(ui,sample_status::RUN);
        IChart::get_instance()->chart_show();//图表显示
        IDevice::get_instance()->start();
        int period = IDevice::get_instance()->def_period();//读取串口设备的默认周期，设置周期
        dev_timer::get_timer()->start(period);//开始采样，设置周期
        count_timer::set_start_time(QDateTime::currentMSecsSinceEpoch());//记录开始时间
        count_timer::get_timer()->start(period);//设置倒计时时间更新周期
        ui->statusBar->showMessage("采样开始! 周期: "+QString::number(period)+" ms");
    } else {
        ui_switch(ui,sample_status::END);
        bool_timer::get_timer()->stop();
        //结束信号
        IDevice::get_instance()->end();
        dev_timer::get_timer()->stop();//结束采样时钟
        count_timer::get_timer()->stop();//结束倒计时时钟
        ui->bool_count->setValue(0);
        ui->statusBar->showMessage("采样结束!");
    }
}

//SLOT，采样开始
void MainWindow::sample_start(){
    status_switch(ui, sample_status::RUN);
}

//SLOT，采样结束
void MainWindow::sample_end(){
    status_switch(ui, sample_status::END);
    //自动保存
    if(ui->check_autosave->isChecked()){
        save_data_to_file();
    }
}

//霍尔开关Timer的周期事件，会检查霍尔触发情况
void MainWindow::bool_timer_tick(){
    //霍尔开关检测函数
    QSerialPort* ser = serial_bool::get_serial();
    if(ser->isOpen()){
        QByteArray buf = ser->readAll();
        if(!buf.isEmpty()){
            //霍尔触发
            bool_light_change(ui);
            int round_num = ui->bool_count->value();
            ui->bool_count->setValue(round_num+1);
            if(ui->bool_count->value() == ui->bool_start->value()){
                //开始信号
                IChart::get_instance()->chart_show();//图表显示
                IDevice::get_instance()->start();
                int period = IDevice::get_instance()->def_period();//读取串口设备的默认周期，设置周期
                dev_timer::get_timer()->start(period);//开始采样，设置周期
                count_timer::set_start_time(QDateTime::currentMSecsSinceEpoch());//记录开始时间
                count_timer::get_timer()->start(period);//设置倒计时时间更新周期
                ui->statusBar->showMessage("采样开始! 周期: "+QString::number(period)+" ms");
            }
            if(ui->bool_count->value() == ui->bool_end->value()){
                //结束信号
                sample_end();
            }
        }
    }
}
//串口设备Timer的周期事件，按照设置的PERIOD值周期触发，每次会申请一次采样进程
void MainWindow::dev_timer_tick(){
    //IDevice::get_instance()->sample();
    sample_task* task = new sample_task();
    QThreadPool::globalInstance()->start(task);
}

//计时器Timer的周期事件，更新采样计时，以及判断是否到点 采样结束
void MainWindow::count_timer_tick(){
    qint64 msec = QDateTime::currentMSecsSinceEpoch() - count_timer::get_start_time();//过去了多久
    ui->timer_count->setText(QString::number(msec/1000.0,'f',1));
    //如果使用了倒计时，就产生结束信号
    if(ui->check_timer->isChecked()){
        //时间到
        if(msec >= ui->sample_time->value() * 1000)
            sample_end();
    }
}
