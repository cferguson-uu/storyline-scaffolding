#ifndef ANALYTICSHANDLER_H
#define ANALYTICSHANDLER_H

#include "analyticslogwindow.h"
#include "analyticssocket.h"
#include <QObject>
#include "zodiacgraph/node.h"
#include "nodeproperties.h"

class AnalyticsHandler : public QObject
{
    Q_OBJECT

public:
    AnalyticsHandler(AnalyticsLogWindow *logger, QAction *connectAction, QAction *disconnectAction, QObject *parent);

signals:
    void unlockNode(QString name);
    void closeNodeProperties();

public slots:
    void connected();
    void disconnected();
    void handleMessage(QString message);
    void connectToServer();

private:
    AnalyticsSocket *m_tcpSocket;
    AnalyticsLogWindow *m_logWindow;

    QAction *m_connectAction;
    QAction *m_disconnectAction;
};

#endif // ANALYTICSHANDLER_H
