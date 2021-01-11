#include "lostnessgraph.h"

#include <QBoxLayout>
#include <QScrollArea>

LostnessGraph::LostnessGraph(QWidget *parent)
: m_series(new QtCharts::QLineSeries())
, m_chartView(new QtCharts::QChartView(new QtCharts::QChart()))
, m_axisX(new QtCharts::QValueAxis())
, m_axisY(new QtCharts::QValueAxis())
, QDialog(parent)
{
    //Set up windows and make chart the centre
    QVBoxLayout *parentLayout = new QVBoxLayout;
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidget(m_chartView);
    parentLayout->addWidget(scrollArea);
    setLayout(parentLayout);
    setWindowTitle(tr("Lostness"));
    resize(QSize(400,400));

    //set up chart
    m_chartView->chart()->legend()->hide();
    m_chartView->chart()->addSeries(m_series);
    initialiseChart();
}

LostnessGraph::~LostnessGraph()
{

}

void LostnessGraph::resetAll()
{
    m_series->clear();
    m_axisX->setRange(0, 30);
    m_axisY->setRange(0, 1.44);
    m_chartView->resize(QSize(360,375));
}

void LostnessGraph::initialiseChart()
{
    m_axisX->setRange(0, 30);
    m_axisX->setLabelFormat("%.0f");
    m_axisX->setTickCount(7);
    m_axisX->setTitleText("Time (Seconds)");
    m_chartView->chart()->setAxisX(m_axisX, m_series);

    m_axisY->setRange(0, 1.44);
    m_axisY->setLabelFormat("%.1f");
    m_axisY->setTickCount(10);
    m_axisY->setTitleText("Lostness");
    m_chartView->chart()->setAxisY(m_axisY, m_series);

    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->resize(QSize(360,375));

    m_series->append(0, 0); //needed to display the line graph correctly
}

void LostnessGraph::addPoint(qint64 x, float y)
{
    if(x > 30 && m_chartView->size().width() < (x*12))
    {
         m_axisX->setRange(0, x);
         m_chartView->resize(x*12, m_chartView->size().height());
    }

    m_series->append(x, y);
}
