#ifndef ANALYTICSHANDLER_H
#define ANALYTICSHANDLER_H

#include "analyticslogwindow.h"
#include "analyticssocket.h"
#include <QObject>
#include "zodiacgraph/node.h"
#include "nodeproperties.h"
#include "curatoranalyticseditor.h"
#include "analyticsproperties.h"
#include "lostnessgraph.h"

class AnalyticsHandler : public QObject
{
    Q_OBJECT

public:
    AnalyticsHandler(AnalyticsLogWindow *logger, QAction *connectAction, QAction *disconnectAction, QAction *editLostnessAction, QAction *loadAction, QAction *clearAction, QAction *lostnessGraphAction, QObject *parent);

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
    void connectToServer();
    void showCuratorLabels();
    void readMessageFromServer(QString message);

private:
    void handleMessage(QString message, bool updateValues = true, bool loadLogFile = false);
    void handleObject(QJsonObject jsonObj, bool updateValues, bool loadLogFile);

    void handleTextOutput(QJsonObject &jsonObj, bool updateValues, bool loadLogFile = false);
    void loadAnalyticsLog();

    void clearAll();

    AnalyticsSocket *m_tcpSocket;
    AnalyticsLogWindow *m_logWindow;
    CuratorAnalyticsEditor *m_curatorAnalyticsEditor;
    LostnessGraph *m_lostnessGraphDialog;

    QAction *m_connectAction;
    QAction *m_disconnectAction;
    QAction *m_editLostnessAction;
    QAction *m_loadLogFileAction;
    QAction *m_clearAnalyticsAction;
    QAction *m_lostnessGraphAction;

    QList<QString> m_activeTasks;

    AnalyticsProperties *m_pProperties;

    bool m_analyticsEnabled;

    QSet<QString> m_lostness_actions;

    QDateTime m_startTime;
};

#endif // ANALYTICSHANDLER_H
