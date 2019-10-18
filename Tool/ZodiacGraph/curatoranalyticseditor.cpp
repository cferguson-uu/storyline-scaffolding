#include "curatoranalyticseditor.h"

CuratorAnalyticsEditor::CuratorAnalyticsEditor(QWidget *parent)
: m_mainLayout(new QGridLayout),
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
}

void CuratorAnalyticsEditor::showWindow()
{
    m_saveBtn = new QPushButton("Save");
    m_loadBtn = new QPushButton("Load");
    m_saveBtn->setMaximumSize(100, 25);
    m_loadBtn->setMaximumSize(100, 25);

    connect(m_saveBtn, &QPushButton::released, [=]{saveCuratorLabels();});
    connect(m_loadBtn, &QPushButton::released, [=]{loadCuratorLabels();});

    m_saveBtn->setEnabled(false);

    m_mainLayout->addWidget(m_saveBtn, 0, 0);
    m_mainLayout->addWidget(m_loadBtn, 0, 1);

    m_mainLayout->setAlignment(Qt::AlignLeft);

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

            m_jsonArray = jsonDoc.array();

            for(QJsonArray::iterator mainArrayIt = m_jsonArray.begin(); mainArrayIt != m_jsonArray.end(); ++mainArrayIt)
            {
                if((*mainArrayIt).isObject())
                {
                    CuratorLabel *curatorLabel = new CuratorLabel;
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
                            if((*mainArrayIt).isObject())
                            {
                                QJsonObject depObj = (*depArrayIt).toObject();

                                if(depObj.contains("narr_id") && depObj["narr_id"].isString())
                                    curatorLabel->narrativeDependencies.insert(depObj["narr_id"].toString(), new CuratorObjective(depObj["narr_id"].toString()));
                                    //qDebug() << depObj["narr_id"].toString();
                            }
                        }

                        if(mainObj.contains("text_id") && mainObj["text_id"].isString())
                            curatorLabel->id = new QLabel(mainObj["text_id"].toString());
                            //qDebug() << mainObj["text_id"].toString();

                        if(mainObj.contains("begin_dep") && mainObj["begin_dep"].isString())
                            curatorLabel->narrativeDependencies.insert(mainObj["begin_dep"].toString(), new CuratorObjective(mainObj["begin_dep"].toString()));
                            //qDebug() << mainObj["begin_dep"].toString();

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
                    m_curatorLabels.insert(mainObj["text_id"].toString(), curatorLabel);
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

                    foreach (CuratorLabel* curatorLabel, m_curatorLabels)
                    {
                        if(curatorLabel->id->text() == mainObj["screen_id"].toString().toInt()) //add the minimum steps to the new file
                        {
                            mainObj["min_steps"] = curatorLabel->minSteps->value();
                            break;
                        }
                    }
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

    foreach (CuratorLabel* curatorLabel, m_curatorLabels)
    {
        qDebug() << curatorLabel->id->text();

        m_mainLayout->addWidget(curatorLabel->id, row, 0);
        m_mainLayout->addWidget(curatorLabel->dependenciesLabel, ++row, 0);

        int column = 0;
        foreach (CuratorObjective *dependency, curatorLabel->narrativeDependencies)
        {
            m_mainLayout->addWidget(dependency->label, row, ++column);
        }

        m_mainLayout->addWidget(curatorLabel->minStepsLabel, ++row, 0);
        m_mainLayout->addWidget(curatorLabel->minSteps, row, 1);
        ++row;
    }

    m_saveBtn->setEnabled(true);
}

void CuratorAnalyticsEditor::hideCuratorLabels()
{
    foreach (CuratorLabel* curatorLabel, m_curatorLabels)
    {
        curatorLabel->id->deleteLater();
        curatorLabel->dependenciesLabel->deleteLater();
        curatorLabel->minStepsLabel->deleteLater();
        curatorLabel->minSteps->deleteLater();

        foreach (CuratorObjective *dependency, curatorLabel->narrativeDependencies)
        {
            dependency->label->deleteLater();
        }
    }

    m_saveBtn->setEnabled(false);
}

void CuratorAnalyticsEditor::nodeVisited(QString id, QJsonObject event)
{
    Q_ASSERT(m_curatorLabels.contains(id));

    bool found = false;
    for(QList<QPair<QString, QString>>::iterator visitedIt = m_curatorLabels[id]->uniqueNodesVisited.begin(); visitedIt != m_curatorLabels[id]->uniqueNodesVisited.end(); ++visitedIt)
    {
        if((*visitedIt).first == event["object"].toString() && (*visitedIt).second == event["verb"].toString())
        {
            found = true;
            break;
        }
    }

    if(!found)
        m_curatorLabels[id]->uniqueNodesVisited.push_back(qMakePair(event["object"].toString(), event["verb"].toString()));

    ++m_curatorLabels[id]->totalNumOfNodesVisited;

}

float CuratorAnalyticsEditor::getLostnessofCuratorLabel(QString id)
{
    //if lostness already set then return it. Otherwise, calculate, save and return
    if(m_curatorLabels[id]->lostness != -1)
        return m_curatorLabels[id]->lostness;

    //R – Minimum number of nodes which need to be visited to complete a task
    //S – Total number of nodes visited whilst searching
    //N – Number of different nodes visited whilst searching
    // L = sqrt[(N/S – 1)² + (R/N – 1)²]

    Q_ASSERT(m_curatorLabels.contains(id));

    qDebug() << "Curator Label: " << id;
    qDebug() << "Minimum number of nodes (R): " << m_curatorLabels[id]->minSteps->value();
    qDebug() << "Total number of nodes visited (S): " << m_curatorLabels[id]->totalNumOfNodesVisited;
    qDebug() << "Number of different nodes visited (N): " << m_curatorLabels[id]->uniqueNodesVisited.size();
    qDebug() << "N/S: " << (float)m_curatorLabels[id]->uniqueNodesVisited.size()/(float)m_curatorLabels[id]->totalNumOfNodesVisited;
    qDebug() << "R/N: " << (float)m_curatorLabels[id]->minSteps->value()/(float)m_curatorLabels[id]->uniqueNodesVisited.size();

    //avoid div 0 errors
    if(m_curatorLabels[id]->totalNumOfNodesVisited == 0)   //no nodes visited so lostness cannot be determined
        return -1;

    float firstHalf = (float)m_curatorLabels[id]->uniqueNodesVisited.size()/(float)m_curatorLabels[id]->totalNumOfNodesVisited - 1; //(N/S – 1)²
    firstHalf *= firstHalf;

    float secondHalf = (float)m_curatorLabels[id]->minSteps->value()/(float)m_curatorLabels[id]->uniqueNodesVisited.size() - 1;   //(R/N – 1)²
    secondHalf *= secondHalf;

    float lostness = firstHalf + secondHalf;    //sqrt[(N/S – 1)² + (R/N – 1)²]
    lostness = sqrt(lostness);

    //qDebug() << "Lostness" << lostness;
    m_curatorLabels[id]->lostness = lostness;
    return lostness;

    qDebug() << "Error: task not found";    //will end up here if the loop executes without finding the task
    return -1;
}

float CuratorAnalyticsEditor::getLostnessofObjective(QString curatorId, QString objectiveId)
{
    return m_curatorLabels[curatorId]->narrativeDependencies[objectiveId]->lostness;
}


bool CuratorAnalyticsEditor::checkIfAnalyticsLoaded()
{
    if(m_curatorLabels.empty())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("No Curator Labels Loaded");
        msgBox.setText("Do you want to load the curator labels (in-game tasks) before starting analytics?");
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
    foreach (CuratorLabel* curatorLabel, m_curatorLabels)
    {
        //reset lostness
        curatorLabel->uniqueNodesVisited.clear();
        curatorLabel->totalNumOfNodesVisited = 0;
        curatorLabel->lostness = -1;

        //and progress
        curatorLabel->progress = 0;

        foreach (CuratorObjective *dependency, curatorLabel->narrativeDependencies)
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

QList<CuratorLabel*> CuratorAnalyticsEditor::getCuratorLabels()
{
    QList<CuratorLabel*> labels;

    foreach (CuratorLabel* label, m_curatorLabels)
    {
        labels.push_back(label);
    }

    return labels;
}

void CuratorAnalyticsEditor::updateGameProgress()
{
    m_gameProgress = 0;

    foreach (CuratorLabel *curatorLabel, m_curatorLabels) //Loop through, get value and average. May be better to do the objectives. Check later
    {
        m_gameProgress += curatorLabel->progress;
    }

    m_gameProgress /= m_curatorLabels.size();
}

//This can be highly optimised
void CuratorAnalyticsEditor::objectiveFound(QString objectiveId, QString curatorID, int r, int s, int n, float lostness, QString startNode, QString endNode)
{
    qDebug() << "obj: " << objectiveId << "cur: " << curatorID << "r: " << r << "s: " << s << "n: " << n << "lostness" << lostness;
    qDebug() << "start: " << startNode << "end: " << endNode;

    CuratorObjective *objective = m_curatorLabels[curatorID]->narrativeDependencies[objectiveId];

    //set objective to found and add all lostness data
    objective->found = true;
    objective->minSteps = r;
    objective->totalNumOfNodesVisited = s;
    objective->totalNumUniqueNodesVisited = n;
    objective->lostness = lostness;
    objective->startNode = startNode;
    objective->endNode = endNode;

    //update progress for curator label and full game
    m_curatorLabels[curatorID]->progress = 0;
    foreach (CuratorObjective *obj, m_curatorLabels[curatorID]->narrativeDependencies)
    {
        if(obj->found)
            ++m_curatorLabels[curatorID]->progress;
    }

    m_curatorLabels[curatorID]->progress /= m_curatorLabels[curatorID]->narrativeDependencies.size();
    updateGameProgress();
    updateLocalLostness();
}

void CuratorAnalyticsEditor::possibleObjectiveFound(QString objectiveId)
{
    //will be implemented for when tool does local lostness on it's own
}

void CuratorAnalyticsEditor::updateLocalLostness()
{
    int sumR(0), sumS(0), sumN(0);
    foreach (CuratorLabel *curatorLabel, m_curatorLabels) //Find all completed objectives, sum up lostness data and get full lostness
    {
        foreach (CuratorObjective *obj, curatorLabel->narrativeDependencies)
        {
            if(obj->found)
            {
                sumR += obj->minSteps;
                sumS += obj->totalNumOfNodesVisited;
                sumN += obj->totalNumUniqueNodesVisited;
            }
        }
    }

    float firstHalf = (float)sumN/(float)sumS - 1; //(N/S – 1)²
    firstHalf *= firstHalf;

    float secondHalf = (float)sumR/(float)sumN - 1;   //(R/N – 1)²
    secondHalf *= secondHalf;

    float lostness = firstHalf + secondHalf;    //sqrt[(N/S – 1)² + (R/N – 1)²]
    lostness = sqrt(lostness);

    m_localLostness = lostness;
}

float CuratorAnalyticsEditor::getCuratorLabelProgress(QString curatorId)
{
    return m_curatorLabels[curatorId]->progress;
}
