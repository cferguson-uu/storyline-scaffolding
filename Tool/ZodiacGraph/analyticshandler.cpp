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
static const QString kName_Unlock = "unlock";
static const QString kName_Fail = "fail";
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
    , m_analyticsEnabled(false)
    , QObject(parent)
{
    connect(m_connectAction, &QAction::triggered, [=]{connectToServer();});
    connect(m_disconnectAction, &QAction::triggered, [=]{m_tcpSocket->disconnectFromServer();});
    connect(m_editLostnessAction, &QAction::triggered, [=]{m_curatorAnalyticsEditor->showWindow();});

    m_disconnectAction->setEnabled(false);
    m_connectAction->setEnabled(false);

    connect(m_tcpSocket, SIGNAL(connectedCallback()), this, SLOT(connected()));
    connect(m_tcpSocket, SIGNAL(disconnectedCallback()), this, SLOT(disconnected()));
    connect(m_tcpSocket, SIGNAL(readMessage(QString)), this, SLOT(handleMessage(QString)));


    connect(m_curatorAnalyticsEditor, SIGNAL(finished(int)), this, SLOT(showCuratorLabels()));
}

void AnalyticsHandler::setAnalyticsProperties(AnalyticsProperties *properties)
{
    m_pProperties = properties;
}

void AnalyticsHandler::connectToServer()
{
    m_tcpSocket->SetUpSocket();
}

void AnalyticsHandler::connected()
{
    m_logWindow->initialiseLogFile();
    m_connectAction->setEnabled(false);
    m_disconnectAction->setEnabled(true);
}

void AnalyticsHandler::disconnected()
{
    m_logWindow->closeLogFile();
    m_connectAction->setEnabled(true);
    m_disconnectAction->setEnabled(false);
}

void AnalyticsHandler::startAnalyticsMode()
{
    checkForGraphs();

    if(m_curatorAnalyticsEditor->checkIfAnalyticsLoaded()) //wait for this to be completed before running the next function
        if(m_pProperties)   //show curator labels
            m_pProperties->StartAnalyticsMode(m_curatorAnalyticsEditor->getCuratorLabels());


    zodiac::Node::setAnalyticsMode(true);
    NodeProperties::setAnalyticsMode(true);
    closeNodeProperties();
    lockAllNodes(); //show all nodes as locked (haven't been unlocked yet)

    m_analyticsEnabled = true;
}

void AnalyticsHandler::stopAnalyticsMode()
{
    zodiac::Node::setAnalyticsMode(false);
    NodeProperties::setAnalyticsMode(false);
    closeNodeProperties();
    resetNodes();   //return nodes to their edit state

    if(m_pProperties)   //hide curator labels
        m_pProperties->StopAnalyticsMode();

    m_analyticsEnabled = false;
}

void AnalyticsHandler::showCuratorLabels()
{
    if(!m_analyticsEnabled)
        return; //callback from curator label editor if called when analytics is starting

    if(m_pProperties)   //show curator labels
        m_pProperties->StartAnalyticsMode(m_curatorAnalyticsEditor->getCuratorLabels());
}

void AnalyticsHandler::handleMessage(QString message)
{
    //check if the JSON data is correct
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());

    if(jsonDoc.isNull() || jsonDoc.isEmpty())
    {
        qDebug() << "Problem with JSON string";
        return;
    }

    if(jsonDoc.isArray())
    {
        QJsonArray jsonArray = jsonDoc.array();
        foreach(QJsonValue jsonVal, jsonArray)
        {
            if(jsonVal.isObject())
                handleObject(jsonVal.toObject());
        }
    }
    else
        if(jsonDoc.isObject())
        {
            handleObject(jsonDoc.object());
        }
        else
        {
            qDebug() << "Problem with JSON string";
        }
}

void AnalyticsHandler::handleObject(QJsonObject jsonObj)
{
    if(!jsonObj.contains(kName_Actor) || !jsonObj.contains(kName_Verb) || !jsonObj.contains(kName_Object) || !jsonObj.contains(kName_Timestamp))
    {
        qDebug() << "Problem with JSON object";
        return;
    }

    if(jsonObj[kName_Verb].toString() == kName_Started) //add new task to active list and set as started in properties
    {
        m_activeTasks.push_back(jsonObj[kName_Object].toString());
        m_pProperties->setCuratorLabelStarted(jsonObj[kName_Object].toString(), true);
    }

    if(jsonObj[kName_Verb].toString() == kName_Completed)   //task completed, get lostness value, remove from the list and update properties
    {
        float lostness = m_curatorAnalyticsEditor->getLostnessValue(jsonObj[kName_Object].toString());

        if(lostness >= 0)
            jsonObj[kName_Lostness] = lostness;

        m_activeTasks.removeAll(jsonObj[kName_Object].toString());
        m_pProperties->updateLostnessOfCuratorLabel(jsonObj[kName_Object].toString(), m_curatorAnalyticsEditor->getLostnessValue(jsonObj[kName_Object].toString()));
    }

    foreach (QString task, m_activeTasks)   //update lostness and sequence similarity values and check progress
    {
        m_curatorAnalyticsEditor->nodeVisited(task, jsonObj);

        if(m_pProperties->getCuratorLabelStarted(task)) //can't do anything unless task is started
        {
            if(jsonObj[kName_Verb].toString() == kName_Attempted && jsonObj[kName_Result].toString() == kName_Unlock)   //if node unlocked, update curator label progress
                m_pProperties->updateProgressOfCuratorLabel(task, jsonObj[kName_Object].toString());

            m_pProperties->updateSimilarityOfCuratorLabel(task, m_curatorAnalyticsEditor->getSimilarityValue(task));
            m_pProperties->updateLostnessOfCuratorLabel(task, m_curatorAnalyticsEditor->getLostnessValue(task));
        }
    }

    if(jsonObj[kName_Verb].toString() == kName_Attempted && jsonObj[kName_Result].toString() == kName_Unlock)   //light up node in scene to show unlocked
    {
        unlockNode(jsonObj[kName_Object].toString());
    }

    handleTextOutput(jsonObj);
}

void AnalyticsHandler::handleTextOutput(QJsonObject &jsonObj)
{
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
