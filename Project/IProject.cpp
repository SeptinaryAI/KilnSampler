#include "Project/IProject.h"
#include "Project/fake_map.h"
#include <QMap>
//单例：被选中的项目对象
IProject* IProject::instance = new IProject("base");
fake_map<QString, IProject*> IProject::project_map = fake_map<QString, IProject*>();

IProject::IProject(){
    project_name = "default";
}
IProject::IProject(QString name){
    project_name = name;
}

/**
 * @brief IProject::set_instance 修改当前所使用的测窑项目
 * @param pr 可变单例的指针（地址
 */
void IProject::set_instance(IProject* pr){
    instance = pr;
}

/**
 * @brief IProject::get_def_device 获取当前项目的默认设备
 * @return
 */
int IProject::get_def_device(){
    return def_device;
}

/**
 * @brief IProject::set_def_device 设置当前项目的默认设备
 * @param dev_num 设备号（对应界面的标签栏排序
 */
void IProject::set_def_device(int dev_num){
    def_device = dev_num;
}

/**
 * @brief IProject::get_project_map 获取测窑项目表
 * @return
 */
fake_map<QString, IProject*> IProject::get_project_map(){
    return project_map;
}

/**
 * @brief IProject::project_map_clear 清空所有测窑项目
 */
void IProject::project_map_clear(){
    project_map.clear();
}

/**
 * @brief IProject::project_map_add 添加项目
 * @param i
 * @param pr
 */
void IProject::project_map_add(QString i, IProject* pr){
    project_map.insert(i,pr);
}

/**
 * @brief IProject::get_para_list 获取当前项目的参数表
 * @return
 */
QList<IPara*> IProject::get_para_list(){
    return para_list;
}
/**
 * @brief IProject::add_para 向测窑项目添加一个参数，（下拉框参数除外）
 * @param para_name 参数名
 * @param para_key 参数的身份标识
 * @param para_type 参数类型，参考IPara.h
 */
void IProject::add_para(QString para_name, QString para_key, para_type para_type){
    add_para(para_name,para_key,para_type,"");
}
/**
 * @brief IProject::add_para 向测窑项目添加一个下拉框参数
 * @param para_name 参数名
 * @param para_key 参数的身份标识
 * @param para_type 参数类型，参考IPara.h
 * @param map 下拉框默认的可选项表
 */
void IProject::add_para(QString para_name, QString para_key, para_type para_type, QMap<QString, QString> map){
    add_para(para_name,para_key,para_type,map,"");
}

/**
 * @brief IProject::add_para 向测窑项目添加一个参数，（下拉框参数除外）
 * @param para_name 参数名
 * @param para_key 参数的身份标识
 * @param para_type 参数类型，参考IPara.h
 * @param tip   参数在用户界面，如果鼠标移动上去，停留，会有文字提醒，tip就是提醒文字
 */
void IProject::add_para(QString para_name, QString para_key, para_type para_type, QString tip){
    IPara* para_add = new IPara(para_name,para_key,para_type,tip);
    para_list.append(para_add);
}

/**
 * @brief IProject::add_para 向测窑项目添加一个下拉框参数
 * @param para_name 参数名
 * @param para_key 参数的身份标识
 * @param para_type 参数类型，参考IPara.h
 * @param map 下拉框默认的可选项表
 * @param tip 参数在用户界面，如果鼠标移动上去，停留，会有文字提醒，tip就是提醒文字
 */
void IProject::add_para(QString para_name, QString para_key, para_type para_type, QMap<QString, QString> map, QString tip){
    IPara* para_add = new IPara(para_name,para_key,para_type,tip);
    auto itor = map.begin();
    while(itor != map.end()){
        para_add->insert_item(itor.key(),itor.value());
        ++itor;
    }
    para_list.append(para_add);
}

/**
 * @brief IProject::add_para 向测窑项目添加一个下拉框参数，该函数使用了fake_map来避免QMap无法根据参数加入顺序排序的问题
 * @param para_name 参数名
 * @param para_key 参数的身份标识
 * @param para_type 参数类型，参考IPara.h
 * @param list 下拉框默认的可选项表
 */
void IProject::add_para(QString para_name, QString para_key, para_type para_type, fake_map<QString, QString> list){
    add_para(para_name,para_key,para_type,list,"");
}

/**
 * @brief IProject::add_para 向测窑项目添加一个下拉框参数，该函数使用了fake_map来避免QMap无法根据参数加入顺序排序的问题
 * @param para_name 参数名
 * @param para_key 参数的身份标识
 * @param para_type 参数类型，参考IPara.h
 * @param list 下拉框默认的可选项表
 * @param tip 参数在用户界面，如果鼠标移动上去，停留，会有文字提醒，tip就是提醒文字
 */
void IProject::add_para(QString para_name, QString para_key, para_type para_type, fake_map<QString, QString> list, QString tip){
    IPara* para_add = new IPara(para_name,para_key,para_type,tip);
    auto itor = list.begin();
    while(itor != list.end()){
        para_add->insert_item(itor->first,itor->second);
        ++itor;
    }
    para_list.append(para_add);
}

