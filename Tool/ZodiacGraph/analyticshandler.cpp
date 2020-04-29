#include "analyticshandler.h"

//json value names
static const QString kName_Actor = "actor";
static const QString kName_Verb = "verb";
static const QString kName_Object = "object";
static const QString kName_Result = "result";
static const QString kName_With = " with ";
static const QString kName_WithResults = " with results: ";
static const QString kName_Difficulty = "difficulty";
static const QString kName_preLostness = "preEpisodeLostness";
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
static const QString kName_Found = "found";

AnalyticsHandler::AnalyticsHandler(AnalyticsLogWindow *logger, QAction *connectAction, QAction *disconnectAction, QAction *editLostnessAction, QAction *loadAction, QAction *clearAction, QObject *parent)
    : m_curatorAnalyticsEditor(new CuratorAnalyticsEditor(qobject_cast<QWidget*>(parent)))
    , m_tcpSocket(new AnalyticsSocket(qobject_cast<QWidget*>(parent)))
    , m_logWindow(logger)
    , m_connectAction(connectAction)
    , m_disconnectAction(disconnectAction)
    , m_editLostnessAction(editLostnessAction)
    , m_loadLogFileAction(loadAction)
    , m_clearAnalyticsAction(clearAction)
    , m_analyticsEnabled(false)
    , QObject(parent)
{
    connect(m_connectAction, &QAction::triggered, [=]{connectToServer();});
    connect(m_disconnectAction, &QAction::triggered, [=]{m_tcpSocket->disconnectFromServer();});
    connect(m_editLostnessAction, &QAction::triggered, [=]{m_curatorAnalyticsEditor->showWindow();});
    connect(m_loadLogFileAction, &QAction::triggered, [=]{loadAnalyticsLog();});
    connect(m_clearAnalyticsAction, &QAction::triggered, [=]{clearAll();});

    m_disconnectAction->setEnabled(false);
    m_connectAction->setEnabled(false);
    m_loadLogFileAction->setEnabled(false);
    m_clearAnalyticsAction->setEnabled(false);

    connect(m_tcpSocket, SIGNAL(connectedCallback()), this, SLOT(connected()));
    connect(m_tcpSocket, SIGNAL(disconnectedCallback()), this, SLOT(disconnected()));
    connect(m_tcpSocket, SIGNAL(readMessage(QString)), this, SLOT(readMessageFromServer(QString)));

    connect(m_curatorAnalyticsEditor, SIGNAL(finished(int)), this, SLOT(showCuratorLabels()));

    m_lostness_actions.insert(kName_JumpedTo);
    m_lostness_actions.insert(kName_PickedUp);
    m_lostness_actions.insert(kName_Examined);
}

void AnalyticsHandler::setAnalyticsProperties(AnalyticsProperties *properties)
{
    m_pProperties = properties;
}

void AnalyticsHandler::connectToServer()
{
    //ask user if they want to clear data before connecting again
    if(!m_logWindow->toPlainText().isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Previous Analytics Present");
        msgBox.setText("Do you want to clear analytics from the previous session?");
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        if(msgBox.exec() == QMessageBox::Yes)
            clearAll();
    }

    m_tcpSocket->SetUpSocket();
}

void AnalyticsHandler::connected()
{
    m_logWindow->initialiseLogFile();
    m_connectAction->setEnabled(false);
    m_disconnectAction->setEnabled(true);
    m_clearAnalyticsAction->setEnabled(false);
    m_pProperties->ConnectedtoServer(m_tcpSocket->getAddressAndPort());
    m_curatorAnalyticsEditor->setFirstNodeToStartNode();
}

