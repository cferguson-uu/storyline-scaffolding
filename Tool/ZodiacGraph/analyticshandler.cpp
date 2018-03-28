#include "analyticshandler.h"

//json value names
static const QString kName_Actor = "actor";
static const QString kName_Verb = "verb";
static const QString kName_Object = "object";
static const QString kName_Result = "result";
static const QString kName_WithResult = " with result ";
static const QString kName_WithLostness = " with lostness value: ";
static const QString kName_WithSimilarity = " with similarity value: ";
static const QString kName_AndSimilarity = " and similarity value: ";
static const QString kName_At = " at ";
static const QString kName_Timestamp = "timestamp";
static const QString kName_Lostness = "lostness";
static const QString kName_Similarity = "similarity";

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

AnalyticsHandler::AnalyticsHandler(AnalyticsLogWindow *logger, QAction *connectAction, QAction *disconnectAction, QAction *editLostnessAction, QAction *loadAction, QAction *clearAction, QAction *exportAction, QObject *parent)
    : m_curatorAnalyticsEditor(new CuratorAnalyticsEditor(qobject_cast<QWidget*>(parent)))
    , m_tcpSocket(new AnalyticsSocket(qobject_cast<QWidget*>(parent)))
    , m_logWindow(logger)
    , m_connectAction(connectAction)
    , m_disconnectAction(disconnectAction)
    , m_editLostnessAction(editLostnessAction)
    , m_loadLogFileAction(loadAction)
    , m_clearAnalyticsAction(clearAction)
    , m_exportAnalyticsAction(exportAction)
    , m_analyticsEnabled(false)
    , QObject(parent)
{
    connect(m_connectAction, &QAction::triggered, [=]{connectToServer();});
    connect(m_disconnectAction, &QAction::triggered, [=]{m_tcpSocket->disconnectFromServer();});
    connect(m_editLostnessAction, &QAction::triggered, [=]{m_curatorAnalyticsEditor->showWindow();});
    connect(m_loadLogFileAction, &QAction::triggered, [=]{loadAnalyticsLog();});
    connect(m_clearAnalyticsAction, &QAction::triggered, [=]{clearAll();});
    connect(m_exportAnalyticsAction, &QAction::triggered, [=]{exportTaskDataToCSV();});

    m_disconnectAction->setEnabled(false);
    m_connectAction->setEnabled(false);
    m_loadLogFileAction->setEnabled(false);
    m_clearAnalyticsAction->setEnabled(false);
    m_exportAnalyticsAction->setEnabled(false);

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
    m_clearAnalyticsAction->setEnabled(false);
    m_exportAnalyticsAction->setEnabled(false);
}

void AnalyticsHandler::disconnected()
{
    m_logWindow->exportToFile();
    m_connectAction->setEnabled(true);
    m_disconnectAction->setEnabled(false);
    m_clearAnalyticsAction->setEnabled(true);
    m_exportAnalyticsAction->setEnabled(true);
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

void AnalyticsHandler::handleMessage(QString message, bool updateValues, bool loadingFile)
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
                handleObject(jsonVal.toObject(), updateValues, loadingFile);
        }
    }
    else
        if(jsonDoc.isObject())
        {
            handleObject(jsonDoc.object(), updateValues, loadingFile);
        }
        else
        {
            qDebug() << "Problem with JSON string";
        }
}

void AnalyticsHandler::handleObject(QJsonObject jsonObj, bool updateValues, bool loadingFile)
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
        if(updateValues)
        {
            float lostness = m_curatorAnalyticsEditor->getLostnessValue(jsonObj[kName_Object].toString());
            float similarity = m_curatorAnalyticsEditor->getSimilarityValue(jsonObj[kName_Object].toString());

            if(lostness >= 0)
                jsonObj[kName_Lostness] = lostness;

            if(similarity >= 0)
                jsonObj[kName_Similarity] = similarity;

            m_pProperties->updateLostnessOfCuratorLabel(jsonObj[kName_Object].toString(), lostness);
            m_pProperties->updateSimilarityOfCuratorLabel(jsonObj[kName_Object].toString(), similarity);
        }
        else
        {
            if(jsonObj.contains(kName_Lostness))
                m_pProperties->updateLostnessOfCuratorLabel(jsonObj[kName_Object].toString(), jsonObj[kName_Lostness].toDouble());

            if(jsonObj.contains(kName_Similarity))
                m_pProperties->updateSimilarityOfCuratorLabel(jsonObj[kName_Object].toString(), jsonObj[kName_Similarity].toDouble());
        }

        m_activeTasks.removeAll(jsonObj[kName_Object].toString());
    }

    foreach (QString task, m_activeTasks)   //update lostness and sequence similarity values and check progress
    {
        m_curatorAnalyticsEditor->nodeVisited(task, jsonObj);

        if(m_pProperties->getCuratorLabelStarted(task)) //can't do anything unless task is started
        {
            if(jsonObj[kName_Verb].toString() == kName_Attempted && jsonObj[kName_Result].toString() == kName_Unlock)   //if node unlocked, update curator label progress
                m_pProperties->updateProgressOfCuratorLabel(task, jsonObj[kName_Object].toString());

            if(updateValues && !loadingFile)
            {
                m_pProperties->updateSimilarityOfCuratorLabel(task, m_curatorAnalyticsEditor->getSimilarityValue(task));
                m_pProperties->updateLostnessOfCuratorLabel(task, m_curatorAnalyticsEditor->getLostnessValue(task));
            }
        }
    }

    if(jsonObj[kName_Verb].toString() == kName_Attempted && jsonObj[kName_Result].toString() == kName_Unlock)   //light up node in scene to show unlocked
    {
        unlockNode(jsonObj[kName_Object].toString());
    }

    handleTextOutput(jsonObj, updateValues);
}

