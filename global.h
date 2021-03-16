#ifndef GLOBAL_H
#define GLOBAL_H
#include "Device/IDevice.h"

class Global
{
public:
    Global();
    static void device_choose_init();
    static void project_choose_init();
};

#endif // GLOBAL_H
