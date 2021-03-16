#ifndef LOGS_H
#define LOGS_H
#include <QFile>

enum log_type{
    NORMAL,
    WARNING,
    ERROR
};

class logs{
public:
    static void write_log(log_type, QString);
    static void write_log(QString);
    static void write_log_kn(log_type, QString);
    static void write_log_kn(QString);
};

#endif













