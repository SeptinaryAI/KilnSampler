#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Project/IProject.h"
#include "global.h"
#include <QMap>
#include <QPointer>

//可选测窑项目相关初始化
void MainWindow::project_init(){
    ui->choose_project->clear();
    fake_map<QString , IProject*> map = IProject::get_project_map();
    if(map.isEmpty()){
        Global::project_choose_init();
        map = IProject::get_project_map();
    }
    auto itor = map.begin();
    while(itor != map.end()){
        ui->choose_project->addItem(itor->first, QVariant::fromValue(itor->second));
        ++itor;
    }
    ui->choose_project->setCurrentIndex(0);
    project_choose();
}

//删除布局中的所有控件
static void delete_all_items_of_widget(QWidget* widget){
    if(widget->layout() != nullptr){
        QLayoutItem * item;
        while((item = widget->layout()->takeAt(0))!= nullptr){
            delete item->widget();
            delete item;
        }
    }
    delete widget->layout();
}

void MainWindow::project_choose(){
    IProject* pr = ui->choose_project->currentData().value<IProject*>();
    QString pr_name = pr->get_name();
    IProject::set_instance(pr);//改变项目
    ui->tab_dev->setCurrentIndex(pr->get_def_device());//使用默认设备来选择具体的设备
    logs::write_log("选择了 " + IProject::get_instance()->get_name()+" 测窑项目| 使用设备 " + IDevice::get_instance()->get_name());
    ui->statusBar->showMessage("当前选择的业务是: "+IProject::get_instance()->get_name() + "| 使用默认设备: "+IDevice::get_instance()->get_name());

    //额外参数读取
    QPointer<QGridLayout> para_layout = new QGridLayout();
    int col = 0;//控件放置位置的column
    int row = 0;//控件放置位置的row
    int col_num = 4;//一行放几个参数
    for(auto& para : IProject::get_instance()->get_para_list()){
        if(col + 1 > col_num){
            col = 0;
            ++row;
        }
        para->place_widget(para_layout,row,col++ * 2);
    }
    delete_all_items_of_widget(ui->groupBox_para);//删掉控件的所有子控件以及所有布局
    ui->groupBox_para->setLayout(para_layout);//设置新的布局
}
