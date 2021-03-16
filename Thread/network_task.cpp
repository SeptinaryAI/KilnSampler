#include "Thread/network_task.h"
#include "Network/network.h"
#include <QMutex>
#include <QDebug>
#include <QThread>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Cryption/rsa/rsa.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QUuid>

extern QMutex network_tick_mutex;
extern QDateTime last_auth_time;//上次认证时间

/* 用于对不同的特殊情况做冷却，通过给tick_count赋值来控制下一个run()到来的时间 */
int tick_count = 1;//run()触发计数,倒着计数

/**
 * @brief network_task::run 网络上传的多线程函数，本质是一个有限状态机
 */
void network_task::run(){
#ifdef DEBUG
    qDebug() << "network task count: " << tick_count;
#endif
    if(--tick_count > 0){
        /* tick数未到达，不执行 */
        network_tick_mutex.unlock();//放掉锁
        return;
    }
    /* tick数到达，执行状态机 */
    tick_count = 1;
    //debug start
#ifdef DEBUG
    //QThread::sleep(2);//这两秒给debug调整状态用
    qDebug() << "status:" << network::status;
#endif
    /* 身份过期本地判断 */
    if(QDateTime::currentMSecsSinceEpoch() - last_auth_time.toMSecsSinceEpoch() > AUTH_AVA_TIME_SSEC){
        network::status = file_status::NEEDLOGIN;
    }

    /* 状态判断 & 操作 */
    if(network::status == file_status::NEEDLOGIN){
        /* 需要重新（或者首次）身份认证 */
        network::get_instance()->postAuth();
        return;
    }
    if(network::status == file_status::WAIT){
        /* 身份认证通过，开始查询数据库是否有重复数据 */
        network::get_instance()->postDBCheck();
        return;
    }
    if(network::status == file_status::INDB){
        /* 在数据库找到重复数据，需要用户判断 （上传文件 or 放弃该文件上传？）*/
        /* network_tick_mutex 在用户完成判断前保持锁住 */
        emit network::get_instance()->db_overwrite_user_do(network::last_filename);//发出让用户确认的信号
        return;
    }
    if(network::status == file_status::NINDB){
        /* 未在数据库找到重复数据，可以开始POST文件 */
        network::get_instance()->postFile();
        return;
    }

}
