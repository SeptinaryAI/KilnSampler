#include "global.h"
#include "Device/device_ellipsometer_classical.h"
#include "Device/device_ellipsometer_new.h"
#include "Device/device_laser_big.h"
#include "Device/device_laser_small.h"
#include "Project/IProject.h"
#include "Project/IPara.h"
#include <QString>
#include <QMap>
#include "Project/fake_map.h"

//可选串口设备初始化
void Global::device_choose_init(){
    IDevice::dev_map_clear();
    //这里的数字要和界面的tab编号一一对应
    IDevice::dev_map_add(0,new device_laser_big());
    IDevice::dev_map_add(1,new device_ellipsometer_classical());
    IDevice::dev_map_add(2,new device_laser_small());
    IDevice::dev_map_add(3,new device_ellipsometer_new());
    //IDevice::dev_map_add(4,new device_you_add());//可自行添加新串口设备
    /* 新添加设备类后，务必要在主界面的 “选择仪器/设备选项” 中添加标签页，页顺序必须和上面的数字一一对应 */
}

//可选测窑项目初始化
void Global::project_choose_init(){
    IProject::project_map_clear();//清空测窑项目map

    /* 构造"筒体截面测量"项目，添加到project_map */
    IProject* p0 = new IProject("section");
    fake_map<QString, QString> list_dir;
    list_dir.insert("顺时针","1");
    list_dir.insert("逆时针","-1");
    p0->add_para("窑旋向","turnDirection",para_type::COMBOBOX,list_dir,"从低往高看窑的旋向");
    p0->add_para("起始测角(°)","startAngle",para_type::INTSPIN,"从低往高看，仪器在左侧为负角度，在右侧为正角度");
    p0->add_para("筒体直径(mm)","diameter",para_type::FLOATSPIN);

    fake_map<QString, QString> list1;
    list1.insert("筒体","0");
    list1.insert("一档轮带","1");
    list1.insert("二档轮带","2");
    list1.insert("三档轮带","3");
    list1.insert("四档轮带","4");
    p0->add_para("截面位于","sectionObj",para_type::COMBOBOX,list1);
    p0->add_para("截面位置长度","axialDistance",para_type::FLOATSPIN);
//    由于截面长度已经可以管理截面顺序，所以Java Web端不再使用截面号
//    fake_map<QString, QString> list_secNo;
//    list_secNo.insert("截面1","1");
//    list_secNo.insert("截面2","2");
//    ....
//    p0->add_para("截面号(不算轮带)","sectionNo",para_type::COMBOBOX,list_secNo,"只有筒体需要截面号,轮带不管即可");

    p0->add_para("备注","note",para_type::TEXT);
    p0->set_def_device(0);//默认设备与上面的设备tab编号对应，这里表示 筒体截面测量 使用 大激光0：device_laser_big 作为默认设备
    IProject::project_map_add("筒体截面测量",p0);

    /* 构造"椭圆度测量"项目，添加到project_map */
    IProject* p1 = new IProject("ovality");
    fake_map<QString, QString> list_loc;
    list_loc.insert("一档低","1D");
    list_loc.insert("一档高","1U");
    list_loc.insert("二档低","2D");
    list_loc.insert("二档高","2U");
    list_loc.insert("三档低","3D");
    list_loc.insert("三档高","3U");
    list_loc.insert("四档低","4D");
    list_loc.insert("四档高","4U");
    p1->add_para("测量位置","gear",para_type::COMBOBOX,list_loc);

    fake_map<QString, QString> list_point;
    list_point.insert("A点","A");
    list_point.insert("B点","B");
    list_point.insert("C点","C");
    p1->add_para("测量点位","dotType",para_type::COMBOBOX,list_point);
    p1->add_para("窑旋向","turnDirection",para_type::COMBOBOX,list_dir,"从低往高看窑的旋向");
    p1->add_para("起始测角(°)","startAngle",para_type::INTSPIN,"从低往高看，仪器在左侧为负角度，在右侧为正角度");
    p1->add_para("筒体直径(mm)","diameter",para_type::FLOATSPIN);
    p1->add_para("测量距离(mm)","distance",para_type::FLOATSPIN);
    p1->add_para("筒体温度(°)","cyT",para_type::FLOATSPIN);
    p1->add_para("轮带温度(°)","beltT",para_type::FLOATSPIN);
    p1->add_para("轮带间隙(mm)","gap",para_type::FLOATSPIN);
    p1->add_para("备注","note",para_type::TEXT);//项目的额外可用参数
    p1->set_def_device(1);//项目的默认设备
    IProject::project_map_add("椭圆度测量",p1);

    //构造"自定义项目"项目，添加到project_map
    IProject* p3 = new IProject("custom");
    p3->add_para("项目名称","project",para_type::TEXT);
    p3->add_para("测量对象","object",para_type::TEXT);
    p3->add_para("测量位置","location",para_type::TEXT);
    p3->add_para("备注","note",para_type::TEXT);
    p3->set_def_device(0);//使用大激光的突发项目需求居多
    IProject::project_map_add("自定义项目",p3);
}

Global::Global(){}
