#ifndef COUNT_TIMER_H
#define COUNT_TIMER_H

#include <QTimer>

class count_timer {
private:
    static QTimer *timer;
    static qint64 start_time;
public:
    count_timer(){}
    static QTimer* get_timer(){
        return timer;
    }
    static void set_start_time(qint64);
    static qint64 get_start_time();
};




#endif
