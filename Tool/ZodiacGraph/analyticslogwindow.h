#ifndef ANALYTICSLOGWINDOW_H
#define ANALYTICSLOGWINDOW_H

#include <QPlainTextEdit>
#include <QScrollBar>

class AnalyticsLogWindow : public QPlainTextEdit
{
    Q_OBJECT

public:
    AnalyticsLogWindow(QWidget *parent);
    void initialiseLogFile();
    void appendMessage(const QString& text);

private:
    QFile m_logFile;
};

#endif // ANALYTICSLOGWINDOW_H
