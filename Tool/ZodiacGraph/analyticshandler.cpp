#include "analyticshandler.h"

//json value names
static const QString kName_Actor = "actor";
static const QString kName_Verb = "verb";
static const QString kName_Object = "object";
static const QString kName_Result = "result";
static const QString kName_WithResult = " with result ";
static const QString kName_WithLostness = " with lostness value: ";
static const QString kName_At = " at ";
static const QString kName_Timestamp = "timestamp";
static const QString kName_Lostness = "lostness";

//verbs
static const QString kName_Attempted = "attempted";
static const QString kName_Unlocked = "unlocked";
static const QString kName_Started = "started";
static const QString kName_Completed = "completed";
static const QString kName_JumpedTo = "jumped to";
static const QString kName_PickedUp = "picked up";
static const QString kName_Examined = "examined";

AnalyticsHandler::AnalyticsHandler(AnalyticsLogWindow *logger, QAction *connectAction, QAction *disconnectAction, QAction *editLostnessAction, QObject *parent)
    : m_curatorAnalyticsEditor(new CuratorAnalyticsEditor(qobject_cast<QWidget*>(parent)))
    , m_tcpSocket(new AnalyticsSocket(qobject_cast<QWidget*>(parent)))
    , m_logWindow(logger)
    , m_connectAction(connectAction)
    , m_disconnectAction(disconnectAction)
    , m_editLostnessAction(editLostnessAction)
    , QObject(parent)
{
    connect(m_connectAction, &QAction::triggered, [=]{connectToServer();});
    connect(m_disconnectAction, &QAction::triggered, [=]{m_tcpSocket->disconnectFromServer();});
    connect(m_editLostnessAction, &QAction::triggered, [=]{m_curatorAnalyticsEditor->showWindow();});

    m_disconnectAction->setEnabled(false);

    connect(m_tcpSocket, SIGNAL(connectedCallback()), this, SLOT(connected()));
    connect(m_tcpSocket, SIGNAL(disconnectedCallback()), this, SLOT(disconnected()));
    connect(m_tcpSocket, SIGNAL(readMessage(QString)), this, SLOT(handleMessage(QString)));
}

void AnalyticsHandler::setAnalyticsProperties(AnalyticsProperties *properties)
{
    m_pProperties = properties;
}

void AnalyticsHandler::connectToServer()
{
    checkForGraphs();
    if(m_curatorAnalyticsEditor->checkIfAnalyticsLoaded())  //don't show the connect dialog if opening analytics dialog
        m_tcpSocket->SetUpSocket();
}

void AnalyticsHandler::connected()
{
    m_logWindow->initialiseLogFile();
    zodiac::Node::setAnalyticsMode(true);
    NodeProperties::setAnalyticsMode(true);
    closeNodeProperties();
    if(m_pProperties)
        m_pProperties->StartAnalyticsMode(m_curatorAnalyticsEditor->getCuratorLabels());

    m_connectAction->setEnabled(false);
    m_disconnectAction->setEnabled(true);
}

void AnalyticsHandler::disconnected()
{
    m_logWindow->closeLogFile();
    zodiac::Node::setAnalyticsMode(false);
    NodeProperties::setAnalyticsMode(false);
    closeNodeProperties();

    m_connectAction->setEnabled(true);
    m_disconnectAction->setEnabled(false);
}

void AnalyticsHandler::handleMessage(QString message)
{
    //check if the JSON data is correct
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());

    if(jsonDoc.isNull() || !jsonDoc.isObject() || jsonDoc.isEmpty())    //check if json string is properly formatted
    {
        qDebug() << "Problem with JSON string";
        return;
    }
    else
    {
        QJsonObject jsonObj = jsonDoc.object();
        if(!jsonObj.contains(kName_Actor) || !jsonObj.contains(kName_Verb) || !jsonObj.contains(kName_Object) || !jsonObj.contains(kName_Timestamp))
        {
            qDebug() << "Problem with JSON string";
            return;
        }

        if(jsonObj[kName_Verb].toString() == kName_Started) //add new task to active list and set as started in properties
        {
            m_activeTasks.push_back(jsonObj[kName_Object].toString());
            m_pProperties->setCuratorLabelStarted(jsonObj[kName_Object].toString(), true);
        }

        if(jsonObj[kName_Verb].toString() == kName_Completed)   //task completed, get lostness value, remove from the list and update properties
        {
            jsonObj[kName_Lostness] = m_curatorAnalyticsEditor->getLostnessValue(jsonObj[kName_Object].toString());
            m_activeTasks.removeAll(jsonObj[kName_Object].toString());    
            m_pProperties->updateLostnessOfCuratorLabel(jsonObj[kName_Object].toString(), m_curatorAnalyticsEditor->getLostnessValue(jsonObj[kName_Object].toString()));
        }

        foreach (QString task, m_activeTasks)   //update lostness and sequence similarity values and check progress
        {
            m_curatorAnalyticsEditor->nodeVisited(task, jsonObj);

            if(m_pProperties->getCuratorLabelStarted(task)) //can't do anything unless task is started
            {
                m_pProperties->updateProgressOfCuratorLabel(task, jsonObj[kName_Object].toString());
                m_pProperties->updateSimilarityOfCuratorLabel(task, m_curatorAnalyticsEditor->getSimilarityValue(task));
            }
        }

        //formulate human-readable string for log window
        QString sentence = jsonObj[kName_Actor].toString() + " " + jsonObj[kName_Verb].toString() + " " + jsonObj[kName_Object].toString();

        if(jsonObj.contains(kName_Result))
        {
            sentence += kName_WithResult + jsonObj[kName_Result].toString();    //append result to string
        }

        if(jsonObj.contains(kName_Lostness))
        {
            //append lostness to string
            sentence += kName_WithLostness;
            sentence += QString::number(jsonObj[kName_Lostness].toDouble());
        }

        sentence += kName_At + QDateTime::fromString(jsonObj[kName_Timestamp].toString(), Qt::ISODate).toString("MMM dd, yyyy hh:mm:ss t");

        //output string to window and full JSON message to file
        m_logWindow->appendToWindow(sentence);
        m_logWindow->appendToLogFile(jsonObj);
    }
}
