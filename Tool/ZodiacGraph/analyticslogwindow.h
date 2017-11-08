#ifndef ANALYTICSLOGWINDOW_H
#define ANALYTICSLOGWINDOW_H

#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
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
    void appendToLogFile(const QString& text);

private:
    QFile m_logFile;
    bool m_fileOpen;
};

#endif // ANALYTICSLOGWINDOW_H
