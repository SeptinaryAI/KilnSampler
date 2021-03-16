#ifndef IPARA_H
#define IPARA_H
#include <QString>
#include <QMap>
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include "fake_map.h"

//IPara参数类型 ： 文本输入、 勾选项、 下拉框
enum para_type{
    TEXT,
    CHECK,
    COMBOBOX,
    INTSPIN,
    FLOATSPIN
};

//不同的测窑项目可能会存在一些除了公共参数以外的其他参数，这个类就记录了可能出现的额外参数情况
class IPara : public QObject
{
    Q_OBJECT
private:
    QString para_name;//参数名
    para_type type;//参数类型
    QString para_value;
    QString para_key;
    QString para_tip;
    //if combobox
    fake_map<QString, QString> combobox_model;//下拉框的候选项
    QLabel* para_label;//参数对应的label标题控件，指针
    QWidget* para_wid;//参数对应的内容控件，指针
public:
    explicit IPara();
    explicit IPara(QString name, QString key, para_type type);//构造一个IPara，传入 显示用名称、 文件参数用名称、 IPara类型
    explicit IPara(QString name, QString key, para_type type, QString tip);//构造一个IPara，传入 显示用名称、 文件参数用名称、 IPara类型、说明文字等
    virtual ~IPara(){}
    bool insert_item(QString, QString);
    void place_widget(QGridLayout* ,int, int);//根据参数类型和内容，放置控件
    QString get_para_name();//参数名
    QString get_para_value();//参数值
    QString get_para_key();//参数的key标识，相当于ID

public slots:
    void get_combobox_value_slot(QString get);
    void get_text_value_slot(QString get);
    void get_bool_value_slot(int get);
    void get_int_value_slot(int get);
    void get_float_value_slot(double get);
};

#endif // IPARA_H
