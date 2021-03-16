#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Profile/profile.h"
#include "Project/IProject.h"

//配置相关初始化
void MainWindow::profile_init(){
    profile::read_config_to_map();//软件设置文件读取
}

