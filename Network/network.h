#ifndef NETWORK_H
#define NETWORK_H
#include <QObject>
#include <QString>
#include <QMap>
#include <QNetworkReply>
#include <QFile>
#include <QStack>

//身份认证，服务器记录时间会更长
#define AUTH_AVA_TIME_SSEC (3600000000 * 6)    //身份认证的本地记录的有效时间 6小时
//文件上传状态机
enum file_status{
    NEEDLOGIN,      /* 需要重新认证身份（登录） */
    WAIT,           /* 待上传，下一步需要向服务器请求判断数据库中是否重复 */
    INDB,           /* 数据已存在数据库，下一步会让用户判断是否坚持上传当前数据覆盖重复数据 */
    NINDB,          /* 数据不在数据库中，下一步会直接上传 */
};

class network : public QObject{
    Q_OBJECT
private:
    static network* network_ins;
    void postFiles(QVector<QString> &files);//文件上传至服务器

    //消息队列
    static QQueue<QString> files_queue;//待上传文件队列

public:
    //构造析构
    explicit network();
    virtual ~network();

    static network* get_instance(){return network_ins;}
    static QString file_url;//文件上传接口
    static QString auth_url;//身份认证接口
    static QString db_url;//数据库查询是否重复接口
    static QString uuid;//记录当前软件的身份
    static file_status status;//当前文件的状态,可以判断下次是否需要发起身份认证 等
    void postFile();//上传文件
    void postAuth();//上传身份认证
    void postDBCheck();//数据重复查询

    static QString last_filename;//最新提交的文件名
    //QQueue文件上传队列 线程安全 的操作方法
    static void queue_push(QString file);//向消息队列尾部添加待上传文件
    static QString queue_topAndPop();//消息队列取头部,并删除头部
    static QString queue_top();//消息队列取头部
    static bool queue_cmpTopAndPop(QString top);//判断队列头部，并且在成功时删除头部
    static void queue_clear();//消息队列清空
    static void queue_reset(QQueue<QString> new_queue);//重新填充消息队列
    static bool is_queue_empty();//消息队列是否为空
public slots:
    void file_nam_finished(QNetworkReply *reply);//文件POST进程结束SLOT
    void auth_nam_finished(QNetworkReply *reply);//身份认证参数POST进程结束SLOT
    void db_nam_finished(QNetworkReply *reply);//数据库查询重复POST进程结束SLOT
    void reply_operation(QNetworkReply *reply);//reply响应操作的总函数

signals:
    void upload_success(QString);//文件上传成功
    void upload_format_err(QString);//文件数据格式有问题
    void upload_skip(QString);//文件其他原因跳过
    void upload_reply_msg(QString);//用于触发主窗体显示上传状态信息
    void db_overwrite_user_do(QString filename);//让用户确认是否覆盖重复数据的信号
};

#endif













