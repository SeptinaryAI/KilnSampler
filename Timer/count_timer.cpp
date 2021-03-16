#include "count_timer.h"
#include <QTime>
//饿汉
QTimer* count_timer::timer = new QTimer();
qint64 count_timer::start_time = 0;

void count_timer::set_start_time(qint64 time){
    start_time = time;
}
qint64 count_timer::get_start_time(){
    return start_time;
}
