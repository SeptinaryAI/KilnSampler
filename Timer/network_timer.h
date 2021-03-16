#ifndef NETWORK_TIMER_H
#define NETWORK_TIMER_H

#include <QTimer>

class network_timer {
private:
    static QTimer *timer;

public:
    network_timer(){}
    static QTimer* get_timer(){
        return timer;
    }
};




#endif
