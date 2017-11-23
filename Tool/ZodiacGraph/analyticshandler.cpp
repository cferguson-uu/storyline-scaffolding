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

AnalyticsHandler::AnalyticsHandler(AnalyticsLogWindow *logger, QAction *connectAction, QAction *disconnectAction, QAction *editLostnessAction, QObject *parent)
    : m_lostnessEditor(new LostnessEditor(qobject_cast<QWidget*>(parent)))
    , m_tcpSocket(new AnalyticsSocket(qobject_cast<QWidget*>(parent)))
    , m_logWindow(logger)
    , m_connectAction(connectAction)
    , m_disconnectAction(disconnectAction)
    , m_editLostnessAction(editLostnessAction)
    , QObject(parent)
{
    connect(m_connectAction, &QAction::triggered, [=]{connectToServer();});
    connect(m_disconnectAction, &QAction::triggered, [=]{m_tcpSocket->disconnectFromServer();});
    connect(m_editLostnessAction, &QAction::triggered, [=]{m_lostnessEditor->showWindow();});

    m_disconnectAction->setEnabled(false);

    connect(m_tcpSocket, SIGNAL(connectedCallback()), this, SLOT(connected()));
    connect(m_tcpSocket, SIGNAL(disconnectedCallback()), this, SLOT(disconnected()));
    connect(m_tcpSocket, SIGNAL(readMessage(QString)), this, SLOT(handleMessage(QString)));
}

void AnalyticsHandler::connectToServer()
{
    m_tcpSocket->SetUpSocket();
}

void AnalyticsHandler::connected()
{
    m_logWindow->initialiseLogFile();
    zodiac::Node::setAnalyticsMode(true);
    NodeProperties::setAnalyticsMode(true);
    closeNodeProperties();

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

        if(jsonObj[kName_Verb].toString() == kName_Started) //add new task to active list
            m_activeTasks.push_back(jsonObj[kName_Object].toString());

        if(jsonObj[kName_Verb].toString() == kName_Completed)   //task completed, get lostness value and remove from the list
        {
            jsonObj[kName_Lostness] = m_lostnessEditor->getLostnessValue(jsonObj[kName_Object].toString());
            m_activeTasks.removeAll(jsonObj[kName_Object].toString());
        }

        if(jsonObj[kName_Verb].toString() == kName_JumpedTo && !m_activeTasks.empty())  //when visiting a node, update lostness value for active tasks
            foreach (QString task, m_activeTasks)
                m_lostnessEditor->nodeVisited(task, jsonObj[kName_Object].toString());

        //formulate human-readable string for log window
        QString sentence = jsonObj[kName_Actor].toString() + " " + jsonObj[kName_Verb].toString() + " " + jsonObj[kName_Object].toString();

        if(jsonObj.contains(kName_Result))
        {
            //if node was unlocked then show on graph
            //if(jsonObj[kName_Verb].toString() == kName_Attempted && jsonObj[kName_Result].toString() == kName_Unlocked)
                //unlockNode(jsonObj[kName_Object].toString());

            //append result to string
            sentence += kName_WithResult + jsonObj[kName_Result].toString();
        }

        if(jsonObj.contains(kName_Lostness))
        {

            //append lostness to string
            sentence += kName_WithLostness;
            sentence += jsonObj[kName_Lostness].toDouble();
        }

        sentence += kName_At + QDateTime::fromString(jsonObj[kName_Timestamp].toString(), Qt::ISODate).toString("MMM dd, yyyy hh:mm:ss t");

        //output string to window and full JSON message to file
        m_logWindow->appendToWindow(sentence);
        m_logWindow->appendToLogFile(jsonObj);
    }
}
