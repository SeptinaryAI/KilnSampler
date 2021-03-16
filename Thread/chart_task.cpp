#include "Thread/chart_task.h"
#include "Chart/IChart.h"
#include <QMutex>
static QMutex thread_mutex;
/**
 * @brief chart_task::run 图表更新的多线程函数
 */
void chart_task::run(){
    if(!thread_mutex.tryLock(0))
        return;
    //临界区
    IChart::get_instance()->task_display();
    thread_mutex.unlock();
}
