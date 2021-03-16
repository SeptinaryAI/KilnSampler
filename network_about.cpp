#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Network/network.h"
#include "Profile/profile.h"
#include "Timer/network_timer.h"
#include "Thread/network_task.h"
#include <QThreadPool>
#include <QFileDialog>
#include <QMessageBox>
#include <QMutex>
#include <QStandardItemModel>

#define UPLOAD_SHOW_NUM 40      //队列显示多少记录项

//定时器Timer的周期事件，检测文件待上传队列
QMutex network_tick_mutex;//全局锁，会被network.cpp extern引用

QStandardItemModel* upload_list_data = new QStandardItemModel();
//递归遍历目录下的所有文件,返回符合 *-*-*-*.txt 格式的采样数据
void get_all_file(QString dir_path,QStringList &ret){
    QDir dir(dir_path);
    if(!dir.exists())
    {
        return;
    }
    //注意：一定要跳过 ".." 和 "."目录，否则会无限循环
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst);//文件夹排在前面
    QFileInfoList infos = dir.entryInfoList();
    for (auto info : infos) {
        QString dir = info.path();//目录
        QString filename = info.fileName();//文件全名
        if(info.isDir()){
            /* 文件夹 */
            get_all_file(dir+'/'+filename,ret);
            continue;
        }
        /* 文件 */
        QString suffix = info.suffix();//后缀名
        /* 不为txt后缀则不上传 */
        if(suffix != "txt")
            continue;
        /* 文件必须有至少三个"-"*/
        int count = 0;
        for(QChar c : filename)
            if(c == '-') ++count;
        if(count < 3)
            continue;
        /* 符合要求，加入到ret中 */
        ret.append(dir + '/' + filename);
    }
}

//网络初始化
void MainWindow::network_init(){
    network::file_url = profile::read_config("FILE_POST_URL",network::file_url);//尝试获取软件配置的FILE_POST_URL参数
    network::auth_url = profile::read_config("AUTH_POST_URL",network::auth_url);//尝试获取软件配置的AUTH_POST_URL参数
    network::db_url = profile::read_config("DB_POST_URL",network::db_url);//尝试获取软件配置的DB_POST_URL参数
    logs::write_log("file_url 被配置为 " + network::file_url);
    logs::write_log("auth_url 被配置为 " + network::auth_url);
    logs::write_log("db_url 被配置为 " + network::db_url);
    connect(network::get_instance(), SIGNAL(upload_success(QString)), this, SLOT(upload_success_slot(QString)));
    connect(network::get_instance(), SIGNAL(upload_format_err(QString)), this, SLOT(upload_format_err_slot(QString)));
    connect(network::get_instance(), SIGNAL(upload_skip(QString)), this, SLOT(upload_skip_slot(QString)));
    connect(network::get_instance(), SIGNAL(upload_reply_msg(QString)), this, SLOT(upload_reply_msg_slot(QString)));
    connect(network::get_instance(), SIGNAL(db_overwrite_user_do(QString)), this, SLOT(db_overwrite_user_do_slot(QString)));
    connect(network_timer::get_timer(), SIGNAL(timeout()), this, SLOT(network_timer_tick()));//周期发起网络请求

    network_timer::get_timer()->start(UPLOAD_CHECK_PERIOD);//周期检测文件待上传队列
    ui->list_upload->setModel(upload_list_data);
}

void MainWindow::network_timer_tick(){
    if(!network_tick_mutex.tryLock(0))
        return;
    /* 临界区 start */
    bool post = false;//成功触发post标记
    if(!network::is_queue_empty()){
        QString file = network::queue_top();
        if(file.isNull()){
            /* 这里正好有其他线程修改queue，导致了queue在if结构中变为了空 */
            /* 由于线程安全函数的判断,这里top()不会报错，会返回null */
            /* 临界区 end 自行退出 */
            network_tick_mutex.unlock();
            return;
        }
        if(!QFile::exists(file)){
            /* 因为某种原因，原本存在的文件不存在了，将文件移出队列，直接解锁，return，文件将从队列中消失*/
            network::queue_cmpTopAndPop(file);
            network_tick_mutex.unlock();
            return;
        }
        network::last_filename = file;//记录到全局
        //异步执行network::postFile(file)（多线程
        network_task* task = new network_task();
        QThreadPool::globalInstance()->start(task);
        post = true;
    }
    if(!post){
        /* 没有成功触发POST，开始下个周期的判断 */
        /* 临界区 end 自行退出 */
        network_tick_mutex.unlock();
    }
    /* 成功触发POST，临界区将一直存在，直到reply返回 */
}

//手动选择文件上传
void MainWindow::choose_file_upload(){
    QStringList filenames = QFileDialog::getOpenFileNames();
    for(auto file : filenames){
        //先判断文件名是否为测窑数据文件
        QFileInfo fileInfo(file);
        QString dir = fileInfo.path();//目录
        QString filename = fileInfo.fileName();//文件全名
        QString suffix = fileInfo.suffix();//后缀名
        /* 不为txt后缀则不上传 */
        if(suffix != "txt")
            continue;
        /* 文件必须有至少三个"-"*/
        int count = 0;
        for(QChar c : filename)
            if(c == '-') ++count;
        if(count < 3)
            continue;
        //添加到待上传文件队列头部
        network::queue_push(file);
    }
}

