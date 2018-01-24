#ifndef ANALYTICSHANDLER_H
#define ANALYTICSHANDLER_H

#include "analyticslogwindow.h"
#include "analyticssocket.h"
#include <QObject>
#include "zodiacgraph/node.h"
#include "nodeproperties.h"
#include "curatoranalyticseditor.h"
#include "analyticsproperties.h"

class AnalyticsHandler : public QObject
{
    Q_OBJECT

public:
    AnalyticsHandler(AnalyticsLogWindow *logger, QAction *connectAction, QAction *disconnectAction, QAction *editLostnessAction, QObject *parent);

    void setAnalyticsProperties(AnalyticsProperties *properties);
    void startAnalyticsMode();
    void stopAnalyticsMode();

signals:
    void unlockNode(QString name);
    void lockAllNodes();
    void resetNodes();
    void closeNodeProperties();
    void checkForGraphs();

public slots:
    void connected();
    void disconnected();
    void handleMessage(QString message);
    void connectToServer();
    void showCuratorLabels();

private:
    ///
    /// \brief The layout to be used by CuratorRows to place their widgets.
    ///
    /// \param [in] json object to parse
    void handleObject(QJsonObject jsonObj);

    void handleTextOutput(QJsonObject &jsonObj);

    AnalyticsSocket *m_tcpSocket;
    AnalyticsLogWindow *m_logWindow;
    CuratorAnalyticsEditor *m_curatorAnalyticsEditor;

    QAction *m_connectAction;
    QAction *m_disconnectAction;
    QAction *m_editLostnessAction;

    QList<QString> m_activeTasks;

    AnalyticsProperties *m_pProperties;

    bool m_analyticsEnabled;
};

#endif // ANALYTICSHANDLER_H
