#ifndef ANALYTICSLOGWINDOW_H
#define ANALYTICSLOGWINDOW_H

#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class AnalyticsLogWindow : public QPlainTextEdit
{
    Q_OBJECT

public:
    AnalyticsLogWindow(QWidget *parent);
    ~AnalyticsLogWindow();
    void initialiseLogFile();
    void closeLogFile();
    void appendToWindow(const QString& text);
    void appendToLogFile(const QJsonObject& obj);

private:
    QFile m_logFile;
    QString m_fileName;
    bool m_fileOpen;
};

#endif // ANALYTICSLOGWINDOW_H
