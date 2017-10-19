#include "analyticslogwindow.h"
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

AnalyticsLogWindow::AnalyticsLogWindow(QWidget *parent)
    : QPlainTextEdit(parent)
{
    QPlainTextEdit::setReadOnly(true);
}

void AnalyticsLogWindow::initialiseLogFile()
{
    QDateTime current = QDateTime::currentDateTime().toUTC();

    m_logFile.setFileName(current.toString("yyyy.MM.dd_hh-mm-ss-t_logFile") + ".txt");
    if (!m_logFile.open(QIODevice::ReadWrite))
        qDebug() << "fail";
}

void AnalyticsLogWindow::appendMessage(const QString& text/*, bool addToLogFile*/)
{
    this->appendPlainText(text); // Adds the message to the widget
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom

    //m_logFile.write(text.toLocal8Bit().constData()); // Logs to file

    QTextStream outputStream(&m_logFile);
    outputStream << text << "\n";
}
