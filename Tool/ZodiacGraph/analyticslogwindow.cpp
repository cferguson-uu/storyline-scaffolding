#include "analyticslogwindow.h"

AnalyticsLogWindow::AnalyticsLogWindow(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_fileOpen(false)
{
    QPlainTextEdit::setReadOnly(true);
}

AnalyticsLogWindow::~AnalyticsLogWindow()
{
    if(m_fileOpen)
        closeLogFile();
}

void AnalyticsLogWindow::initialiseLogFile()
{
    QDateTime current = QDateTime::currentDateTime().toUTC();

    m_logFile.setFileName(current.toString("yyyy.MM.dd_hh-mm-ss-t_logFile") + ".txt");

    if (!m_logFile.open(QIODevice::ReadWrite))
        qDebug() << "fail";
    else
        m_fileOpen = true;
}

void AnalyticsLogWindow::closeLogFile()
{
    m_logFile.write("\n}");
    m_logFile.close();

    m_fileOpen = false;
}

void AnalyticsLogWindow::appendToWindow(const QString& text)
{
    this->appendPlainText(text); // Adds the message to the widget
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom
}

void AnalyticsLogWindow::appendToLogFile(const QString& text)
{
    if (m_logFile.size() == 0)
        m_logFile.write(text.toLocal8Bit().constData()); // Logs to file
    else
    {
        m_logFile.write(",\n");
        m_logFile.write(text.toLocal8Bit().constData()); // Logs to file
    }

    QTextStream outputStream(&m_logFile);
    outputStream << text << "\n";
}
