#ifndef CHART_TASK_H
#define CHART_TASK_H

#include <QRunnable>

class chart_task : public QRunnable
{
public:
    chart_task(){}
    void run() override;
};


#endif
