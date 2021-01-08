#include "lostnessgraph.h"

#include <QScrollArea>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>

LostnessGraph::LostnessGraph(QWidget *parent)
: m_series(new QtCharts::QLineSeries())
, m_chart(new QtCharts::QChart())
, QDialog(parent)
{
    m_series->append(1, 0.2);
    m_series->append(3, 0);
    m_series->append(5, 0.8);
    m_series->append(7, 0.33);
    m_series->append(11, 0.72);

    m_chart->legend()->hide();
    m_chart->addSeries(m_series);
    m_chart->createDefaultAxes();
    m_chart->resize(QSize(400,400));

    QtCharts::QChartView *chartView = new QtCharts::QChartView(m_chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *parentLayout = new QVBoxLayout;
    QScrollArea *scrollArea = new QScrollArea;

    scrollArea->setWidget(chartView);

    parentLayout->addWidget(scrollArea);
    setLayout(parentLayout);

    setWindowTitle(tr("Lostness"));
    resize(QSize(400,400));

}

void LostnessGraph::showWindow()
{
    show();
}

void LostnessGraph::resetAll()
{

}
