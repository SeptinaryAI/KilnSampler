#include "network.h"
#include "mainwindow.h"
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>
#include <QFile>
#include <QObject>
#include <QDebug>
#include <QMutex>
#include <QFileInfo>
#include <QMap>
#include <QQueue>
#include <QUuid>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include "Profile/profile.h"
#include "Cryption/rsa/rsa.h"
#include "Cryption/aes/aes.h"
#include "Thread/network_task.h"

#define REPEAT_MAX 1    //最多重试次数

const QString CONTENT_TYPE = "application/json";

/* 状态码 */
#define LOGIN_SUCCESS       "2000"//登录成功
#define LOGIN_FAIL          "2001"//登录失败
#define LOGIN_OUT           "2002"//登录过期
#define AUTH_FAIL           "2003"//校验失败，数据包有问题

#define CHECK_FAIL          "2010"//业务数据格式有问题
#define NOREPEAT            "2011"//数据库没有重复数据，可上传
#define REPEAT              "2012"//数据库有重复数据，如果上传将会覆盖

#define ADD_SUCCESS         "2020"//文件上传成功
#define ADD_FAIL            "2021"//文件格式错误

static int repeat_time = 0;//重试次数

//状态机初始化
file_status network::status = file_status::NEEDLOGIN;//下次需要发起认证

QString network::uuid = "{fakeuuid-123123-asadfqhjjkaudjhkj}";//记录当前软件的身份
//单例
network* network::network_ins = new network();
//默认
QString network::file_url = "http://localhost/upload/";     //文件上传接口 默认值，实际接口会读取config.txt配置文件 ，见函数network_init
QString network::auth_url = "http://localhost/auth/";       //身份认证接口
QString network::db_url = "http://localhost/db/";           //数据库查询是否重复接口

QQueue<QString> network::files_queue = QQueue<QString>();//待上传文件队列
QString network::last_filename = "";//最近一次上传文件名

extern QMutex network_tick_mutex;//这是网络上传周期检测文件函数的锁（来自network_about.cpp
extern int tick_count;//状态机执行计数（倒着计，来自network_task.cpp

QMutex reply_mutex;//用于保证一次POST过程中，超时 和 正常返回 的过程只能执行其中一个

//上次信息reply的时间戳，用于增加数据随机性，判断数据时间是否合理
qint64 last_time_stamp = 0;//当前信息的时间戳必须大于上次的时间戳，否则不合理，可能为人为伪造的数据
QDateTime last_auth_time = QDateTime::currentDateTime().addYears(-1);//上次认证时间，默认为1年前

network::network(){}
network::~network(){}

//输出队列的文件
void debug_files_queue(QQueue<QString> in){
    QString pr = "[queue] ";
    for(auto e : in)
        pr += e+" , ";
    logs::write_log_kn("当前待上传队列为: "+pr);
}

//文件上传队列操作锁
static QMutex queue_mutex;
//向消息队列添加待上传文件
void network::queue_push(QString file){
    queue_mutex.lock();//锁了就阻塞等待
    /* 临界区 start*/
    network_ins->files_queue.enqueue(file);
    debug_files_queue(network_ins->files_queue);
    /* 临界区 end*/
    queue_mutex.unlock();
}
//消息队列取头部,并删除头部
QString network::queue_topAndPop(){
    queue_mutex.lock();//锁了就阻塞等待
    /* 临界区 start*/
    if(network_ins->files_queue.isEmpty()){
        queue_mutex.unlock();
        return nullptr;//空指针表示为空，不可top
    }
    //临时拷贝top，避免unlock后线程竞争，导致top报错
    QString get_top = network_ins->files_queue.head();
    network_ins->files_queue.pop_front();
    debug_files_queue(network_ins->files_queue);
    /* 临界区 end*/
    queue_mutex.unlock();
    return get_top;
}
//消息队列取头部
QString network::queue_top(){
    queue_mutex.lock();//锁了就阻塞等待
    /* 临界区 start*/
    if(network_ins->files_queue.isEmpty()){
        queue_mutex.unlock();
        return nullptr;//空指针表示为空，不可top
    }
    //临时拷贝top，避免unlock后线程竞争，导致top报错
    QString get_top = network_ins->files_queue.head();
    /* 临界区 end*/
    queue_mutex.unlock();
    return get_top;
}
//判断队列头部，并且在成功时删除头部
bool network::queue_cmpTopAndPop(QString top){
    queue_mutex.lock();//锁了就阻塞等待
    bool ret = false;
    /* 临界区 start*/
    if(network_ins->files_queue.head() == top){
        network_ins->files_queue.pop_front();
        ret = true;
    }
    debug_files_queue(network_ins->files_queue);
    /* 临界区 end*/
    queue_mutex.unlock();
    return ret;
}
//消息队列清空
void network::queue_clear(){
    queue_mutex.lock();//锁了就阻塞等待
    /* 临界区 start*/
    network_ins->files_queue.clear();
    debug_files_queue(network_ins->files_queue);
    /* 临界区 end*/
    queue_mutex.unlock();
}
//重新填充消息队列
void network::queue_reset(QQueue<QString> new_queue){
    queue_mutex.lock();//锁了就阻塞等待
    /* 临界区 start*/
    network_ins->files_queue.clear();
    network_ins->files_queue = new_queue;//拷贝赋值，替换原来的队列
    debug_files_queue(network_ins->files_queue);
    /* 临界区 end*/
    queue_mutex.unlock();
}
//消息队列是否为空
bool network::is_queue_empty(){
    queue_mutex.lock();//锁了就阻塞等待
    /* 临界区 start*/
    bool ret = network_ins->files_queue.isEmpty();
    /* 临界区 end*/
    queue_mutex.unlock();
    return ret;
}
/**
 * @brief make_hex_16
 * @return 随机生成的16个十六进制字符
 */
