#include "IPara.h"
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
IPara::IPara(){
    this->para_name = "def";
    this->para_key = "def";
    this->type = para_type::TEXT;
    this->para_value = "";
    this->para_tip = "";
}
IPara::IPara(QString name, QString key, para_type type){
    this->para_name = name;
    this->para_key = key;
    this->type = type;
    this->para_value = "";
    this->para_tip = "";
}
IPara::IPara(QString name, QString key, para_type type, QString tip){
    this->para_name = name;
    this->para_key = key;
    this->type = type;
    this->para_value = "";
    this->para_tip = tip;
}

QString IPara::get_para_name(){
    return para_name;
}
QString IPara::get_para_key(){
    return para_key;
}
QString IPara::get_para_value(){
    return para_value;
}
/**
 * @brief IPara::insert_item 向当前参数的下拉框数据模型中插入一项key-value，使用这个函数说明当前参数是下拉框类型
 * @param key
 * @param val
 * @return
 */
bool IPara::insert_item(QString key, QString val){
    if(combobox_model.find(key) != combobox_model.end()){
        return false;
    }
    combobox_model.insert(key,val);
    return true;
}

/**
 * @brief IPara::place_widget 将参数可视化为界面控件，放置于指定位置
 * @param layout 放置控件到哪个布局中
 * @param row
 * @param column
 */
void IPara::place_widget(QGridLayout* layout ,int row, int column){
    QLabel* para_label;//参数对应的label标题控件，指针
    QWidget* para_wid;//参数对应的内容控件，指针
    para_label = new QLabel(para_name);
    para_label->setToolTip(para_tip);//设置tooltip提示
    this->disconnect();//删除该变量所有的connect
    if(type == para_type::COMBOBOX){
        QComboBox* para_wid_combo = new QComboBox();
        if(!combobox_model.isEmpty()){
            auto itor = combobox_model.begin();
            while(itor != combobox_model.end()){
                para_wid_combo->addItem(itor->first, itor->second);
                ++itor;
            }
        }
        para_wid = para_wid_combo;
        connect(para_wid_combo, SIGNAL(currentIndexChanged(QString)), this, SLOT(get_combobox_value_slot(QString)));
        //默认值,因为不写这个可能不会触发currentIndexChanged，从而获取不到数据
        this->para_value = para_wid_combo->currentData().value<QString>();
    }
    else if(type == para_type::CHECK){
        QCheckBox* para_wid_check = new QCheckBox();
        para_wid = para_wid_check;
        connect(para_wid_check, SIGNAL(stateChanged(int)), this, SLOT(get_bool_value_slot(int)));
        //默认值,因为不写这个可能不会触发stateChanged，从而获取不到数据
        this->para_value = para_wid_check->isChecked() ? "true" : "false";
    }
    else if(type == para_type::TEXT){
        QLineEdit* para_wid_text = new QLineEdit();
        para_wid = para_wid_text;
        connect(para_wid_text, SIGNAL(textChanged(QString)), this, SLOT(get_text_value_slot(QString)));
        this->para_value = "";//TEXT默认为空
    }
    else if(type == para_type::INTSPIN){
        QSpinBox* para_wid_int = new QSpinBox();
        para_wid_int->setMaximum(10000);
        para_wid_int->setMinimum(-10000);
        para_wid = para_wid_int;
        connect(para_wid_int, SIGNAL(valueChanged(int)), this, SLOT(get_int_value_slot(int)));
        this->para_value = "0";//int默认为0
    } else {
        QDoubleSpinBox* para_wid_float = new QDoubleSpinBox();
        para_wid_float->setDecimals(1);//一位小数
        para_wid_float->setMinimum(0.0);
        para_wid_float->setMaximum(10000.0);
        para_wid = para_wid_float;
        connect(para_wid_float, SIGNAL(valueChanged(double)), this, SLOT(get_float_value_slot(double)));
        this->para_value = "0.0";//float默认为0.0

    }
    layout->addWidget(para_label,row,column);
    layout->addWidget(para_wid,row,column+1);
}

/**
 * @brief IPara::get_combobox_value_slot 得到下拉框参数的值value
 * @param get
 */
void IPara::get_combobox_value_slot(QString get){
    para_value = combobox_model.value(get);
}
/* 下面类似 */
void IPara::get_text_value_slot(QString get){
    para_value = get;
}
void IPara::get_bool_value_slot(int get){
    para_value = (get ? "true" : "false");
}
void IPara::get_int_value_slot(int get){
    para_value = QString::number(get);
}
void IPara::get_float_value_slot(double get){
    para_value = QString::number(get, 'f', 1);
}
