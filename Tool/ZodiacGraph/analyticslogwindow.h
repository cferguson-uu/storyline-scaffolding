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
    void initialiseLogFile();
    void appendMessage(const QString& text);

private:
    QString convertJSONtoSentence(const QString& text);

    QFile m_logFile;
};

#endif // ANALYTICSLOGWINDOW_H
