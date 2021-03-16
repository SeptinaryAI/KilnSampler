#ifndef IPROJECT_H
#define IPROJECT_H
#include <QString>
#include <QObject>
#include "IPara.h"
//测窑项目
class IProject : public QObject{
    Q_OBJECT
private:
    QString project_name;
    static IProject* instance;//全局唯一常驻对象，可被替换为各个具体项目对象
    static fake_map<QString, IProject*> project_map;//记录了所有可用测窑项目的map
    int def_device;//默认设备编号
    QList<IPara*> para_list;//测窑项目特有参数列表
public:
    IProject();
    IProject(QString name);
    QString get_name() {return project_name;}
    QList<IPara*> get_para_list();
    void add_para(QString para_name, QString para_key, para_type para_type);//无combobox
    void add_para(QString para_name, QString para_key, para_type para_type, QMap<QString, QString> map);//map对应combobox类型的下拉框候选项
    void add_para(QString para_name, QString para_key, para_type para_type, QString tip);//无combobox,有tip
    void add_para(QString para_name, QString para_key, para_type para_type, QMap<QString, QString> map, QString tip);//map对应combobox类型的下拉框候选项
    void add_para(QString para_name, QString para_key, para_type para_type, fake_map<QString, QString> list);//map对应combobox类型的下拉框候选项
    void add_para(QString para_name, QString para_key, para_type para_type, fake_map<QString, QString> list, QString tip);//map对应combobox类型的下拉框候选项
    void set_def_device(int);//设置默认设备
    int get_def_device();//获取默认设备
    static IProject* get_instance(){return instance;}//可切换单例
    static void set_instance(IProject*);//切换当前选择的测窑项目，单例可切换

    static fake_map<QString, IProject*> get_project_map();//获取可用测窑项目map
    static void project_map_clear();//清空可用测窑项目map
    static void project_map_add(QString, IProject*);//添加项目到 可用测窑项目map
};

#endif