static QByteArray make_hex_16(){
    qsrand((uint)(QTime(0,0,0).secsTo(QTime::currentTime())));
    char hexs[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    QByteArray key = "";
    for(int i = 0 ; i < 16 ; ++i){
        uint a = qrand()%15;   //随机生成0到f的随机数
        key += hexs[a];
    }
    return key;
}

/**
 * @brief sha256_hash
 * @param data
 * @return 对数据sha256 hash的结果
 */
static QByteArray sha256_hash(QByteArray& data){
    QCryptographicHash *hash=new QCryptographicHash(QCryptographicHash::Sha256);
    hash->addData(data);
    QByteArray sha256=hash->result();//sha256的结果
    delete hash;//释放内存
    QByteArray get = sha256.toHex().toLower();
    return sha256.toHex().toLower();
}

//身份认证参数POST
void network::postAuth(){
    //临界区
    //重新认证/或首次认证，需要刷新aes对称密钥 和 本机身份uuid

    /* debug */
#ifdef DEBUG
    aes::key = "1234567890abcdef";
#else
    aes::key = make_hex_16();//随机生成16字符aes密钥
    network::uuid = QUuid::createUuid().toString();//生成全球唯一码uuid，记录软件身份
#endif


    //构造Qt软件身份识别码、本机身份码和对称密钥
    /*
        {
            "authorization": "xxxx",//data明文sha256后,然后非对称加密生成摘要
            "data": "xxxx"//非对称加密后的业务数据数据
        }

    data中的明文如下，data存放明文非对称加密的内容
        {
            "busD":{
                "authId": "xxx",//身份验证码 字符串
                "uuid": "xxx",//唯一识别码 字符串
                "secretKey": "xxx",//密钥 字符串
                "timeStamp": "xxx"//时间戳，发送请求时的时间戳 长整型
            }
        }
     */
    QJsonObject busD;
    busD.insert("authId",CONST_AUTH);//Qt软件身份识别码，所有Qt采集软件的识别码相同
    busD.insert("uuid",network::uuid);//生成全球唯一码作为本机身份，用于区分多个Qt采集软件
    busD.insert("secretKey",QString(aes::key));//aes对称加密的密钥
    auto now_time = QDateTime::currentDateTime();
    busD.insert("timeStamp",tr("%1").arg(now_time.toMSecsSinceEpoch()));//时间戳
    QJsonObject data;
    data.insert("busD",busD);
    QByteArray msg_data = json_to_str(data);//构造Json字符串记录实际数据
    QByteArray msg_authorization = msg_data;//构造data内容的拼接字符串,用于校验
    logs::write_log_kn("登录请求 data明文" + msg_data);
    //POST参数  [data] ,身份认证的实际数据,返回的数据已经是base64了
    QByteArray post_data = rsa::rsa_pub_encrypt_base64(msg_data,rsa::pkey);//身份认证时用非对称加密
    //POST参数  [authorization] ,用于验证[data]的完整性，保证其未被hack
    QByteArray post_authorization = rsa::rsa_pub_encrypt_base64(sha256_hash(msg_authorization),rsa::pkey);

    //构造POST,Json字符串
    QMap<QByteArray, QByteArray> posts;
    posts.insert("data", post_data);
    posts.insert("authorization",post_authorization);

    //提交POST
    QNetworkRequest request(auth_url);//接口为auth_url
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant(CONTENT_TYPE));
    QNetworkAccessManager* nam = new QNetworkAccessManager();
    QByteArray req_bytes = make_single_json_str(posts);
    logs::write_log_kn("登录请求 post内容" + req_bytes);
    QNetworkReply* reply = nam->post(request,make_single_json_str(posts));
    QEventLoop loop;
    QTimer timer;

    //超时设置
    timer.setInterval(5000);//设置超时时间 5 秒
    timer.setSingleShot(true);//单次触发

    //绑定信号槽用于释放内存 以及 对POST返回值进行对应操作
    connect(nam,SIGNAL(finished(QNetworkReply*)),this,SLOT(auth_nam_finished(QNetworkReply*)));
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));       //返回信息   终止loop的正常情况
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));    //超时      终止loop的特殊情况
    reply_mutex.tryLock(0);
    reply_mutex.unlock();
    timer.start();
    loop.exec();

    //稍后释放reply，nam的内存以及信号槽绑定
    if(timer.remainingTime() == -1){
        /* 如果触发过timeout */
        //如果在 loop退出 到 超时判断前 这段时间，reply见缝插针，正常返回了，则不进行超时处理
        if(!reply_mutex.tryLock(0))
            return;
        tick_count = POSTTIMEOUT_WAIT_TICK;//网络连接超时，需要等待 TIMEOUT_WAIT_TICK 毫秒后才执行下一次
        network_tick_mutex.unlock();//tick锁解除，使下一次流程可以重新发起
        emit upload_reply_msg("通信超时！");
    }
    disconnect(reply,nullptr,nullptr,nullptr);//解除信号槽绑定
    disconnect(nam,nullptr,nullptr,nullptr);//解除信号槽绑定
    reply->deleteLater();
    nam->deleteLater();
}
//数据重复查询
void network::postDBCheck(){
    //临界区
    QHttpMultiPart mMultiPart(QHttpMultiPart::FormDataType);

    //获取文件头
    QFile file(network::last_filename);
    file.open(QFile::ReadOnly|QIODevice::Text);
    QByteArray data_head = file.readLine();
    file.close();

    //构造JSON
    /*
        {
            "authorization": "xxxx",//【data明文 + uuid的值】进行sha256,然后非对称加密
            "uuid": "xxx",//uuid非对称加密结果
            "data": "xxxx",//对称加密后的业务数据数据
        }
    data中的明文如下，data存放明文对称加密的内容
        {
            "busD":{业务数据},
            "timeStamp": "xxx",//时间戳，发送请求时的时间戳 长整型
         }
    */
    QJsonDocument json_doc;
    QJsonObject busD = str_to_json(data_head);//文件数据头转json对象
    //初步检查数据格式是否合理
    if(busD.isEmpty()){
        /* 数据头有误或者测量数据为空，跳过当前文件 */
        network::queue_cmpTopAndPop(last_filename);//删除队列头元素，开始下一个文件的执行
        status = file_status::WAIT;//标记下一个文件初始状态
        network_tick_mutex.unlock();//tick锁解除，使下一次流程可以重新发起
        emit upload_format_err(last_filename);//跳过
        emit upload_reply_msg("文件内容无法通过本地规则检查！");
        return;
    }
    //构建json
    QJsonObject data;//存放data json对象
    data.insert("busD",busD);
    auto now_time = QDateTime::currentDateTime();
    data.insert("timeStamp",tr("%1").arg(now_time.toMSecsSinceEpoch()));
    QByteArray msg_data = json_to_str(data);//构造data json字符串记录实际数据
    QByteArray msg_authorization = msg_data+network::uuid.toUtf8();//构造data和uuid的内容拼接字符串
    logs::write_log_kn("数据查重请求 data明文" + msg_data);

    //POST参数  [data] ,身份认证的实际数据,返回的数据已经是base64了
    QByteArray post_data;
    aes::aes128_encrypt(msg_data,post_data);//aes对称加密
    //POST参数  [authorization] ,用于验证[data]和[uuid]的正确和完整性，保证其未被hack
    QByteArray post_authorization = rsa::rsa_pub_encrypt_base64(sha256_hash(msg_authorization),rsa::pkey);
    //POST参数  [uuid] , 文件传输以及数据库查重的时候，服务器需要用户的uuid才能确定解密使用的key
    QByteArray post_uuid = rsa::rsa_pub_encrypt_base64(network::uuid,rsa::pkey);

    //构造POST,Json字符串
    QMap<QByteArray, QByteArray> posts;
    posts.insert("uuid",post_uuid);
    posts.insert("authorization",post_authorization);
    posts.insert("data", post_data);

    //提交POST
    QNetworkRequest request(db_url);//接口为auth_url
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant(CONTENT_TYPE));
    QNetworkAccessManager* nam = new QNetworkAccessManager();
    QByteArray req_bytes = make_single_json_str(posts);
    logs::write_log_kn("数据查重请求 post内容" + req_bytes);
    QNetworkReply* reply = nam->post(request,make_single_json_str(posts));
    QEventLoop loop;
    QTimer timer;

    //超时设置
    timer.setInterval(5000);//设置超时时间 5 秒
    timer.setSingleShot(true);//单次触发

    //绑定信号槽用于释放内存 以及 对POST返回值进行对应操作
    connect(nam,SIGNAL(finished(QNetworkReply*)),this,SLOT(db_nam_finished(QNetworkReply*)));
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));       //返回信息   终止loop的正常情况
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));    //超时      终止loop的特殊情况
    reply_mutex.tryLock(0);
    reply_mutex.unlock();
    timer.start();
    loop.exec();

    //稍后释放reply，nam的内存以及信号槽绑定
    if(timer.remainingTime() == -1){
        /* 如果触发过timeout */
        //如果在 loop退出 到 超时判断前 这段时间，reply见缝插针，正常返回了，则不进行超时处理
        if(!reply_mutex.tryLock(0))
            return;
        tick_count = POSTTIMEOUT_WAIT_TICK;//网络连接超时，需要等待 TIMEOUT_WAIT_TICK 毫秒后才执行下一次
        network_tick_mutex.unlock();//tick锁解除，使下一次流程可以重新发起
        emit upload_reply_msg("通信超时！");
    }
    disconnect(reply,nullptr,nullptr,nullptr);//解除信号槽绑定
    disconnect(nam,nullptr,nullptr,nullptr);//解除信号槽绑定
    reply->deleteLater();
    nam->deleteLater();
}
//文件上传
void network::postFile(){
    //临界区
    QHttpMultiPart mMultiPart(QHttpMultiPart::FormDataType);

    //获取文件头
    QFile file(network::last_filename);
    file.open(QFile::ReadOnly|QIODevice::Text);
    QByteArray data_head = file.readLine();
    //文件数据内容
    QJsonArray data_arr;//存放数据的json数组
    QByteArray get;
    while( !(get = file.readLine()).isEmpty()){
        data_arr.push_back(QString::fromUtf8(get).simplified());//去掉前后空格，去掉换行等特殊符，push到json array
    }
    file.close();
    //构造JSON
    /*
        {
            "authorization": "xxxx",//【data明文 + uuid的值】进行sha256,然后非对称加密
            "uuid": "xxx",//uuid非对称加密结果
            "data": "xxxx",//对称加密后的业务数据数据
        }
    data中的明文如下，data存放明文对称加密的内容
        {
            "busD":{业务数据},
            "timeStamp": "xxx",//时间戳，发送请求时的时间戳 长整型
         }
    */
    QJsonDocument json_doc;
    QJsonObject busD = str_to_json(data_head);//文件数据头转json对象
    //初步检查数据格式是否合理
    if(busD.isEmpty() || data_arr.isEmpty()){
        /* 数据头有误或者测量数据为空，跳过当前文件 */
        network::queue_cmpTopAndPop(last_filename);//删除队列头元素，开始下一个文件的执行
        status = file_status::WAIT;//标记下一个文件初始状态
        network_tick_mutex.unlock();//tick锁解除，使下一次流程可以重新发起
        emit upload_format_err(last_filename);//跳过
        emit upload_reply_msg("文件内容无法通过本地规则检查！");
        return;
    }
    //构建json
    busD.insert("oriData",data_arr);//插入数据文件
    QJsonObject data;//存放json对象
    data.insert("busD",busD);
    auto now_time = QDateTime::currentDateTime();
    data.insert("timeStamp",tr("%1").arg(now_time.toMSecsSinceEpoch()));
    QByteArray msg_data = json_to_str(data);//构造Json字符串记录实际数据
    QByteArray msg_authorization = msg_data+network::uuid.toUtf8();//构造data和uuid的内容拼接字符串
    logs::write_log_kn("文件上传请求 data明文" + msg_data);

    //POST参数  [data] ,身份认证的实际数据,返回的数据已经是base64了
    QByteArray post_data;
    aes::aes128_encrypt(msg_data,post_data);//aes对称加密
    //POST参数  [authorization] ,用于验证[data]和[uuid]的正确和完整性，保证其未被hack
    QByteArray post_authorization = rsa::rsa_pub_encrypt_base64(sha256_hash(msg_authorization),rsa::pkey);
    //POST参数  [uuid]
    QByteArray post_uuid = rsa::rsa_pub_encrypt_base64(network::uuid,rsa::pkey);

    //构造POST,Json字符串
    QMap<QByteArray, QByteArray> posts;
    posts.insert("uuid",post_uuid);
    posts.insert("authorization",post_authorization);
    posts.insert("data", post_data);

    //提交POST
    QNetworkRequest request(file_url);//接口为file_url
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant(CONTENT_TYPE));
    QNetworkAccessManager* nam = new QNetworkAccessManager();
    QByteArray req_bytes = make_single_json_str(posts);
    logs::write_log_kn("文件上传请求 post内容" + req_bytes);
    QNetworkReply* reply = nam->post(request,make_single_json_str(posts));
    QEventLoop loop;
    QTimer timer;

    //超时设置
    timer.setInterval(5000);//设置超时时间 5秒
    timer.setSingleShot(true);//单次触发

    //绑定信号槽用于释放内存 以及 对POST返回值进行对应操作
    connect(nam,SIGNAL(finished(QNetworkReply*)),this,SLOT(file_nam_finished(QNetworkReply*)));//对返回信息的处理
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));       //返回信息   终止loop的正常情况
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));    //超时      终止loop的特殊情况
    reply_mutex.tryLock(0);
    reply_mutex.unlock();
    timer.start();
    loop.exec();

    //稍后释放reply，nam的内存以及信号槽绑定
    if(timer.remainingTime() == -1){
        /* 如果触发过timeout */
        //如果在 loop退出 到 超时判断前 这段时间，reply见缝插针，正常返回了，则不进行超时处理
        if(!reply_mutex.tryLock(0))
            return;
        tick_count = POSTTIMEOUT_WAIT_TICK;//网络连接超时，需要等待 TIMEOUT_WAIT_TICK 毫秒后才执行下一次
        network_tick_mutex.unlock();//tick锁解除，使下一次流程可以重新发起
        emit upload_reply_msg("通信超时！");
    }
    disconnect(reply,nullptr,nullptr,nullptr);//解除信号槽绑定
    disconnect(nam,nullptr,nullptr,nullptr);//解除信号槽绑定
    reply->deleteLater();
    nam->deleteLater();
}

