#ifndef PROFILE_H
#define PROFILE_H
#include <QtXml/QDomDocument>
#include <QMap>

//主要管理配置相关的文件以及相关函数
//软件的全局设置文件config.txt,配置软件的各项默认参数，例如“网络模块的默认接口地址”等
class profile{
public:
    //config
    static QMap<QString, QString> config_map;
    static void read_config_to_map();//读取config配置，生成配置参数map, 即初始化
    static QString read_config(QString key, QString default_ret);//读指定的配置参数,如果读不到，返回默认值default_ret
};

#endif
