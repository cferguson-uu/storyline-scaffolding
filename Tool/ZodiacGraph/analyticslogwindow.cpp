#include "analyticslogwindow.h"
#include <QTextStream>

AnalyticsLogWindow::AnalyticsLogWindow(QWidget *parent)
    : QPlainTextEdit(parent)
{
    QPlainTextEdit::setReadOnly(true);
}

void AnalyticsLogWindow::appendMessage(const QString& text)
{
    this->appendPlainText(text); // Adds the message to the widget
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom

    //m_logFile.write(text.toLocal8Bit().constData()); // Logs to file

    QTextStream outputStream(&m_logFile);
    outputStream << text << "\n";
}
