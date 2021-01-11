#ifndef LOSTNESSGRAPH_H
#define LOSTNESSGRAPH_H

#include <QDialog>
#include <QWidget>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>

class LostnessGraph : public QDialog
{
    Q_OBJECT
public:
    LostnessGraph(QWidget *parent = 0);
    ~LostnessGraph();

    void showWindow(){show();}
    void resetAll();

    void addPoint(qint64 x, float y);
private:
    void initialiseChart();

    QtCharts::QChartView *m_chartView;

    QtCharts::QLineSeries *m_series;

    QtCharts::QValueAxis *m_axisX;
    QtCharts::QValueAxis *m_axisY;
};

#endif // LOSTNESSGRAPH_H