void AnalyticsHandler::disconnected()
{
    m_logWindow->exportToFile();
    m_connectAction->setEnabled(true);
    m_disconnectAction->setEnabled(false);
    m_clearAnalyticsAction->setEnabled(true);

    m_pProperties->DisconnectedFromServer();
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

void AnalyticsHandler::readMessageFromServer(QString message)
{
    handleMessage(message, m_curatorAnalyticsEditor->getUseLostnessInTool());
}

void AnalyticsHandler::handleMessage(QString message, bool updateValues, bool loadLogFile)
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
            {
                if(!(jsonVal.toObject()[kName_Verb].toString() == kName_Found && updateValues))    //ignore a found action sent by the game if tool is calculating lostness
                    handleObject(jsonVal.toObject(), updateValues, loadLogFile);
            }
            else
            {
                qDebug() << "Problem with JSON string";
            }
        }
    }
    else
        if(jsonDoc.isObject())
        {
            if(!(jsonDoc.object()[kName_Verb].toString() == kName_Found && updateValues))    //ignore a found action sent by the game if tool is calculating lostness
                handleObject(jsonDoc.object(), updateValues, loadLogFile);
        }
        else
        {
            qDebug() << "Problem with JSON string";
        }
}

void AnalyticsHandler::handleObject(QJsonObject jsonObj, bool updateValues, bool loadLogFile)
{
    if(!jsonObj.contains(kName_Actor) || !jsonObj.contains(kName_Verb) || !jsonObj.contains(kName_Object) || !jsonObj.contains(kName_Timestamp))
    {
        qDebug() << "Problem with JSON object";
        return;
    }

    if(!m_curatorAnalyticsEditor->isEmpty())    //don't need any of this if no tasks to log
    {
        if(jsonObj[kName_Verb].toString() == kName_Started) //add new task to active list and set as started in properties
        {
            m_activeTasks.push_back(jsonObj[kName_Object].toString());
            m_pProperties->setCuratorLabelStarted(jsonObj[kName_Object].toString(), true);
            //handle difficulty in results
        }
        else
            if(jsonObj[kName_Verb].toString() == kName_Completed)   //task completed, get lostness value, remove from the list and update properties
            {
                if(updateValues)
                {
                    float lostness = m_curatorAnalyticsEditor->getLostnessofCuratorLabel(jsonObj[kName_Object].toString());

                    if(lostness >= 0)
                        jsonObj[kName_Lostness] = lostness;

                    m_pProperties->updateLostnessOfCuratorLabel(jsonObj[kName_Object].toString(), lostness);
                }
                else
                {
                    if(jsonObj.contains(kName_Lostness))
                        m_pProperties->updateLostnessOfCuratorLabel(jsonObj[kName_Object].toString(), jsonObj[kName_Lostness].toDouble());
                    else
                    {
                        float lostness = m_curatorAnalyticsEditor->getLostnessofCuratorLabelFromObjectives(jsonObj[kName_Object].toString());

                        if(lostness >= 0)
                            jsonObj[kName_Lostness] = lostness;

                        m_pProperties->updateLostnessOfCuratorLabel(jsonObj[kName_Object].toString(), lostness);
                    }
                }

                m_activeTasks.removeAll(jsonObj[kName_Object].toString());
            }
            else
                if(m_lostness_actions.contains(jsonObj["verb"].toString()))
                {
                    m_curatorAnalyticsEditor->updatePath(jsonObj["object"].toString(), jsonObj["verb"].toString());

                    foreach (QString task, m_activeTasks)   //update lostness and check progress
                    {
                        m_curatorAnalyticsEditor->nodeVisited(task, jsonObj["object"].toString(), jsonObj["verb"].toString());   //Update lostness values (S and N) for each curator label

                        if(updateValues)    //update lostness
                        {
                            m_pProperties->updateLostnessOfCuratorLabel(task, m_curatorAnalyticsEditor->getLostnessofCuratorLabel(task));
                        }
                    }
                }
                else
                    if(jsonObj[kName_Verb].toString() == kName_Attempted) //light up node in scene to show unlocked
                    {
                        if(jsonObj.contains(kName_Result) && jsonObj[kName_Result].isObject())
                        {
                            QJsonObject jsonResultsObj = jsonObj[kName_Result].toObject();
                            if(jsonResultsObj.contains(kName_Result) && jsonResultsObj[kName_Result].toString() == kName_Unlock)
                            {
                                unlockNode(jsonObj[kName_Object].toString());

                                //functionality needed for when tool detects objectives and lostness
                                if(updateValues && jsonObj[kName_Verb].toString() == kName_Attempted && jsonObj[kName_Result].toString() == kName_Unlock)   //if node unlocked, update curator label progress
                                    if(m_curatorAnalyticsEditor->possibleObjectiveFound(jsonObj[kName_Object].toString()))
                                    {
                                        //get variables needed for json output
                                        QString objective = jsonObj[kName_Object].toString();
                                        QString parent = m_curatorAnalyticsEditor->getParentId(jsonObj[kName_Object].toString());
                                        int r, s, n;
                                        QString startNode, endNode;
                                        float lostness = m_curatorAnalyticsEditor->getLostnessofObjective(parent, objective, r, s, n, startNode, endNode);

                                        //send to properties
                                        m_pProperties->updateLostnessOfObjective(parent, objective, lostness);
                                        m_pProperties->updateLocalLostness(m_curatorAnalyticsEditor->getLocalLostness());
                                        m_pProperties->updateFullGameProgress(m_curatorAnalyticsEditor->getGameProgress());
                                        m_pProperties->updateProgressOfCuratorLabel(parent, m_curatorAnalyticsEditor->getCuratorLabelProgress(parent));

                                        handleTextOutput(jsonObj, updateValues);    //send this out now before doing to found message immediately afterwards

                                        //make found message
                                        QJsonObject foundObject, resultObject;

                                        resultObject["r"] = r;
                                        resultObject["s"] = s;
                                        resultObject["n"] = n;
                                        resultObject[kName_Lostness] = lostness;
                                        resultObject["startNode"] = startNode;
                                        resultObject["endNode"] = endNode;
                                        resultObject["curatorLabel"] = parent;

                                        foundObject[kName_Actor] = jsonObj[kName_Actor];
                                        foundObject[kName_Object] = jsonObj[kName_Object];
                                        foundObject[kName_Verb] = kName_Found;
                                        foundObject[kName_Result] = resultObject;

                                        handleTextOutput(foundObject, true);
                                    }
                            }
                        }
                    }
                    else
                        if(jsonObj[kName_Verb].toString() == kName_Found)   //found an objective of a curator label
                        {
                            //do something with the curator label manager here
                            //add lostness to the result, maybe even remove the other stuff when done?
                            if(jsonObj.contains(kName_Result) && jsonObj[kName_Result].isObject()) //append list of results or only one result, if they are available
                            {
                                QJsonObject jsonResultsObj = jsonObj[kName_Result].toObject();
                                QString startNode, endNode, curatorID, objectiveID = "";
                                int r, s, n = 0;
                                float lostness = 0.0f;
                                objectiveID = jsonObj[kName_Object].toString();

                                if(jsonResultsObj.contains("r"))
                                    r = jsonResultsObj["r"].toInt();
                                else
                                    qDebug() << "R not found";

                                if(jsonResultsObj.contains("s"))
                                    s = jsonResultsObj["s"].toInt();
                                else
                                    qDebug() << "S not found";

                                if(jsonResultsObj.contains("n"))
                                    n = jsonResultsObj["n"].toInt();
                                else
                                    qDebug() << "N not found";

                                if(jsonResultsObj.contains("lostness"))
                                    lostness = jsonResultsObj["lostness"].toDouble();
                                else
                                    qDebug() << "Lostness not found";

                                if(jsonResultsObj.contains("startNode"))
                                    startNode = jsonResultsObj["startNode"].toString();
                                else
                                    qDebug() << "Start node not found";

                                if(jsonResultsObj.contains("endNode"))
                                    endNode = jsonResultsObj["endNode"].toString();
                                else
                                    qDebug() << "End node not found";

                                if(jsonResultsObj.contains("curatorLabel"))
                                    curatorID = jsonResultsObj["curatorLabel"].toString();
                                else
                                    qDebug() << "Curator Label not found";

                                qDebug() << "obj: " << objectiveID << "cur: " << curatorID << "r: " << r << "s: " << s << "n: " << n << "lostness" << lostness;
                                qDebug() << "start: " << startNode << "end: " << endNode;

                                //update lostness
                                m_curatorAnalyticsEditor->objectiveFound(objectiveID, curatorID, r, s, n, lostness, startNode, endNode);

                                //display all related properties in the sidebar
                                m_pProperties->updateLostnessOfObjective(curatorID, jsonObj[kName_Object].toString(), lostness);
                                m_pProperties->updateLocalLostness(m_curatorAnalyticsEditor->getLocalLostness());
                                m_pProperties->updateFullGameProgress(m_curatorAnalyticsEditor->getGameProgress());
                                m_pProperties->updateProgressOfCuratorLabel(curatorID, m_curatorAnalyticsEditor->getCuratorLabelProgress(curatorID));
                            }
                        }
    }

    handleTextOutput(jsonObj, updateValues, loadLogFile);
}

