#include "curatoranalyticseditor.h"

CuratorAnalyticsEditor::CuratorAnalyticsEditor(QWidget *parent)
: m_mainLayout(new QGridLayout),
  m_saveCuratorBtn(nullptr),
  m_loadCuratorBtn(nullptr),
  m_useTool(nullptr),
  QDialog(parent)
{
    QVBoxLayout *parentLayout = new QVBoxLayout;
    QScrollArea *scrollArea = new QScrollArea;
    QWidget *widg = new QWidget();

    m_mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    m_mainLayout->setAlignment(Qt::AlignTop);
    widg->setLayout(m_mainLayout);
    scrollArea->setWidget(widg);

    parentLayout->addWidget(scrollArea);
    setLayout(parentLayout);

    setWindowTitle(tr("Curator Labels"));
    resize(QSize(400,400));

    m_loadedLabel = new QLabel("No tasks loaded");

    m_saveCuratorBtn = new QPushButton("Save Tasks");
    m_loadCuratorBtn = new QPushButton("Load Tasks");
    m_saveCuratorBtn->setMaximumSize(100, 25);
    m_loadCuratorBtn->setMaximumSize(100, 25);

    m_saveEdgesBtn = new QPushButton("Save Spatial Graph");
    m_loadEdgesBtn = new QPushButton("Load Spatial Graph");
    m_saveEdgesBtn->setMaximumSize(100, 25);
    m_loadEdgesBtn->setMaximumSize(100, 25);

    m_saveEdgesBtn->setVisible(false);
    m_loadEdgesBtn->setVisible(false);

    m_startNodeLabel = new QLabel("Starting Node:");
    m_startNodeInput = new QLineEdit();

    m_startNodeLabel->setVisible(false);
    m_startNodeInput->setVisible(false);

    m_useGlobalLostness = new QRadioButton(tr("&Use Global Lostness"));;
    m_useLocalLostness = new QRadioButton(tr("&Use Local Lostness"));;
    m_useNoLostness = new QRadioButton(tr("&No Lostness"));;
    m_useNoLostness->setChecked(true);

    connect(m_saveCuratorBtn, &QPushButton::released, [=]{saveCuratorLabels();});
    connect(m_loadCuratorBtn, &QPushButton::released, [=]{loadCuratorLabels();});

    connect(m_loadEdgesBtn, &QPushButton::released, [=]{loadSpatialGraph();});

    connect(m_useGlobalLostness, &QRadioButton::toggled, [=](bool checked){onGlobalSelected(checked);});
    connect(m_useLocalLostness, &QRadioButton::toggled, [=](bool checked){onLocalSelected(checked);});

    m_saveCuratorBtn->setEnabled(false);
    m_saveEdgesBtn->setEnabled(false);
    m_startNodeInput->setEnabled(false);

    m_mainLayout->addWidget(m_loadedLabel, 0, 0);

    m_mainLayout->addWidget(m_useGlobalLostness, 1, 0);
    m_mainLayout->addWidget(m_useLocalLostness, 1, 1);
    m_mainLayout->addWidget(m_useNoLostness, 1, 2);

    m_mainLayout->addWidget(m_saveCuratorBtn, 2, 0);
    m_mainLayout->addWidget(m_loadCuratorBtn, 2, 1);

    m_mainLayout->addWidget(m_saveEdgesBtn, 3, 0);
    m_mainLayout->addWidget(m_loadEdgesBtn, 3, 1);

    m_mainLayout->addWidget(m_startNodeLabel, 4, 0);
    m_mainLayout->addWidget(m_startNodeInput, 4, 1);

    m_mainLayout->setAlignment(Qt::AlignLeft);
}

void CuratorAnalyticsEditor::onLocalSelected(bool checked)
{
    m_saveEdgesBtn->setVisible(checked);
    m_loadEdgesBtn->setVisible(checked);

    m_startNodeLabel->setVisible(checked);
    m_startNodeInput->setVisible(checked);
}

void CuratorAnalyticsEditor::onGlobalSelected(bool checked)
{
    foreach (CuratorLabel* curatorLabel, m_curatorLabelsList)
    {
        curatorLabel->minStepsLabel->setVisible(checked);
        curatorLabel->minSteps->setVisible(checked);
    }
}

void CuratorAnalyticsEditor::showWindow()
{
    show();
}

