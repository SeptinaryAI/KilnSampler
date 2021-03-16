#include "logs.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QDateTime>

void logs::write_log(log_type type, QString content){
    QFile file("./logs.txt");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append)){
        file.close();
        return;
    }
    QString head = "[]";
    if(type == log_type::NORMAL)
        head = "[NORMAL]";
    else if(type == log_type::WARNING)
        head = "[WARNING]";
    else
        head = "[ERROR]";
    QString wri = head
            + " " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
            + "\n"
            + content
            + "\n";

    file.write(wri.toUtf8() + "\n");
    file.close();
}

void logs::write_log(QString content){
    write_log(log_type::NORMAL, content);
}

void logs::write_log_kn(log_type type, QString content){
    QFile file("./logs_kernel.txt");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append)){
        file.close();
        return;
    }
    QString head = "[]";
    if(type == log_type::NORMAL)
        head = "[NORMAL]";
    else if(type == log_type::WARNING)
        head = "[WARNING]";
    else
        head = "[ERROR]";
    QString wri = head
            + " " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
            + "\n"
            + content
            + "\n";

    file.write(wri.toUtf8() + "\n");
    file.close();
}

void logs::write_log_kn(QString content){
    write_log_kn(log_type::NORMAL, content);
}
