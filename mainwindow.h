#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Device/IDevice.h"
#include "Logs/logs.h"
#define NODEBUG

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void init();
    void serial_init();
    void timer_init();
    void device_fun_init();
    void project_init();
    void device_init();
    void profile_init();
    void file_init();
    void network_init();

private slots:
    void serial_bool_onoff();//霍尔串口连接/断开
    void serial_dev_onoff();//各仪器串口连接/断开
    void serial_bool_update();//可选串口更新
    void serial_dev_update();
    void project_choose();//测量业务选择
    void device_choose();//设备选择
    void sample_start();//采样开始
    void sample_end();//采样结束
    void buf_show_slot(QByteArray bytes);//实时串口数据显示
    void true_data_show_slot(QByteArray data);//实际串口数据转换得到的真实结果显示
    void status_show_slot(QString msg);//状态栏显示
    void bool_timer_tick();//霍尔开关定时器执行函数
    void dev_timer_tick();//设备采样定时器执行函数
    void count_timer_tick();//倒计时结束定时器执行函数
    void network_timer_tick();//文件网络传输定时器执行函数
    void update_to_chart();//更新数据到折线图表
    void destroy_chart();//销毁折线图窗口
    void update_data_count();//重新计算Text中的数据，得到有效数据的数量，更新到控件中显示
    //device_fun
    void device_fun_laser_on();//大激光:打开激光
    void device_fun_laser_off();//大激光:关闭激光
    //file about
    void choose_dir();//选择总文件夹
    void save_data_to_file();//保存本次测量
    void open_save_ptah();//打开保存目录
    //window
    void open_data();//打开数据文件
    //debug
    void debug_fun();
    void debug_tick();
    //network progress bar
    void upload_on_off();//开始/停止上传文件
    void choose_file_upload();//手动选择文件上传
    void auto_upload_check();//自动上传总目录按钮勾选
    void upload_success_slot(QString file);//上传文件成功，表单显示
    void upload_format_err_slot(QString file);//文件数据格式有误
    void upload_skip_slot(QString file);//文件其他原因跳过
    void upload_reply_msg_slot(QString);//上传成功
    void db_overwrite_user_do_slot(QString filename);//让用户决定是否覆盖重复数据
};

/* 工具 */
QByteArray make_single_json_str(QMap<QByteArray,QByteArray> args);//构建简单的 key-value json字节数组（字符串）
QByteArray make_single_json_str(QMap<QString,QString> args);//构建简单的 key-value json字节数组（字符串）
QJsonObject str_to_json(QByteArray jsonString);//字节数组（字符串）转QJsonObject
QByteArray json_to_str(QJsonObject jsonObj);//QJsonObject 转 字节数组（字符串）
#endif // MAINWINDOW_H