void CuratorAnalyticsEditor::loadCuratorLabels()
{
    QFile file(QFileDialog::getOpenFileName(this,
                                                     QObject::tr("Load Curator Labels"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty() && !file.fileName().isNull())
    {
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString settings = file.readAll();
            file.close();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(settings.toUtf8());

            if(jsonDoc.isNull() || !jsonDoc.isArray() || jsonDoc.isEmpty())
            {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
                messageBox.setFixedSize(500,200);
                //close window?
            }

            m_useTool = new QCheckBox("Calculate lostness within tool");    //check if user wants to calculate lostness within the tool

            m_jsonArray = jsonDoc.array();

            for(QJsonArray::iterator mainArrayIt = m_jsonArray.begin(); mainArrayIt != m_jsonArray.end(); ++mainArrayIt)
            {
                if((*mainArrayIt).isObject())
                {
                    CuratorLabel *curatorLabel = new CuratorLabel;
                    curatorLabel->startDependencyLabel = new QLabel("Start Objective:");
                    curatorLabel->dependenciesLabel = new QLabel("Objectives:");
                    curatorLabel->minStepsLabel = new QLabel("Minimum Steps:");
                    curatorLabel->totalNumOfNodesVisited = 0;

                    curatorLabel->lostness = -1;
                    curatorLabel->progress = 0;

                    QJsonObject mainObj = (*mainArrayIt).toObject();

                    if(mainObj.contains("narrative_deps") && mainObj["narrative_deps"].isArray())
                    {
                        QJsonArray depArray = mainObj["narrative_deps"].toArray();

                        for(QJsonArray::iterator depArrayIt = depArray.begin(); depArrayIt != depArray.end(); ++depArrayIt)
                        {                            
                            if(mainObj.contains("text_id") && mainObj["text_id"].isString())
                            {
                                curatorLabel->id = new QLabel(mainObj["text_id"].toString());
                                curatorLabel->id->setStyleSheet("font-weight: bold;");
                                //qDebug() << mainObj["text_id"].toString();
                            }

                            if(mainObj.contains("begin_dep") && mainObj["begin_dep"].isString())
                            {
                                CuratorObjective *newObj = new CuratorObjective(mainObj["begin_dep"].toString());
                                curatorLabel->startDependency = newObj;
                                //curatorLabel->narrativeDependenciesHash.insert(mainObj["begin_dep"].toString(), newObj);
                                //curatorLabel->narrativeDependenciesList.append(newObj);
                                //qDebug() << mainObj["begin_dep"].toString();
                            }

                            if((*mainArrayIt).isObject())
                            {
                                QJsonObject depObj = (*depArrayIt).toObject();

                                if(depObj.contains("narr_id") && depObj["narr_id"].isString())
                                {
                                    CuratorObjective *newObj = new CuratorObjective(depObj["narr_id"].toString());
                                    curatorLabel->narrativeDependenciesHash.insert(depObj["narr_id"].toString(), newObj);
                                    curatorLabel->narrativeDependenciesList.append(newObj);
                                    //qDebug() << depObj["narr_id"].toString();
                                }
                            }
                        }

                        /*if(depObj.contains("subtitle") && depObj["subtitle"].isString())
                            qDebug() << depObj["subtitle"].toString();

                        if(depObj.contains("screen_id") && depObj["screen_id"].isString())
                            qDebug() << depObj["screen_id"].toString();

                        if(depObj.contains("element_from") && depObj["element_from"].isString())
                            qDebug() << depObj["element_from"].toString();

                        if(depObj.contains("element_to") && depObj["element_to"].isString())
                            qDebug() << depObj["element_to"].toString();

                        if(mainObj.contains("screen_id") && mainObj["screen_id"].isString())
                            qDebug() << mainObj["screen_id"].toString();

                        if(mainObj.contains("element_id") && mainObj["element_id"].isString())
                            qDebug() << mainObj["element_id"].toString();

                        if(mainObj.contains("subtitle") && mainObj["subtitle"].isString())
                            qDebug() << mainObj["subtitle"].toString();

                        if(mainObj.contains("target_alpha") && mainObj["target_alpha"].isString())
                            qDebug() << mainObj["target_alpha"].toString();

                        if(mainObj.contains("complete_dep") && mainObj["complete_dep"].isString())
                            qDebug() << mainObj["complete_dep"].toString();

                        if(mainObj.contains("on_progress_completed") && mainObj["on_progress_completed"].isString())
                            qDebug() << mainObj["on_progress_completed"].toString();*/


                        curatorLabel->minSteps = new QSpinBox();

                        if(mainObj.contains("min_steps"))
                                curatorLabel->minSteps->setValue(mainObj["min_steps"].toDouble());
                    }
                    m_curatorLabelsHash.insert(mainObj["text_id"].toString(), curatorLabel);
                    m_curatorLabelsList.append(curatorLabel);

                    curatorLabel->minStepsLabel->setVisible(m_useGlobalLostness->isChecked());
                    curatorLabel->minSteps->setVisible(m_useGlobalLostness->isChecked());
                }
            }

            showCuratorLabels();
        }
        else
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
            messageBox.setFixedSize(500,200);
        }
    }
    else
    {
        qDebug() << "Load aborted by user";
    }
}

