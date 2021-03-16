#ifndef NETWORK_TASK_H
#define NETWORK_TASK_H

#include <QRunnable>
#include <QObject>

#define UPLOAD_CHECK_PERIOD 200 //多少毫秒检测一次文件待上传队列
/* 一些特殊情况下重新发起请求的冷却时间 */
#define LOGIN_WAIT_SEC_ 5000                                                 //登录的失败情况需要等待多久才会再次发起run()
#define LOGIN_WAIT_TICK (LOGIN_WAIT_SEC_ / UPLOAD_CHECK_PERIOD)             //转为超时需要等待的tick数
#define NORMAL_WAIT_SEC_ 1500                                                //各种常规的失败情况需要等待多久才会再次发起run()
#define NORMAL_WAIT_TICK (NORMAL_WAIT_SEC_ / UPLOAD_CHECK_PERIOD)            //转为超时需要等待的tick数
#define TRYAGAIN_WAIT_SEC_ 2000                                              //重试需要等待多久才会再次发起run()
#define TRYAGAIN_WAIT_TICK (TRYAGAIN_WAIT_SEC_ / UPLOAD_CHECK_PERIOD)        //转为超时需要等待的tick数
#define NETERR_WAIT_SEC_ 10000                                               //网络失败需要等待多久才会再次发起run()
#define NETERR_WAIT_TICK (NETERR_WAIT_SEC_ / UPLOAD_CHECK_PERIOD)            //转为超时需要等待的tick数
#define POSTTIMEOUT_WAIT_SEC_ 3000                                           //POST请求接收超时需要等待多久才会再次发起run()
#define POSTTIMEOUT_WAIT_TICK (POSTTIMEOUT_WAIT_SEC_ / UPLOAD_CHECK_PERIOD)  //转为超时需要等待的tick数

class network_task : public QRunnable{
public:
    network_task(){}
    void run() override;
};


#endif
