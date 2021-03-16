#ifndef SAMPLE_TASK_H
#define SAMPLE_TASK_H

#include <QRunnable>

class sample_task : public QRunnable
{
public:
    sample_task(){}
    void run() override;
};


#endif
