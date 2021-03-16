#ifndef ICHART_H
#define ICHART_H

#include <QtCharts>
#include <QTimer>

class IChart : QObject{
    Q_OBJECT
private:
    static IChart* instance;
    static QLineSeries *series;//数据series
    static QScatterSeries *series_err;//错误series,显示为散点
    static QChart *chart;
    static QChartView *view;
    static int count;
    static double max_val,min_val;
    static QVector<QPointF> buf;
    static QVector<QPointF> buf_err;
    static QTimer* timer;
    static long long total_count;//总点数
    static long long success_count;//成功点数(例如：数据为E18就是不成功的点，数据为123.45就是成功的点

public:
    virtual ~IChart();
    static IChart* get_instance();
    void chart_init();
    void chart_show();//开始动态生成示意图，会开启定时器实时更新数据
    void chart_show_once();//只生成一次，不开启定时器Tick
    void chart_close();
    void chart_add_point(double);//有限制添加点的函数，用于动态展示，限制点个数即为BUF_SIZE,见cpp文件宏定义
    void chart_add_point_unlimited(double);//无限制添加点的函数，用于单次全部展示
    void task_display();//提供多线程使用的展示数据接口，用于动态展示数据，限定了一次展示的数据量
    void all_display();//用于展示全部数据，不限制数据量，全部展示
public slots:
    void timer_tick();
};

#endif