void AnalyticsHandler::handleTextOutput(QJsonObject &jsonObj, bool updateValues)
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

    if(jsonObj.contains(kName_Similarity))
    {
        //append similarity to string, use with or and depending on if lostness is present
        if(jsonObj.contains(kName_Lostness))
            sentence += kName_AndSimilarity;
        else
            sentence += kName_WithSimilarity;
        sentence += QString::number(jsonObj[kName_Similarity].toDouble());
    }

    sentence += kName_At + QDateTime::fromString(jsonObj[kName_Timestamp].toString(), Qt::ISODate).toString("MMM dd, yyyy hh:mm:ss t");

    //output string to window and full JSON message to file
    m_logWindow->appendToWindow(sentence);

    if(updateValues)
        m_logWindow->appendToLogFile(jsonObj);
}

void AnalyticsHandler::loadAnalyticsLog()
{
    /*QFile file(QFileDialog::getOpenFileName(qobject_cast<QWidget*>(parent()),
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
            msgBox.setWindowTitle("Lostness and Similarity Values");
            msgBox.setText("Do you want to update the lostness and similarity values for the loaded log file?");
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

    m_exportAnalyticsAction->setEnabled(true);*/


   QStringList fileNames(QFileDialog::getOpenFileNames(qobject_cast<QWidget*>(parent()),
                                                     QObject::tr("Load Analytics File"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));
    foreach (QString fileName, fileNames)
    {
        QFile file(fileName);

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

                m_logWindow->initialiseLogFile(fileName);
                handleMessage(docString, true, true);
                m_logWindow->exportToFile();    //save new file once message is handled
                exportTaskDataToCSV(fileName.section(".",0,0));
                clearAll();
            }
            else
            {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
                messageBox.setFixedSize(500,200);
                return;
            }
        }
    }

}

void AnalyticsHandler::clearAll()
{
    m_logWindow->setPlainText("");
    lockAllNodes();
    m_pProperties->resetAllCuratorLabels();
    m_curatorAnalyticsEditor->resetAllLostnessAndSequenceCalculations();
    m_exportAnalyticsAction->setEnabled(true);
}

void AnalyticsHandler::exportTaskDataToCSV(QString fileName)
{
    QFile file(QFileDialog::getSaveFileName(qobject_cast<QWidget*>(parent()),
                                                         QObject::tr("Save task data"), fileName,
                                                        QObject:: tr("CSV File (*.csv);;All Files (*)")));

    if(!file.fileName().isEmpty()&& !file.fileName().isNull())
    {
        if(!file.open(QFile::WriteOnly))
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
            messageBox.setFixedSize(500,200);
        }
        else
        {
            QLocale locale;
            QTextStream output(&file);
            QList<CuratorLabel*> curatorTasks = m_curatorAnalyticsEditor->getCuratorLabels();

            output << "Task;Dependencies;Progress;Lostness;Similarity";

            foreach (CuratorLabel *task, curatorTasks)
            {
                QString taskID = task->id->text();

                output << "\n" << taskID << ";\"" << locale.toString(task->narrativeDependencies.size()) << "\";\"" << locale.toString(m_pProperties->getProgressOfCuratorLabel(taskID))
                       << "\";\"" << locale.toString(m_pProperties->getLostnessOfCuratorLabel(taskID)) << "\";\"" << locale.toString(m_pProperties->getSimilarityOfCuratorLabel(taskID)) << "\"";
            }

            output << "\nProgress;" << "\"" << locale.toString(m_pProperties->getFullGameProgress())<< "\"";

            file.close();
        }
    }
}
