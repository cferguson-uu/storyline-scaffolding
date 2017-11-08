#ifndef ANALYTICSHANDLER_H
#define ANALYTICSHANDLER_H

#include "analyticslogwindow.h"
#include "analyticssocket.h"
#include <QObject>

class AnalyticsHandler : public QObject
{
    Q_OBJECT

public:
    AnalyticsHandler(AnalyticsLogWindow *logger, QObject *parent);

signals:
    void unlockNode(QString name);

public slots:
    void connected();
    void disconnected();
    void handleMessage(QString message);
    void connectToServer();

private:
    AnalyticsSocket *m_tcpSocket;
    AnalyticsLogWindow *m_logWindow;
};

#endif // ANALYTICSHANDLER_H
