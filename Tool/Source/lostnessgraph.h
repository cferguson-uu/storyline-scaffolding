#ifndef LOSTNESSGRAPH_H
#define LOSTNESSGRAPH_H

#include <QDialog>
#include <QWidget>
#include <QGridLayout>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>

class LostnessGraph : public QDialog
{
    Q_OBJECT
public:
    LostnessGraph(QWidget *parent = 0);

    void showWindow();
    void resetAll();
private:
    QtCharts::QLineSeries *m_series;
    QtCharts::QChart *m_chart;
};

#endif // LOSTNESSGRAPH_H
