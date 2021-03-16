#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDesktopServices>
#include "Network/network.h"
#include "Project/IProject.h"
//文件相关初始化，配置默认路径
void MainWindow::file_init(){
    ui->text_path->setText(QDir::currentPath()+ "/data");
    ui->text_path->setToolTip(QDir::currentPath() + "/data");
}
//选择数据保存路径
void MainWindow::choose_dir(){
    QString dir = QFileDialog::getExistingDirectory();
    if(dir.isEmpty())
        return;
    ui->text_path->setText(dir);
    ui->text_path->setToolTip(dir);
    //如果“自动上传总目录”被勾选，则触发自动上传
    if(ui->check_autoupload->isChecked()){
        //模拟一次手动触发重新上传
        ui->check_autoupload->setChecked(false);
        ui->check_autoupload->setChecked(true);
    }
}

/**
 * @brief make_para 构建参数格式的方法
 * @param args 引用map，会将构建好的参数插入map
 * @param key 参数名
 * @param val 参数值
 * @return 构建好的参数表示字符串
 * @remark Ex:   传入 ("time","10;12;15") ，去掉一些特殊字符后 ， 构建为 map的一个键值对
 */
static void make_para(QMap<QString,QString>&args, QString key, QString val){
    QString key_ = key.replace("=","_").replace(";","_") ;
    QString val_ = val.replace("=","_").replace(";","_") ;
    args.insert(key_,val_);
}
/**
 * @brief is_int 判断字符串是否为整数  正负均可
 * @return
 */
//static bool is_int(QString in){
//    //必须为正负整数
//    QRegExp rx("-?[0-9]+");
//    QValidator *validator = new QRegExpValidator(rx);
//    int pos = 0;
//    auto rst = validator->validate(in.replace(" ",""), pos);
//    delete validator;
//    if(rst == QValidator::Acceptable)
//        return true;
//    return false;
//}

/**
 * @brief get_project_name
 * @return 项目名称
 * @remark 获取保存用的项目名称
 * @remark 如果有自定义项目，则优先使用自定义项目中设置的项目名，如果自定义项目没有项目名，则使用选择的项目名
 */
QString get_project_name(){
    for(auto& para : IProject::get_instance()->get_para_list()){
        //测窑项目额外参数中有项目名称，代替“自定义项目”
        if(para->get_para_key() == "project"){
            return "[自定义项目]" + para->get_para_value();
        }
    }
    return IProject::get_instance()->get_name();
}

/**
 * @brief get_subpath_by_project
 * @return 通过当前项目获得保存数据文件的子目录结构定义
 */
QString get_subpath_by_project(){
    QString name = get_project_name();
    if(name == "section"){
        /* 筒体截面 */
        /* 筒体截面测量将各个截面放在同一个文件夹下，便于查看 */
        return "";
    } else if(name == "ovality"){
        /* 椭圆度 */
        /* 椭圆度按照测量对象划分文件夹 */
        for(auto& para : IProject::get_instance()->get_para_list()){
            if(para->get_para_key() == "gear"){
                return "/" + para->get_para_value();
            }
        }
        return "/unknown";
    }
    //else 自定义项目同 筒体截面测量
    return "";
}
/**
 * @brief get_filename_by_project
 * @param times 测量次数
 * @return 通过当前项目获得保存数据文件的命名
 */
QString get_filename_by_project(QString times){
    QString name = get_project_name();
    if(name == "section"){
        /* 筒体截面 */
        QString p1 = "unkn";//sectionNo 截面号
        QString p2 = "unkn";//axialDistance 截面位于 (位置长度
        for(auto& para : IProject::get_instance()->get_para_list()){
            if(para->get_para_key() == "axialDistance"){
                p1 = para->get_para_value() + "m";
            }
            if(para->get_para_key() == "startAngle"){
                p2 = para->get_para_value() + "°";
            }
        }
        return p1 + "-" + p2 + "--" + times + "次" + ".txt";
    } else if(name == "ovality"){
        /* 椭圆度 */
        QString p1 = "unkn";//dotType 点位
        QString p2 = "unkn";//到轮带的距离
        for(auto& para : IProject::get_instance()->get_para_list()){
            if(para->get_para_key() == "dotType"){
                p1 = para->get_para_value();
            }
            if(para->get_para_key() == "distance"){
                p2 = para->get_para_value();
            }
        }
        return p1 + "-" + p2 + "--" + times + "次" + ".txt";
    }
    //else
    QString p1 = "unkn";//自定义的测量对象（如果有的话
    QString p2 = "unkn";//自定义的测量位置（如果有的话
    for(auto& para : IProject::get_instance()->get_para_list()){
        if(para->get_para_key() == "object"){
            p1 = para->get_para_value();
        }
        if(para->get_para_key() == "location"){
            p2 = para->get_para_value();
        }
    }
    return p1 + "-" + p2 + "--" + times + "次" + ".txt";
}

