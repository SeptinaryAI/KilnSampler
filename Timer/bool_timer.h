#ifndef BOOL_TIMER_H
#define BOOL_TIMER_H

#include <QTimer>

class bool_timer {
private:
    static QTimer *timer;

public:
    bool_timer(){}
    static QTimer* get_timer(){
        return timer;
    }
};




#endif
