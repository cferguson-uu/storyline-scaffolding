#include "analyticslogwindow.h"

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
    this->appendPlainText(convertJSONtoSentence(text)); // Adds the message to the widget
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom

    m_logFile.write(text.toLocal8Bit().constData()); // Logs to file

    QTextStream outputStream(&m_logFile);
    outputStream << text << "\n";
}

QString AnalyticsLogWindow::convertJSONtoSentence(const QString& text)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(text.toUtf8());

    if(jsonDoc.isNull() || !jsonDoc.isObject() || jsonDoc.isEmpty())    //check if json string is properly formatted
    {
        qDebug() << "Problem with JSON string";
        return "";
    }
    else
    {
        QJsonObject jsonObj = jsonDoc.object();
        if(!jsonObj.contains("actor") || !jsonObj.contains("verb") || !jsonObj.contains("object") || !jsonObj.contains("timestamp"))
        {
            qDebug() << "Problem with JSON string";
            return "";
        }

        return jsonObj["actor"].toString() + " " + jsonObj["verb"].toString() + " " + jsonObj["object"].toString() + " at "
                + QDateTime::fromString(jsonObj["timestamp"].toString(), Qt::ISODate).toString("MMM dd, yyyy hh:mm:ss t");
    }
}