//数据保存为文件，同时文件第一行构造参数，包括默认参数【公司，窑号，测量对象，位置，测量次数、测量用时...】 以及 【所选测窑项目的额外可用参数】
void MainWindow::save_data_to_file(){
    QString project = get_project_name();//获取测窑项目名称
    QString device = IDevice::get_instance()->get_name();//获取使用测量设备名称
    QString company = ui->text_company->text();//获取公司名称
    QString number = ui->text_kilnnum->text();//获取窑号
    QString times = QString::number(ui->times->value());//获取测量次数
    QString time_count = ui->timer_count->text();//获取该次测量过程总时间的计时
    QString data = ui->data_switch->toPlainText();//获取该次测量取得的数据流，将会存到文件中

    /* 保存规则判断 */
    if(project.replace(" ","").isEmpty() || project.replace(" ","") == "[自定义项目]"){
        QMessageBox::about(nullptr,"Error","保存数据的 <测窑项目名称> 不能为空！ \r\n如果当前使用了[自定义项目],请输入自定义项目的 <项目名称> 字段后再保存！");
        return;
    }
    if(company.replace(" ","").isEmpty()){
        QMessageBox::about(nullptr,"Error","保存数据的 <公司名称> 不能为空！ 请输入 <公司名称> 后再保存！");
        return;
    }
    if(number.replace(" ","").isEmpty()){
        QMessageBox::about(nullptr,"Error","保存数据的 <窑号> 不能为空！ 请输入 <窑号> 后再保存！");
        return;
    }

    /* 目录结构与数据文件建立 */
    QString choose_dir = ui->text_path->toPlainText();
    if(choose_dir.isEmpty())
        choose_dir = "./data";//默认当前目录
    QString build_dir = choose_dir + "/" + company + "/" + number + "/" + project + get_subpath_by_project();
    QDir dir;
    if(!dir.exists(build_dir)){
        bool ret = dir.mkpath(build_dir);
        if(ret == false){
            status_show_slot("保存目录创建失败!");
            QMessageBox::warning(nullptr,"Error",
                                 "保存目录创建失败！请检查<公司名称>、<窑号>等字段是否合法！\r\n请不要使用除下划线“_”以外的其他特殊字符!",
                                 QMessageBox::Ok);
            return;
        }
    }
    QString file_name =build_dir + "/" + get_filename_by_project(times);
    QFile file(file_name);
    if(file.exists()){
        QMessageBox::StandardButton btn =
            QMessageBox::warning(nullptr,"Warning","数据文件已存在于本地磁盘，是否覆盖?",QMessageBox::Yes|QMessageBox::No);
        if(btn == QMessageBox::No){
            return;
        }
    }
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        status_show_slot("数据文件创建失败!");
        QMessageBox::warning(nullptr,"Error",
            "数据文件创建失败！请检查<当前项目各输入参数>的字符串是否合法！\r\n请不要使用除下划线“_”以外的其他特殊字符!请使用管理员身份运行！",QMessageBox::Ok);
        return;
    }

    /* 文件头部参数构建 */
    QMap<QString,QString> args;
    for(auto& para : IProject::get_instance()->get_para_list()){
        make_para(args, para->get_para_key() ,  para->get_para_value());
    }
    //根据java web端的数据库字段规则命名key
    make_para(args, "project", project );
    make_para(args, "corporation", company );
    make_para(args, "kilnNo", number );
    make_para(args, "number", times );
    auto now_time = QDateTime::currentDateTime();
    make_para(args, "time", now_time.toString("yyyy-MM-dd") );
    make_para(args, "timeMeasure", now_time.toString("yyyy-MM-dd hh:mm:ss.zzz") );
    make_para(args, "timeStamp", tr("%1").arg(now_time.toMSecsSinceEpoch()) );
    make_para(args, "cycle", tr("%1").arg(time_count) );
    file.write(make_single_json_str(args));
    file.write("\n");
    file.write(data.toUtf8());
    file.close();

    /* 判断“自动上传总目录” 是否勾选，如果是，则push到上传队列 */
    if(ui->check_autoupload->isChecked())
        network::queue_push(file_name);

    /* 输出提示信息 */
    logs::write_log("数据 " + file_name + " 保存成功！");
    status_show_slot("数据保存成功! 位于: "+file_name);
    QMessageBox::about(nullptr,"Notify","数据保存成功! 位于: \r\n"+file_name);
}
//打开保存目录
void MainWindow::open_save_ptah(){
    QString path = ui->text_path->toPlainText();
    QDesktopServices::openUrl(QUrl("file:"+path, QUrl::TolerantMode));
}