void AnalyticsHandler::handleTextOutput(QJsonObject &jsonObj, bool updateValues, bool loadLogFile)
{
    //formulate human-readable string for log window
    QString sentence = jsonObj[kName_Actor].toString() + " " + jsonObj[kName_Verb].toString() + " " + jsonObj[kName_Object].toString();

    if(jsonObj.contains(kName_Result) && jsonObj[kName_Result].isObject()) //append list of results or only one result, if they are available
    {
        QJsonObject jsonResultsObj = jsonObj[kName_Result].toObject();

        if(!jsonResultsObj.empty())
            if(jsonResultsObj.count() == 1)
            {
                if(jsonObj[kName_Result].isString())
                    sentence += kName_With + jsonResultsObj.begin().key() + " " + jsonObj[kName_Result].toString();
                else
                    if(jsonObj[kName_Result].isDouble())
                        sentence += kName_With + jsonResultsObj.begin().key() + " " + QString::number(jsonObj[kName_Result].toDouble());
            }
            else
            {
                int i = 1;

                sentence += kName_WithResults;

                foreach(const QString& key, jsonResultsObj.keys())
                {
                    if(i > 1)   //comma separate everything
                        sentence += ", ";

                    sentence += key + " = ";

                    if(jsonResultsObj.value(key).isString())
                        sentence += jsonResultsObj.value(key).toString();
                    else
                        if(jsonResultsObj.value(key).isDouble())
                            sentence += QString::number(jsonResultsObj.value(key).toDouble());

                    ++i;
                }
            }
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

    if(!(loadLogFile && !updateValues))
        m_logWindow->appendToLogFile(jsonObj);
}

void AnalyticsHandler::loadAnalyticsLog()
{
    QFile file(QFileDialog::getOpenFileName(qobject_cast<QWidget*>(parent()),
                                                     QObject::tr("Load Analytics File"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty()&& !file.fileName().isNull())
    {
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString docString = file.readAll();
            QString fileName = file.fileName();
            file.close();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(docString.toUtf8());

            if(jsonDoc.isNull() || !jsonDoc.isArray() || jsonDoc.isEmpty())
            {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
                messageBox.setFixedSize(500,200);
                return;
            }

            QMessageBox msgBox;
            msgBox.setWindowTitle("Lostness Values");
            msgBox.setText("Do you want to update the lostness values for the loaded log file?");
            msgBox.setStandardButtons(QMessageBox::Yes);
            msgBox.addButton(QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);

            if(msgBox.exec() == QMessageBox::Yes)
            {
                m_logWindow->initialiseLogFile(fileName);
                handleMessage(docString, true, true);
                m_logWindow->exportToFile();    //save new file once message is handled
            }
            else
                handleMessage(docString, false, true);
        }
        else
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
            messageBox.setFixedSize(500,200);
            return;
        }
    }
    else
    {
        qDebug() << "Load aborted by user";
        return;
    }
}

void AnalyticsHandler::clearAll()
{
    m_logWindow->setPlainText("");
    lockAllNodes();
    m_pProperties->resetAllCuratorLabels();
    m_curatorAnalyticsEditor->resetAll();
}
