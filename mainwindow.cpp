#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Timer/bool_timer.h"
#include "Timer/dev_timer.h"
#include "Timer/count_timer.h"
#include "Serial/serial_bool.h"
#include <QStyleFactory>
#include <QMap>
#include <QtSerialPort/QSerialPort>
#include <QtXml/QDomDocument>
#include <QTextStream>
#include <QMessageBox>
#include "Profile/profile.h"
#include "Project/IProject.h"
#include <QThread>
#include "Chart/IChart.h"
#include "Network/network.h"
#include "Cryption/rsa/rsa.h"
#include "Cryption/aes/aes.h"
#include "Project/fake_map.h"
#include "Logs/logs.h"
#include <QByteArray>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QStringList keyList = QStyleFactory::keys();
    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setPalette(qApp->style()->standardPalette());     // 使用风格默认的颜色
    //初始化
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//主窗体总初始化
void MainWindow::init(){
    profile_init();
    serial_init();
    timer_init();
    device_init();
    project_init();
    device_fun_init();
    file_init();
    network_init();
    //串口设备抽象接口的 观察者 相关信号槽绑定
    connect(IDevice::get_observer(), SIGNAL(status_show(QString)), this, SLOT(status_show_slot(QString)));
    connect(IDevice::get_observer(), SIGNAL(buf_show(QByteArray)), this, SLOT(buf_show_slot(QByteArray)));
    connect(IDevice::get_observer(), SIGNAL(true_data_show(QByteArray)), this, SLOT(true_data_show_slot(QByteArray)));
}

static QTimer debug_timer;

//timer相关初始化
void MainWindow::timer_init(){
    bool_timer::get_timer()->stop();
    connect(bool_timer::get_timer(), SIGNAL(timeout()), this, SLOT(bool_timer_tick()));
    dev_timer::get_timer()->stop();
    connect(dev_timer::get_timer(), SIGNAL(timeout()), this, SLOT(dev_timer_tick()));
    count_timer::get_timer()->stop();
    connect(count_timer::get_timer(), SIGNAL(timeout()), this, SLOT(count_timer_tick()));
    connect(&debug_timer, SIGNAL(timeout()), this, SLOT(debug_tick()));
}

//bottom状态栏信息更新，相当于 信息提示
void MainWindow::status_show_slot(QString msg){
    ui->statusBar->showMessage(msg);
}

//重新计算串口读取数据的Text中的文本，得到有效数据的数量，更新到界面显示
void MainWindow::update_data_count(){
    QTextDocument *document;
    document=ui->data_switch->document();
    int count = 0;
    for(auto itor=document->begin();itor!=document->end();itor=itor.next()){
        bool success = false;//转double是否成功
        itor.text().toDouble(&success);
        if(success)
            ++count;
    }
    ui->num_data_switch->setValue(count);
}

//打开数据文件
void MainWindow::open_data(){
    QString filename = QFileDialog::getOpenFileName();
    if(!QFile::exists(filename)){
        QMessageBox::warning(nullptr,"Error","要打开的文件不存在！",QMessageBox::Ok);
        return;
    }
    QFile file(filename);
    file.open(QFile::ReadOnly|QIODevice::Text);
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    ui->data_switch->clear();
    ui->data_switch->appendPlainText(stream.readAll());
    ui->btn_savedata->setEnabled(false);//因为打开的文件一般不属于当次测量的文件，避免错误覆盖，将保存按钮设置为不可用，直到下次采集数据结束
    file.close();
    update_data_count();//更新有效数字显示
    update_to_chart();//更新到折现图显示
}

/* 因为这个项目比较简单，工具/工具类就都写在这了 */
QByteArray make_single_json_str(QMap<QByteArray,QByteArray> args){
    QJsonObject json;
    auto itor = args.begin();
    while(itor != args.end()){
        json.insert(itor.key(),QString::fromUtf8(itor.value()));
        ++itor;
    }
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    return byte_array;
}
QByteArray make_single_json_str(QMap<QString,QString> args){
    QJsonObject json;
    auto itor = args.begin();
    while(itor != args.end()){
        json.insert(itor.key(),itor.value());
        ++itor;
    }
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    return byte_array;
}

//字节数组（也可作字符串）转QJsonObject
QJsonObject str_to_json(QByteArray bytes)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(bytes);
    QJsonObject json_obj = json_doc.object();
    return json_obj;
}
//QJsonObject 转 字节数组（也可作字符串）
QByteArray json_to_str(QJsonObject jsonObject)
{
    QByteArray json_bytes = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
    return json_bytes;
}

void MainWindow::debug_tick(){
    static int cnt = 0;
}

extern QDateTime last_auth_time;//上次认证时间

int roll = 0;
void MainWindow::debug_fun(){
#ifdef DEBUG
    switch (roll++) {
        case 0 :
              network::status = file_status::WAIT;
              qDebug()<<"adj status to WAIT";
              break;
        case 1 :
              network::status = file_status::NINDB;
              qDebug()<<"adj status to NINDB";
              break;
        case 2 :
              network::status = file_status::INDB;
              qDebug()<<"adj status to INDB";
              break;
        case 3 :
              network::status = file_status::NEEDLOGIN;
              qDebug()<<"adj status to NEEDLOGIN";
              break;
        default:break;
    }
    if(roll > 3) roll = 0;

//    rsa::test();
//    QByteArray data_add_uuid =  "{\"busD\":{\"authId\":\"1fc5a4b8652c0d3f\",\"key\":\"1234567890abcdef\",\"timeStamp\":\"1614084767069\",\"uuid\":\"{fakeuuid-123123-asadfqhjjkaudjhkj}\"}}";
//    QCryptographicHash *hash=new QCryptographicHash(QCryptographicHash::Sha256);
//    hash->addData(data_add_uuid);
//    QByteArray sha256=hash->result();//sha256的结果
//    delete hash;//释放内存
//    QByteArray get = sha256.toHex().toLower();
//    qDebug()<< get;

#endif

    qDebug()<<QString::fromUtf8( rsa::rsa_pub_decrypt_base64("ZBJbL4fC7Z79ZApPH/YmJwVCMOoktiunYLELbyaMLEb3cNvopT2+kARnLF0Ftuc4arlzSPEdAkUXaJUnnGLZIjZqnsUYX5PSxr9Fpesfnsc=\"",rsa::pkey));
}

