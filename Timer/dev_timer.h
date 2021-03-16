#ifndef DEV_TIMER_H
#define DEV_TIMER_H

#include <QTimer>

class dev_timer {
private:
    static QTimer *timer;

public:
    dev_timer(){}
    static QTimer* get_timer(){
        return timer;
    }
};




#endif
