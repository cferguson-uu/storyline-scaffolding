#include "analyticslogwindow.h"
#include <QDir>

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
    m_fileName = "logs/" + current.toString("yyyy.MM.dd_hh-mm-ss-t_logFile") + ".json";

    m_logFile.setFileName(m_fileName);
}

void AnalyticsLogWindow::overwriteLogFile(QString fileName)
{
    m_fileName = fileName;
    m_logFile.setFileName(m_fileName);

    m_logFile.open(QIODevice::ReadWrite);   //open the file and clear it
    m_logFile.resize(0);
    m_logFile.close();
}

void AnalyticsLogWindow::closeLogFile()
{
    m_logFile.close();
    m_fileOpen = false;
}

void AnalyticsLogWindow::appendToWindow(const QString& text)
{
    this->appendPlainText(text); // Adds the message to the widget
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom
}

void AnalyticsLogWindow::appendToLogFile(const QJsonObject& obj)
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArray;

    m_logFile.open(QIODevice::ReadWrite);   //open the file

    if (m_logFile.size() > 0)   //if it's not empty then append the latest message to the array
    {
        QString settings;

        QTextStream in(&m_logFile);     //read in file line by line
           while (!in.atEnd())
               settings += in.readLine();

        jsonDoc = QJsonDocument::fromJson(settings.toUtf8());

        if(jsonDoc.isArray())
            jsonArray = jsonDoc.array();
    }

    jsonArray.append(obj);      //append the object
    jsonDoc.setArray(jsonArray);

    m_logFile.resize(0);    //delete all data in the file

    QTextStream outputStream(&m_logFile);   //write updated (or new) array then close the file
    outputStream << jsonDoc.toJson();

    m_logFile.close();
}
