#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Chart/IChart.h"

//更新数据到折线图表
void MainWindow::update_to_chart(){
    IChart::get_instance()->chart_show_once();
    QTextDocument *document;
    document=ui->data_switch->document();
    for(auto itor=document->begin();itor!=document->end();itor=itor.next()){
        bool success = false;//转double是否成功
        double tmp_double = itor.text().toDouble(&success);
        if(success)
            IChart::get_instance()->chart_add_point_unlimited(tmp_double);
        else {
            IChart::get_instance()->chart_add_point_unlimited(-1);
        }
    }
    IChart::get_instance()->all_display();
}

//销毁折线图相关 SLOT
void MainWindow::destroy_chart(){
    delete IChart::get_instance();
}