//自动上传总目录按钮勾选
void MainWindow::auto_upload_check(){
    //保证被勾选
    if(!ui->check_autoupload->isChecked())
        return;
    //首先将所有的总目录可用数据文件找到，push到上传队列
    QString choose_dir = ui->text_path->toPlainText();
    if(choose_dir.isEmpty())
        choose_dir = "./data";//默认当前目录
    QStringList files;
    get_all_file(choose_dir,files);
    for(auto f : files){
        QFileInfo fileInfo(f);
        QString dir = fileInfo.path();//目录
        QString filename = fileInfo.fileName();//文件全名
        QString flag_file = dir + "/uploaded[" + filename + "]";
        //判断是否存在uploaded标识文件,存在则跳过该文件
        if(QFile::exists(flag_file))
            continue;
        network::queue_push(f);
    }
    /* 会在在file_ablout中判断：Check状态将成为标识，每次保存数据文件时，将会自动把数据文件 push到上传队列*/
}
//开始/停止上传文件
static bool upload_onoff = true;
void MainWindow::upload_on_off(){
    upload_onoff = !upload_onoff;
    if(upload_onoff){
        logs::write_log("文件上传被继续！");
        ui->btn_upload_stop->setText("停用上传");
        network_timer::get_timer()->start(UPLOAD_CHECK_PERIOD);//开启网络上传功能的时钟
        upload_reply_msg_slot("上传继续！");
    } else {
        logs::write_log("文件上传被停止！");
        ui->btn_upload_stop->setText("启用上传");
        network_timer::get_timer()->stop();//停止网络上传功能的时钟
        upload_reply_msg_slot("上传停止！");
    }
}

//上传成功，界面显示
void MainWindow::upload_success_slot(QString file){
    logs::write_log(log_type::WARNING, "文件<" + file + ">上传成功！");
    QFileInfo fileInfo(file);
    QString filename = fileInfo.fileName();//文件全名
    QStandardItem *item = new QStandardItem("[" + QTime::currentTime().toString("HH:mm:ss") + " 0] "+filename+"  上传成功！ ");
    QList<QStandardItem*> add;
    add.append(item);
    upload_list_data->insertRow(0,add);
    ui->label_network_status->setText("");//网络异常状态清空
    //保持 UPLOAD_SHOW_NUM 条内容显示
    while(upload_list_data->rowCount() > UPLOAD_SHOW_NUM){
        upload_list_data->removeRows(UPLOAD_SHOW_NUM,1);
    }
}
//文件上传跳过： 格式原因
void MainWindow::upload_format_err_slot(QString file){
    logs::write_log(log_type::WARNING, "文件<" + file + ">内容格式有误，被跳过！");
    QFileInfo fileInfo(file);
    QString filename = fileInfo.fileName();//文件全名
    QStandardItem *item = new QStandardItem("[" + QTime::currentTime().toString("HH:mm:ss") + " 1] "+filename+" 数据格式错误！ ");
    QList<QStandardItem*> add;
    add.append(item);
    upload_list_data->insertRow(0,add);
    ui->label_network_status->setText("");//网络异常状态清空
    //保持 UPLOAD_SHOW_NUM 条内容显示
    while(upload_list_data->rowCount() > UPLOAD_SHOW_NUM){
        upload_list_data->removeRows(UPLOAD_SHOW_NUM,1);
    }
}
//文件上传跳过： 其他原因
void MainWindow::upload_skip_slot(QString file){
    logs::write_log(log_type::WARNING, "文件<" + file + ">内容有误，被跳过！");
    QFileInfo fileInfo(file);
    QString filename = fileInfo.fileName();//文件全名
    QStandardItem *item = new QStandardItem("[" + QTime::currentTime().toString("HH:mm:ss") + " 2] "+filename+" 被跳过！ ");
    QList<QStandardItem*> add;
    add.append(item);
    upload_list_data->insertRow(0,add);
    ui->label_network_status->setText("");//网络异常状态清空
    //保持 UPLOAD_SHOW_NUM 条内容显示
    while(upload_list_data->rowCount() > UPLOAD_SHOW_NUM){
        upload_list_data->removeRows(UPLOAD_SHOW_NUM,1);
    }
}

//上传状态显示
void MainWindow::upload_reply_msg_slot(QString msg){
    logs::write_log("文件队列消息："+msg);
    ui->label_network_status->setText(msg);
}

//让用户决定是否覆盖重复数据
void MainWindow::db_overwrite_user_do_slot(QString filename){
    logs::write_log(log_type::WARNING, "文件<" + filename + ">在云端数据库中被检测到重复！");
    QMessageBox::StandardButton btn =
        QMessageBox::warning(nullptr,"Warning","在云端数据库中找到与"+ filename +
                             "拥有相同[公司名称][窑号][测量对象][测量次数]的数据，是否覆盖?\r\n"
                             "此操作对数据库的更改不可逆！！！\r\n"
                             "如果你选择了‘否’,将会取消该文件的上传",
                             QMessageBox::Yes|QMessageBox::No);
    if(btn == QMessageBox::Yes){
        /* 确认覆盖 */
        logs::write_log(log_type::WARNING, "文件<" + filename + ">确认覆盖到数据库！");
        network::get_instance()->status = file_status::NINDB;//把文件状态标记为"不在数据库中"，这样就会认为该文件可以上传，默认覆盖
    }
    else{
        /* 不覆盖，放弃该文件的上传 */
        network::queue_cmpTopAndPop(filename);//推出该文件队列记录项
        emit network::get_instance()->upload_skip(filename);
        network::get_instance()->status = file_status::WAIT;//为下一个文件上传做准备，初始化状态机
        ui->label_network_status->setText("终止了一个在数据库中重复数据项的上传！");//网络异常状态
    }
    network_tick_mutex.unlock();//用户判断完后解锁
}