void CuratorAnalyticsEditor::saveCuratorLabels()
{
    QFile file(QFileDialog::getSaveFileName(this,
                                                     QObject::tr("Save Curator Labels"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty()&& !file.fileName().isNull())
    {
        if(file.open(QFile::WriteOnly))
        {
            QJsonDocument newJsonDoc;
            QJsonArray newJsonArray;

            for(QJsonArray::iterator mainArrayIt = m_jsonArray.begin(); mainArrayIt != m_jsonArray.end(); ++mainArrayIt)
            {
                if((*mainArrayIt).isObject())
                {
                    QJsonObject mainObj = (*mainArrayIt).toObject();
                    mainObj["min_steps"] = m_curatorLabelsHash[mainObj["text_id"].toString()]->minSteps->value();

                    newJsonArray.append(mainObj);   //append updated object to the new json file
                }
            }

            newJsonDoc.setArray(newJsonArray);  //write new json array to file
            file.write(newJsonDoc.toJson());
        }
        else
        {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","File could not be opened.");
                messageBox.setFixedSize(500,200);
        }
    }
    else
        qDebug() << "Save aborted by user";
}

void CuratorAnalyticsEditor::showCuratorLabels()
{
    int row = m_mainLayout->rowCount();

    m_mainLayout->addWidget(m_useTool, row, 0);

    foreach (CuratorLabel* curatorLabel, m_curatorLabelsList)
    {
        qDebug() << curatorLabel->id->text();

        m_mainLayout->addWidget(curatorLabel->id, ++row, 0);
        m_mainLayout->addWidget(curatorLabel->startDependencyLabel, ++row, 0);
        m_mainLayout->addWidget(curatorLabel->startDependency->label, row, 1);
        m_mainLayout->addWidget(curatorLabel->dependenciesLabel, ++row, 0);

        foreach (CuratorObjective *dependency, curatorLabel->narrativeDependenciesList)
        {
            m_mainLayout->addWidget(dependency->label, row, 1);
            ++row;
        }

        m_mainLayout->addWidget(curatorLabel->minStepsLabel, row, 0);
        m_mainLayout->addWidget(curatorLabel->minSteps, row, 1);
    }

    m_saveCuratorBtn->setEnabled(true);
}

void CuratorAnalyticsEditor::hideCuratorLabels()
{
    foreach (CuratorLabel* curatorLabel, m_curatorLabelsList)
    {
        curatorLabel->id->deleteLater();
        curatorLabel->startDependencyLabel->deleteLater();
        curatorLabel->startDependency->label->deleteLater();
        curatorLabel->dependenciesLabel->deleteLater();
        curatorLabel->minStepsLabel->deleteLater();
        curatorLabel->minSteps->deleteLater();

        foreach (CuratorObjective *dependency, curatorLabel->narrativeDependenciesList)
        {
            dependency->label->deleteLater();
        }
    }

    m_saveCuratorBtn->setEnabled(false);
}

void CuratorAnalyticsEditor::loadSpatialGraph()
{
    if(m_lostnessHandler.loadSpatialGraph())
    {
        m_loadedLabel->setText(QString::number(m_lostnessHandler.getNumEdges()) + " edges loaded");
        m_startNodeInput->setEnabled(true);
    }
}

void CuratorAnalyticsEditor::updatePath(QString object, QString verb)
{
    bool found = false;
    for(QList<QPair<QString, QString>>::iterator visitedIt = m_uniqueNodes.begin(); visitedIt != m_uniqueNodes.end(); ++visitedIt)
    {
        if((*visitedIt).first == object && (*visitedIt).second == verb)
        {
            found = true;
            break;
        }
    }

    if(!found)
        m_uniqueNodes.push_back(qMakePair(object, verb));

    ++m_totalNodes;

    m_endNode = object;

    if(verb == "jumped to")
        m_lastLocomotionNode = object;
}

void CuratorAnalyticsEditor::nodeVisited(QString id, QString object, QString verb)
{
    Q_ASSERT(m_curatorLabelsHash.contains(id));

    bool found = false;
    for(QList<QPair<QString, QString>>::iterator visitedIt = m_curatorLabelsHash[id]->uniqueNodesVisited.begin(); visitedIt != m_curatorLabelsHash[id]->uniqueNodesVisited.end(); ++visitedIt)
    {
        if((*visitedIt).first == object && (*visitedIt).second == verb)
        {
            found = true;
            break;
        }
    }

    if(!found)
        m_curatorLabelsHash[id]->uniqueNodesVisited.push_back(qMakePair(object, verb));

    ++m_curatorLabelsHash[id]->totalNumOfNodesVisited;

}

float CuratorAnalyticsEditor::getLostnessofCuratorLabel(QString id)
{
    Q_ASSERT(m_curatorLabelsHash.contains(id));

    CuratorLabel* curatorLabel = m_curatorLabelsHash[id];

    //if lostness already set then return it. Otherwise, calculate, save and return
    if(curatorLabel->lostness != -1)
        return curatorLabel->lostness;

    qDebug() << "Calculating lostness for Curator Label: " << id;

    float lostness = m_lostnessHandler.getLostnessValue(curatorLabel->minSteps->value(), curatorLabel->totalNumOfNodesVisited, curatorLabel->uniqueNodesVisited.size());

    //qDebug() << "Lostness: " << lostness;

    curatorLabel->lostness = lostness;
    return lostness;
}

float CuratorAnalyticsEditor::getLostnessofCuratorLabelFromObjectives(QString id)
{
    Q_ASSERT(m_curatorLabelsHash.contains(id));

    CuratorLabel* curatorLabel = m_curatorLabelsHash[id];
    qDebug() << "Calculating lostness for Curator Label: " << id;

    int sumR(0), sumS(0), sumN(0);
    QString startNode = m_curatorLabelsHash[id]->startDependency->startNode;
    QString endNode = m_endNode;

    foreach (CuratorObjective *obj, curatorLabel->narrativeDependenciesList)
    {
        if(obj->found) //if this isn't true, we have a problem
        {
            sumR += obj->minSteps;
            sumS += obj->totalNumOfNodesVisited;
            sumN += obj->totalNumUniqueNodesVisited;
        }
    }

    curatorLabel->totalNumOfNodesVisited = sumS;
    curatorLabel->totalNumUniqueNodesVisited = sumN;
    curatorLabel->startNode = startNode;
    curatorLabel->endNode = endNode;

    float lostness = m_lostnessHandler.getLostnessValue(sumR, sumS, sumN);

    //qDebug() << "Lostness: " << lostness;

    curatorLabel->lostness = lostness;
    return lostness;
}

float CuratorAnalyticsEditor::getLostnessofObjective(QString curatorId, QString objectiveId)
{
    Q_ASSERT(m_curatorLabelsHash.contains(curatorId));
    Q_ASSERT(m_curatorLabelsHash[curatorId]->narrativeDependenciesHash.contains(objectiveId) || m_curatorLabelsHash[curatorId]->startDependency->label->text() == objectiveId);

    CuratorObjective *objective;

    if(m_curatorLabelsHash[curatorId]->narrativeDependenciesHash.contains(objectiveId))
        objective = m_curatorLabelsHash[curatorId]->narrativeDependenciesHash[objectiveId];
    else
        objective = m_curatorLabelsHash[curatorId]->startDependency;

    if(objective->lostness != -1)
        return objective->lostness;

    qDebug() << "Calculating lostness for Objective: " << objectiveId << " from Curator Label: " << curatorId;

    float lostness = m_lostnessHandler.getLostnessForObjective(objective->startNode, objective->endNode, objective->totalNumOfNodesVisited, objective->totalNumUniqueNodesVisited, objective->minSteps);

    //qDebug() << "Lostness: " << lostness;

    objective->lostness = lostness;
    return lostness;
}

float CuratorAnalyticsEditor::getLostnessofObjective(QString curatorId, QString objectiveId, int &r, int &s, int &n, QString &startNode, QString &endNode)
{
    Q_ASSERT(m_curatorLabelsHash.contains(curatorId));
    Q_ASSERT(m_curatorLabelsHash[curatorId]->narrativeDependenciesHash.contains(objectiveId) || m_curatorLabelsHash[curatorId]->startDependency->label->text() == objectiveId);

    CuratorObjective *objective;

    if(m_curatorLabelsHash[curatorId]->narrativeDependenciesHash.contains(objectiveId))
        objective = m_curatorLabelsHash[curatorId]->narrativeDependenciesHash[objectiveId];
    else
        objective = m_curatorLabelsHash[curatorId]->startDependency;

    float lostness = getLostnessofObjective(curatorId, objectiveId);

    r = objective->minSteps;
    s = objective->totalNumOfNodesVisited;
    n = objective->totalNumUniqueNodesVisited;
    startNode = objective->startNode;
    endNode = objective->endNode;

    return lostness;
}

bool CuratorAnalyticsEditor::checkIfAnalyticsLoaded()
{
    bool showAlert(false);
    QString title(""), message("");

    if(m_curatorLabelsList.empty() && m_lostnessHandler.getNumEdges() == 0)
    {
        showAlert = true;
        title = "No Curator Labels or Spatial Graph Loaded";
        message = "Do you want to load the curator labels (in-game tasks) and spatial graph before starting analytics?\n\n(Needed for task analysis and lostness calculations)";
    }
    else
        if(m_curatorLabelsList.empty())
        {
            showAlert = true;
            title = "No Curator Labels Loaded";
            message = "Do you want to load the curator labels (in-game tasks) before starting analytics?\n\n(Needed for task analysis)";
        }
    else
        if(m_lostnessHandler.getNumEdges() == 0)
        {
            showAlert = true;
            title = "No Spatial Graph Loaded";
            message = "Do you want to load the spatial graph before starting analytics?\n\n(Needed for lostness calculations)";
        }
        else
            if(m_startNodeInput->text().isEmpty())
            {
                showAlert = true;
                title = "No Start Node Set";
                message = "Do you want to specify the start node before starting analytics?\n\n(First lostness calculation will not take place without this)";
            }


    //if(m_curatorLabelsList.empty())
    if(showAlert)
    {
        QMessageBox msgBox;
        //msgBox.setWindowTitle();
        //msgBox.setText("Do you want to load the curator labels (in-game tasks) before starting analytics?");
        msgBox.setWindowTitle(title);
        msgBox.setText(message);
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if(msgBox.exec() == QMessageBox::Yes)
        {
          showWindow();
          return false;
        }
        else
            return true;
    }
    else
        return true;
}

void CuratorAnalyticsEditor::resetAll()
{
    foreach (CuratorLabel* curatorLabel, m_curatorLabelsList)
    {
        //reset lostness
        curatorLabel->uniqueNodesVisited.clear();
        curatorLabel->totalNumOfNodesVisited = 0;
        curatorLabel->lostness = -1;

        //and progress
        curatorLabel->progress = 0;

        //starting node
        curatorLabel->startDependency->minSteps = 0;
        curatorLabel->startDependency->totalNumOfNodesVisited = 0;
        curatorLabel->startDependency->totalNumUniqueNodesVisited = 0;
        curatorLabel->startDependency->lostness = -1;
        curatorLabel->startDependency->startNode = "";
        curatorLabel->startDependency->endNode = "";
        curatorLabel->startDependency->found = false;

        //and other dependencies (duplicate code I know)
        foreach (CuratorObjective *dependency, curatorLabel->narrativeDependenciesList)
        {
            dependency->minSteps = 0;
            dependency->totalNumOfNodesVisited = 0;
            dependency->totalNumUniqueNodesVisited = 0;
            dependency->lostness = -1;
            dependency->startNode = "";
            dependency->endNode = "";
            dependency->found = false;
        }
    }

    m_gameProgress = 0;
    m_localLostness = 0;
}

void CuratorAnalyticsEditor::updateGameProgress()
{
    m_gameProgress = 0;

    foreach (CuratorLabel *curatorLabel, m_curatorLabelsList) //Loop through, get value and average. May be better to do the objectives. Check later
    {
        m_gameProgress += curatorLabel->progress;
    }

    m_gameProgress /= m_curatorLabelsList.size();
}

//This can be highly optimised
void CuratorAnalyticsEditor::objectiveFound(QString objectiveId, QString curatorId, int r, int s, int n, float lostness, QString startNode, QString endNode)
{
    Q_ASSERT(m_curatorLabelsHash.contains(curatorId));
    Q_ASSERT(m_curatorLabelsHash[curatorId]->narrativeDependenciesHash.contains(objectiveId) || m_curatorLabelsHash[curatorId]->startDependency->label->text() == objectiveId);

    qDebug() << "obj: " << objectiveId << "cur: " << curatorId << "r: " << r << "s: " << s << "n: " << n << "lostness" << lostness;
    qDebug() << "start: " << startNode << "end: " << endNode;

    CuratorObjective *objective;

    if(m_curatorLabelsHash[curatorId]->narrativeDependenciesHash.contains(objectiveId))
        objective = m_curatorLabelsHash[curatorId]->narrativeDependenciesHash[objectiveId];
    else
        objective = m_curatorLabelsHash[curatorId]->startDependency;

    //set objective to found and add all lostness data
    objective->found = true;
    objective->minSteps = r;
    objective->totalNumOfNodesVisited = s;
    objective->totalNumUniqueNodesVisited = n;
    objective->lostness = lostness;
    objective->startNode = startNode;
    objective->endNode = endNode;

    //update progress for curator label and full game
    m_curatorLabelsHash[curatorId]->progress = 0;
    foreach (CuratorObjective *obj, m_curatorLabelsHash[curatorId]->narrativeDependenciesList)
    {
        if(obj->found)
            ++m_curatorLabelsHash[curatorId]->progress;
    }

    m_curatorLabelsHash[curatorId]->progress /= m_curatorLabelsHash[curatorId]->narrativeDependenciesList.size();
    updateGameProgress();
    updateLocalLostness();
}

bool CuratorAnalyticsEditor::possibleObjectiveFound(QString objectiveId)
{
    CuratorObjective* objective = nullptr;

    foreach (CuratorLabel* task, m_curatorLabelsHash)
    {
        if(task->narrativeDependenciesHash.contains(objectiveId))
        {
            objective = task->narrativeDependenciesHash[objectiveId];
        }
        else
            if(task->startDependency->label->text() == objectiveId)
            {
                objective = task->startDependency;
            }
    }

    if(objective == nullptr)
        return false; //not found

    //save the information needed for lostness to the objective
    objective->found = true;
    objective->startNode = m_firstNode;
    objective->endNode = m_endNode;
    objective->totalNumOfNodesVisited = m_totalNodes;
    objective->totalNumUniqueNodesVisited = m_uniqueNodes.size();

    //reset everything for next objective
    m_firstNode = m_lastLocomotionNode;
    m_endNode = "";
    m_lastLocomotionNode = "";

    m_totalNodes = 0;
    m_uniqueNodes.clear();

    return true;
}

void CuratorAnalyticsEditor::updateLocalLostness()
{
    int sumR(0), sumS(0), sumN(0);
    foreach (CuratorLabel *curatorLabel, m_curatorLabelsList) //Find all completed objectives, sum up lostness data and get full lostness
    {
        if(curatorLabel->startDependency->found)
        {
            sumR += curatorLabel->startDependency->minSteps;
            sumS += curatorLabel->startDependency->totalNumOfNodesVisited;
            sumN += curatorLabel->startDependency->totalNumUniqueNodesVisited;
        }

        foreach (CuratorObjective *obj, curatorLabel->narrativeDependenciesList)
        {
            if(obj->found)
            {
                sumR += obj->minSteps;
                sumS += obj->totalNumOfNodesVisited;
                sumN += obj->totalNumUniqueNodesVisited;
            }
        }
    }

    m_localLostness = m_lostnessHandler.getLostnessValue(sumR, sumS, sumN);
}

float CuratorAnalyticsEditor::getCuratorLabelProgress(QString curatorId)
{
    return m_curatorLabelsHash[curatorId]->progress;
}

QString CuratorAnalyticsEditor::getParentId(QString objectiveId)
{
    foreach (CuratorLabel* task, m_curatorLabelsHash)
    {
        if(task->narrativeDependenciesHash.contains(objectiveId) || task->startDependency->label->text() == objectiveId)
        {
            return task->id->text();
        }
    }
    return "";  //error
}
