#include "Chart/IChart.h"
#include "QtCharts"
#include <float.h>
#include <limits.h>
#include "Thread/chart_task.h"
QT_CHARTS_USE_NAMESPACE

#define BUF_SIZE 3000

IChart* IChart::instance = new IChart();
QLineSeries* IChart::series = nullptr;
QScatterSeries* IChart::series_err = nullptr;//错误series,显示为散点
QChart* IChart::chart = nullptr;
QChartView* IChart::view = nullptr;
double IChart::max_val = DBL_MIN;
double IChart::min_val = DBL_MAX;
long long IChart::total_count = 0;
long long IChart::success_count = 0;
QVector<QPointF> IChart::buf = QVector<QPointF>();//有效数据buf
QVector<QPointF> IChart::buf_err = QVector<QPointF>();//错误数据buf
QTimer* IChart::timer = new QTimer();

IChart* IChart::get_instance(){
    return instance;
}

IChart::~IChart(){
    delete series;
    delete chart;
    delete view;
}
/**
 * @brief IChart::chart_show 图标持续触发更新显示
 */
void IChart::chart_show(){
    if(chart == nullptr)
        chart_init();
    max_val = DBL_MIN;
    min_val = DBL_MAX;
    total_count = 0;
    success_count = 0;
    buf.clear();
    buf_err.clear();
    series->clear();
    view->show();
    chart->show();
    timer->start(50);//定时器50ms触发一次tick，更新折线图
}
/**
 * @brief IChart::chart_show_once 图标触发一次更新显示
 */
void IChart::chart_show_once(){
    if(chart == nullptr)
        chart_init();
    max_val = DBL_MIN;
    min_val = DBL_MAX;
    total_count = 0;
    success_count = 0;
    buf.clear();
    buf_err.clear();
    series->clear();
    view->show();
    chart->show();
    timer->stop();//定时器不启动
}
/**
 * @brief IChart::chart_close 关闭图表
 */
void IChart::chart_close(){
    if(chart == nullptr)
        chart_init();
    max_val = DBL_MIN;
    min_val = DBL_MAX;
    total_count = 0;
    success_count = 0;
    buf.clear();
    buf_err.clear();
    view->close();
    series->clear();
    chart->close();
    timer->stop();
}
/**
 * @brief IChart::chart_add_point 有限制添加点的函数，用于动态展示，限制点个数即为BUF_SIZE
 * @param p
 */
void IChart::chart_add_point(double p){
    if(chart == nullptr)
        chart_init();
    while(buf.size() >=BUF_SIZE)
        buf.pop_front();
    //负数表示没有测到的点，要根据前后关系确定y坐标
    if(p < 0){
        buf_err.append(QPointF(total_count++,min_val));//加入列表
        return;
    }
    buf.append(QPointF(total_count++,p));//加入列表
    ++success_count;
    if(p > max_val)
        max_val = p;
    if(p < min_val)
        min_val = p;
}
/**
 * @brief IChart::chart_add_point_unlimited 无限制添加点的函数，一般用于一次性展示所有数据
 * @param p
 */
void IChart::chart_add_point_unlimited(double p){
    if(chart == nullptr)
        chart_init();
    if(p < 0){
        buf_err.append(QPointF(total_count++,min_val));//加入列表
        return;
    }
    buf.append(QPointF(total_count++,p));//加入列表
    ++success_count;
    if(p > max_val)
        max_val = p;
    if(p < min_val)
        min_val = p;
}

static int twinkle_flag = 0;

/**
 * @brief IChart::task_display 周期更新的图表展示函数
 */
void IChart::task_display(){
    series->replace(buf);
    for(auto itor = buf_err.begin() ; itor != buf_err.end(); ++itor){
        itor->setY(min_val-2);
    }
    series_err->replace(buf_err);
    chart->axisX()->setRange(total_count - BUF_SIZE > 0 ? total_count - BUF_SIZE : 0,total_count);//限制数据量为BUF_SIZE,用宏来限定
    chart->axisY()->setRange(min_val-2, max_val+2);
    QString title_str = "实时采集数据 - 正常采集的点数 " + QString::number(success_count) + " / " + QString::number(total_count);
    chart->setTitle(title_str);// 设置图表标题
    if(twinkle_flag++ > 3)
        series_err->show();
    else
        series_err->hide();
    if(twinkle_flag > 15)
        twinkle_flag = 0;
}
/**
 * @brief IChart::all_display 一次性更新的图表展示函数
 */
void IChart::all_display(){
    series->replace(buf);
    for(auto itor = buf_err.begin() ; itor != buf_err.end(); ++itor){
        itor->setY(min_val-2);
    }
    series_err->replace(buf_err);
    chart->axisX()->setRange(0,total_count);//限制数据量为BUF_SIZE,用宏来限定
    chart->axisY()->setRange(min_val-2, max_val+2);
    long long total = buf.size() + buf_err.size();
    long long success = buf.size();
    QString title_str = "打开数据文件 - 正常的点数 " + QString::number(success) + " / " + QString::number(total);
    chart->setTitle(title_str);// 设置图表标题
    series_err->show();
}

void IChart::timer_tick(){
    chart_task* task = new chart_task();
    QThreadPool::globalInstance()->start(task);
}

static bool inited = false;
void IChart::chart_init(){
    if(inited)
        return;
    inited = true;//表示只能初始化一次
    series = new QLineSeries();
    series_err = new QScatterSeries();
    chart = new QChart();
    // 将图例隐藏
    chart->legend()->hide();
    chart->addSeries(series);// 关联series
    chart->addSeries(series_err);
    series_err->setUseOpenGL(true);
    series_err->setColor(Qt::red);//红色标识
    series_err->setMarkerShape(QScatterSeries::MarkerShapeCircle);//圆形的点
    series_err->setMarkerSize(3);//大小12
    series->setUseOpenGL(true);// 开启OpenGL
    series->setColor(Qt::blue);//红色标识
    chart->createDefaultAxes();// 创建默认的坐标系
    chart->setTitle("实时数据");// 设置图表标题
    view = new QChartView(chart);
    // 开启抗锯齿，让显示效果更好
    view->setRenderHint(QPainter::Antialiasing);
    view->resize(700,400);
    //WA_QuitOnClose为true的所有窗口关闭后，程序退出。只让MainWindow的WA_QuitOnClose是true,其他窗口都为false
    view->setAttribute(Qt::WA_QuitOnClose, false);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_tick()));
};
