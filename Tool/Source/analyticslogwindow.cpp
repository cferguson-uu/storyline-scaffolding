#include "analyticslogwindow.h"
#include <QDir>

AnalyticsLogWindow::AnalyticsLogWindow(QWidget *parent)
    : QPlainTextEdit(parent)
{
    QPlainTextEdit::setReadOnly(true);
}

AnalyticsLogWindow::~AnalyticsLogWindow()
{
    if(m_fileInitialised && !m_jsonEvents.empty())  //export to file if cross button is pressed without disconnecting
    {
        exportToFile();
    }
}

void AnalyticsLogWindow::initialiseLogFile(QString fileName)
{
    if(fileName == "")
    {
        if(!QDir("logs").exists())
            QDir().mkdir("logs");

        QDateTime current = QDateTime::currentDateTime().toUTC();
        m_fileName = "logs/" + current.toString("yyyy.MM.dd_hh-mm-ss-t_logFile") + ".json";
    }
    else
        m_fileName = fileName;

    m_logFile.setFileName(m_fileName);
    m_fileInitialised = true;
}

void AnalyticsLogWindow::appendToWindow(const QString& text)
{
    this->appendPlainText(text); // Adds the message to the widget
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom
}

void AnalyticsLogWindow::appendToLogFile(const QJsonObject& obj)
{
    m_jsonEvents.append(obj);
}

void AnalyticsLogWindow::exportToFile()
{
    //called at the end to save data to the file
    QJsonDocument jsonDoc;

    m_logFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);   //open the file and set to overwrite

    jsonDoc.setArray(m_jsonEvents);

    QTextStream outputStream(&m_logFile);   //write updated (or new) array then close the file
    outputStream << jsonDoc.toJson();

    m_logFile.close();  //close log file and clear json array
    m_jsonEvents = QJsonArray();
    m_fileInitialised = false;
}
