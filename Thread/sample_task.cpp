#include "Thread/sample_task.h"
#include "Device/IDevice.h"

#include <QMutex>
static QMutex thread_mutex;
/**
 * @brief sample_task::run 采集数据的多线程函数，周期执行，周期由Device决定
 */
void sample_task::run(){
    if(!thread_mutex.tryLock(0))
        return;
    //临界区
    //IDevice::get_instance()->sendcmd();
    IDevice::get_instance()->sample();
    thread_mutex.unlock();
}