/**
 * @brief reply_operation 对reply的响应操作函数
 * @param reply
 */
void network::reply_operation(QNetworkReply *reply){
    //如果在 超时判断生效后，reply正常返回了，依然进行超时处理，reply响应操作将被取消，然后return
    if(!reply_mutex.tryLock(0))
        return;
    QByteArray get_reply = reply->readAll();
    if(get_reply.isEmpty()){
        /* 上传失败，网络连接失败的特殊情况,reply为空 */
#ifdef DEBUG
        get_reply = "{"
                    "\"authorization\": \"Eoamxe7nyxSVI8tqCwDdB4nK9F2YqxAUyhiNTC4871hytjPkDjRMr/yp+ItqOcq6CAAjTEW8IjDxkfl3pryU/u13k2j2gD8zD19EjudaXzLc7A7jwfH8KNMzfry2Xd4z6vQYdlvJB0xsthswNy5IIkhEAanzBmcNSlBiYIYUbUo=\","
                    "\"data\": \"G05w0QmZ++e6cB4NeQzmt9Ng0h9F6VRT9C7Y3k4rbVIT69YzjWnsMRA8uf9pBVynLCFI80HKFI0XGxz/M04KCcQAyqS0Fg5MnWlaJ9kLd/I=\""
               "}";
#else
        tick_count = NETERR_WAIT_TICK;//网络连接失败，需要等待 NETERR_WAIT_TICK 毫秒后才执行下一次
        network_tick_mutex.unlock();
        emit upload_reply_msg("网络错误！正在重试...");//网络错误信号，用于触发主窗体网络状态显示
        return;
#endif
    }
    logs::write_log_kn("服务器返回消息" + get_reply);
    QJsonObject get_json = str_to_json(get_reply);
    if(get_json.isEmpty()){
        /* 返回信息转json失败，返回数据错误，重试 */
json_err:
/* json解析失败 */
        status = file_status::NEEDLOGIN;//标记需要身份验证
        tick_count = LOGIN_WAIT_TICK;//重新登录情况（同登录失败）在下次发起线程时等待一段时间
        network_tick_mutex.unlock();
        emit upload_reply_msg("接收数据Json格式解析失败！重新身份认证...");
        return;
    }
    /* 外层json */
    if((!get_json.contains("authorization")) || (!get_json.contains("data")))
        /* 关于goto语句，goto适合处理流程跳转的情景，
         * 但是goto已被证明可以被其他手段代替，在逻辑不清晰时goto会产生很多麻烦，
         * 请不要滥用 */
        goto json_err;
    QByteArray auth_rsa = get_json.take("authorization").toString().toUtf8();
    QByteArray dat_rsa = get_json.take("data").toString().toUtf8();
    //获取data的明文
    QByteArray data_decrypt;//对称解密后的data明文
    aes::aes128_decrypt(dat_rsa,data_decrypt);
    QByteArray data_hash = sha256_hash(data_decrypt);//data明文sha256后
    QByteArray auth_decrypt = rsa::rsa_pub_decrypt_base64(auth_rsa,rsa::pkey);//auth非对称解密后
    logs::write_log_kn("服务器返回消息 校验" "\n"
                       "data解码" + data_decrypt + "\n"
                       + "data解码后sha256" + data_hash + "\n"
                       + "authorization解码" + auth_decrypt);
    if(data_hash != auth_decrypt){
        /* auth摘要校验失败，说明数据可能被篡改了 */
//auth_err:
/* 摘要校验失败 */
        status = file_status::NEEDLOGIN;//标记需要身份验证
        tick_count = LOGIN_WAIT_TICK;//重新登录情况（同登录失败）在下次发起线程时等待一段时间
        network_tick_mutex.unlock();
        emit upload_reply_msg("接收数据摘要校验失败！重新身份认证...");
        return;
    }

    QJsonObject data_json = str_to_json(data_decrypt);//data的明文json对象
    /* data 二层json */
    if((!data_json.contains("busD")) || (!data_json.contains("timeStamp")))
        goto json_err;
    QJsonObject busD = data_json.take("busD").toObject();
    if(busD.isEmpty() || (!busD.contains("code")) || (!busD.contains("msg")))
        goto json_err;
    QString code = busD.take("code").toString();//状态码
    QString msg = busD.take("msg").toString();//返回信息
    auto time_stamp = data_json.take("timeStamp");//时间戳字符串
    qint64 now_time_stamp = -1;//获取时间戳
    if(time_stamp.isString())
        now_time_stamp = time_stamp.toString().toLongLong();
    else if (time_stamp.isDouble())
        now_time_stamp = (long long)(time_stamp.toDouble());

    logs::write_log_kn("服务器返回消息 状态" "\n"
                       "code" + code + "\n"
                       + "msg" + msg + "\n"
                       + "timestamp" + time_stamp.toString());

    /* 时间戳验证，必须大于上一次reply信息的时间戳，否则判断消息无效， */
#ifdef DEBUG
    if(now_time_stamp < last_time_stamp){
#else
    if(now_time_stamp <= last_time_stamp){
#endif
timestamp_err:
        /* 时间戳校验错误 */
        status = file_status::NEEDLOGIN;//标记需要身份验证
        tick_count = LOGIN_WAIT_TICK;//重新登录情况（同登录失败）在下次发起线程时等待一段时间
        network_tick_mutex.unlock();
        emit upload_reply_msg("时间戳错误！重新身份认证...");
        return;
    }
    last_time_stamp = now_time_stamp;//更新时间戳

    /* 状态码的处理 */
    if(code == LOGIN_OUT){
auth_ope:
        /* 身份错误，or 需要重新验证身份 */
        status = file_status::NEEDLOGIN;//标记需要身份验证
        tick_count = LOGIN_WAIT_TICK;//登录的失败情况在下次发起线程时等待一段时间
        network_tick_mutex.unlock();
        emit upload_reply_msg(msg);//身份认证错误，用于触发主窗体网络状态显示
        return;
    }
    if(code == ADD_SUCCESS){
        /* 上传成功，给文件做个标记 ,（成功返回的字符串是和服务器开发那边协商出来的 */
        //如果队列内容未被重置，头元素为last_filename，删除头元素
        network::queue_cmpTopAndPop(last_filename);//删除队列头元素，开始下一个文件的执行
        repeat_time = 0;//重复请求次数清0
        network_tick_mutex.unlock();//给Timer解锁，让它后面能持续检测待上传文件
        status = file_status::WAIT;//标记下一个文件初始状态
        emit upload_success(last_filename);//上传成功信号，用于触发主窗体列表显示
        //新建一个文件已上传标志
        QFileInfo fileInfo(last_filename);
        QString dir = fileInfo.path();//目录
        QString filename = fileInfo.fileName();//文件全名
        /* 假设文件 '/home/myfile.txt' 上传成功 , 会生成标记文件 '/home/uploaded[myfile.txt]' 用于告知该文件已经上传过 */
        QFile flag_file(dir + "/uploaded[" + filename + "]");
        flag_file.open( QIODevice::ReadWrite | QIODevice::Text );//创建标记
        flag_file.close();
        return;
    }
    if(code == ADD_FAIL){
        /* 上传文件已被正确接收，但其业务数据格式错误，直接跳过该文件 */
        emit upload_format_err(last_filename);//上传格式错误
        goto skip;
    }
    if(code == AUTH_FAIL){
        /* 校验错误，网络数据包有问题，需要重新身份认证 */
        /* 该情况下由于数据无法解密，是在 “json解析” 阶段就被捕获终止，原理上不会得到状态码，也就不会进入这个条件判断中 */
        /* 所以这里只是记录出现该情况时需要进行的操作 */
        goto auth_ope;
    }
    if(code == CHECK_FAIL){
        /* 请求错误，数据业务格式问题， 重试跳过*/
        emit upload_skip(last_filename);//跳过
        goto skip;
    }
    if(code == LOGIN_SUCCESS){
        /* 认证成功 */
        last_auth_time = QDateTime::currentDateTime();//更新上次认证时间
        status = file_status::WAIT;//验证已经成功,进入下一状态
        network_tick_mutex.unlock();
        return;
    }
    if(code == LOGIN_FAIL){
        /* 身份认证失败*/
        goto auth_ope;
    }
    if(code == REPEAT){
        /* 数据库发现重复，标记状态为“数据重复” */
        status = file_status::INDB;
        network_tick_mutex.unlock();
        emit upload_reply_msg(msg);
        return;
    }
    if(code == NOREPEAT){
        /* 数据库未发现重复，可以开始上传文件 */
        status = file_status::NINDB;//验证已经成功,进入下一状态
        network_tick_mutex.unlock();
        return;
    }

    /* 其他状态码 或 状态码为空 的情况 */
    //重试后不行再跳过处理
try_and_skip:
    if(++repeat_time <= REPEAT_MAX){
        /* 可以重试 */
        tick_count = TRYAGAIN_WAIT_TICK;//重试的情况需要在下次发起线程前等待一段时间
        network_tick_mutex.unlock();
        emit upload_reply_msg(msg + " 重试 x"+QString::number(repeat_time));
        return;
    }
    /* 重试次数不足，跳过该文件 */
    emit upload_skip(last_filename);//跳过
skip:
    repeat_time = 0;//重复请求次数清0
    //如果队列内容未被重置，头元素为last_filename，删除头元素
    network::queue_cmpTopAndPop(last_filename);//删除队列头元素，开始下一个文件的执行
    status = file_status::WAIT;//标记下一个文件初始状态
    network_tick_mutex.unlock();
    emit upload_reply_msg(msg);
    return;
}

//文件POST进程结束SLOT
void network::file_nam_finished(QNetworkReply *reply){
    reply_operation(reply);
}
//身份认证参数POST进程结束SLOT
void network::auth_nam_finished(QNetworkReply *reply){
    reply_operation(reply);
}
//数据库查询重复POST进程结束SLOT
void network::db_nam_finished(QNetworkReply *reply){
    reply_operation(reply);
}
