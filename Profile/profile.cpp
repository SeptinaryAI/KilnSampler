#include "profile.h"
#include <QFile>
#include <QtXml/QDomDocument>
#include <QMessageBox>
#include <QTextStream>
#include "Device/IDevice.h"
#include "Project/IProject.h"

//config参数map
QMap<QString, QString> profile::config_map = QMap<QString, QString>();

//读取config配置，生成配置参数map, 即初始化
void profile::read_config_to_map(){
    QFile file("./config.txt");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(nullptr,"Warning","全局设置文件丢失!");
        file.close();
        return;
    }
    QTextStream in(&file);
    QString line;
    while(!(line = in.readLine()).isEmpty()){
        QString pre = line.split('#')[0];
        if(!pre.contains('='))
            continue;
        QStringList arr = pre.split('=');
        QString key = arr[0].trimmed();
        if(key.isEmpty() || config_map.contains(key))
            continue;
        QString val = arr[1].trimmed();
        config_map.insert(arr[0].trimmed(), arr[1].trimmed());
    }
    config_map.size();
    file.close();
}

//读指定的配置参数,如果读不到，返回默认值default_ret
QString profile::read_config(QString key, QString default_ret){
    if(config_map.contains(key)){
        return config_map[key];
    } else {
        return default_ret;
    }
}
