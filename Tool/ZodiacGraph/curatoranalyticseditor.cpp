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

    //add ignored actions
    m_ignored_actions.insert("looked at");
    m_ignored_actions.insert("stopped looking at");
    m_ignored_actions.insert("dropped");
    m_ignored_actions.insert("attempted");
}

void CuratorAnalyticsEditor::showWindow()
{
    m_saveBtn = new QPushButton("Save");
    m_loadBtn = new QPushButton("Load");
    m_loadFullSequenceBtn = new QPushButton("Load Perfect Full Sequence");
    m_saveFullSequenceBtn = new QPushButton("Save Perfect Full Sequence");
    m_saveBtn->setMaximumSize(100, 25);
    m_loadBtn->setMaximumSize(100, 25);
    m_loadFullSequenceBtn->setMaximumSize(150, 25);
    m_saveFullSequenceBtn->setMaximumSize(150, 25);
    m_saveBtn->setMinimumSize(100, 25);
    m_loadBtn->setMinimumSize(100, 25);
    m_loadFullSequenceBtn->setMinimumSize(150, 25);
    m_saveFullSequenceBtn->setMinimumSize(150, 25);

    connect(m_saveBtn, &QPushButton::released, [=]{saveCuratorLabels();});
    connect(m_loadBtn, &QPushButton::released, [=]{loadCuratorLabels();});
    connect(m_loadFullSequenceBtn, &QPushButton::released, [=]{addSequenceToAllCuratorLabels();});
    connect(m_saveFullSequenceBtn, &QPushButton::released, [=]{saveAllPerfectSequencesToFile();});

    m_saveBtn->setEnabled(false);
    m_loadFullSequenceBtn->setEnabled(false);
    m_saveFullSequenceBtn->setEnabled(false);

    m_mainLayout->addWidget(m_saveBtn, 0, 0);
    m_mainLayout->addWidget(m_loadBtn, 0, 1);
    m_mainLayout->addWidget(m_loadFullSequenceBtn, 1, 0);
    m_mainLayout->addWidget(m_saveFullSequenceBtn, 1, 1);

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
                    curatorLabel->dependenciesLabel = new QLabel("Dependencies:");
                    curatorLabel->minStepsLabel = new QLabel("Minimum Steps:");
                    curatorLabel->totalNumOfNodesVisited = 0;

                    curatorLabel->addSequenceBtn = new QPushButton("Add Perfect Sequence");
                    connect(curatorLabel->addSequenceBtn, &QPushButton::released, [=]{addSequenceToSingleCuratorLabel(curatorLabel, readSequenceFromFile());});

                    curatorLabel->sequenceStatus = new QLabel("Sequence Not Loaded");
                    curatorLabel->sequenceStatus->setStyleSheet("QLabel { color : red }");

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
                                    curatorLabel->narrativeDependencies.push_back(new QLabel(depObj["narr_id"].toString()));
                                    //qDebug() << depObj["narr_id"].toString();

                                /*if(depObj.contains("subtitle") && depObj["subtitle"].isString())
                                    qDebug() << depObj["subtitle"].toString();

                                if(depObj.contains("screen_id") && depObj["screen_id"].isString())
                                    qDebug() << depObj["screen_id"].toString();

                                if(depObj.contains("element_from") && depObj["element_from"].isString())
                                    qDebug() << depObj["element_from"].toString();

                                if(depObj.contains("element_to") && depObj["element_to"].isString())
                                    qDebug() << depObj["element_to"].toString();*/
                            }
                        }

                        if(mainObj.contains("text_id") && mainObj["text_id"].isString())
                            curatorLabel->id = new QLabel(mainObj["text_id"].toString());
                            //qDebug() << mainObj["text_id"].toString();

                        /*if(mainObj.contains("screen_id") && mainObj["screen_id"].isString())
                            qDebug() << mainObj["screen_id"].toString();

                        if(mainObj.contains("element_id") && mainObj["element_id"].isString())
                            qDebug() << mainObj["element_id"].toString();

                        if(mainObj.contains("subtitle") && mainObj["subtitle"].isString())
                            qDebug() << mainObj["subtitle"].toString();

                        if(mainObj.contains("target_alpha") && mainObj["target_alpha"].isString())
                            qDebug() << mainObj["target_alpha"].toString();

                        if(mainObj.contains("begin_dep") && mainObj["begin_dep"].isString())
                            qDebug() << mainObj["begin_dep"].toString();

                        if(mainObj.contains("complete_dep") && mainObj["complete_dep"].isString())
                            qDebug() << mainObj["complete_dep"].toString();

                        if(mainObj.contains("on_progress_completed") && mainObj["on_progress_completed"].isString())
                            qDebug() << mainObj["on_progress_completed"].toString();*/


                        curatorLabel->minSteps = new QSpinBox();

                        if(mainObj.contains("min_steps") && mainObj["min_steps"].isString())
                            curatorLabel->minSteps->setValue(mainObj["min_steps"].toDouble());
                    }
                    m_curatorLabels.push_back(curatorLabel);
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
        m_mainLayout->addWidget(curatorLabel->id, row, 0);
        ++row;
        m_mainLayout->addWidget(curatorLabel->dependenciesLabel, row, 0);
        ++row;

        int column = 0;
        foreach (QLabel* dependency, curatorLabel->narrativeDependencies)
        {
            m_mainLayout->addWidget(dependency, row, column);
            ++column;
        }
        ++row;

        m_mainLayout->addWidget(curatorLabel->minStepsLabel, row, 0);
        m_mainLayout->addWidget(curatorLabel->minSteps, row, 1);
        ++row;

        m_mainLayout->addWidget(curatorLabel->addSequenceBtn, row, 0);
        m_mainLayout->addWidget(curatorLabel->sequenceStatus, row, 1);
        ++row;
    }

    m_saveBtn->setEnabled(true);
    m_loadFullSequenceBtn->setEnabled(true);
}

void CuratorAnalyticsEditor::hideCuratorLabels()
{
    foreach (CuratorLabel* curatorLabel, m_curatorLabels)
    {
        curatorLabel->id->deleteLater();
        curatorLabel->dependenciesLabel->deleteLater();
        curatorLabel->minStepsLabel->deleteLater();
        curatorLabel->minSteps->deleteLater();

        foreach (QLabel* dependency, curatorLabel->narrativeDependencies)
        {
            dependency->deleteLater();
        }
    }

    m_saveBtn->setEnabled(false);
    m_loadFullSequenceBtn->setEnabled(false);
}

void CuratorAnalyticsEditor::nodeVisited(QString task, QJsonObject event)
{
    qDebug() << event["verb"].toString();
    qDebug() << event["object"].toString();

    //just want to be logging what the player has visited, picked up, examined etc.
    if(m_ignored_actions.contains(event["verb"].toString()))
        return;

    qDebug() << "yes";

    foreach (CuratorLabel* curatorLabel, m_curatorLabels)
    {
        if(curatorLabel->id->text() == task)
        {
            bool found = false;

            for(QList<QPair<QString, QString>>::iterator visitedIt = curatorLabel->uniqueNodesVisited.begin(); visitedIt != curatorLabel->uniqueNodesVisited.end(); ++visitedIt)
            {
                if((*visitedIt).first == event["object"].toString() && (*visitedIt).second == event["verb"].toString())
                {
                    found = true;
                    break;
                }
            }

            if(!found)
                curatorLabel->uniqueNodesVisited.push_back(qMakePair(event["object"].toString(), event["verb"].toString()));

            ++curatorLabel->totalNumOfNodesVisited;

            //update the sequence for sequence similarity
            curatorLabel->sequenceMatcher.compareLatestUserSequence(event); //this will return the sequence matching value

            break;
        }
    }
}

float CuratorAnalyticsEditor::getLostnessValue(QString task)
{
    //R – Minimum number of nodes which need to be visited to complete a task
    //S – Total number of nodes visited whilst searching
    //N – Number of different nodes visited whilst searching
    // L = sqrt[(N/S – 1)² + (R/N – 1)²]

    foreach (CuratorLabel* curatorLabel, m_curatorLabels)
    {
        if(curatorLabel->id->text() == task)
        {
            qDebug() << "Minimum number of nodes: " << curatorLabel->minSteps->value();
            qDebug() << "Total number of nodes visited: " << curatorLabel->totalNumOfNodesVisited;
            qDebug() << "Number of different nodes visited: " << curatorLabel->uniqueNodesVisited.size();

            //avoid div 0 errors
            if(curatorLabel->totalNumOfNodesVisited == 0)   //no nodes visited so lostness cannot be determined
                return -1;

            float firstHalf = curatorLabel->uniqueNodesVisited.size()/curatorLabel->totalNumOfNodesVisited - 1; //(N/S – 1)²
            firstHalf *= firstHalf;

            float secondHalf = curatorLabel->minSteps->value()/curatorLabel->uniqueNodesVisited.size() - 1;   //(R/N – 1)²
            secondHalf *= secondHalf;

            float lostness = firstHalf + secondHalf;    //sqrt[(N/S – 1)² + (R/N – 1)²]
            lostness = sqrt(lostness);

            qDebug() << "Lostness decimal: " << lostness;

            lostness *= 100;    //from decimal to percentage

            qDebug() << "Lostness percentage: " << lostness;
            return lostness;
        }
    }

    qDebug() << "Error: task not found";    //will end up here if the loop executes without finding the task
    return -1;
}


float CuratorAnalyticsEditor::getSimilarityValue(QString task)
{
    foreach (CuratorLabel* curatorLabel, m_curatorLabels)
    {
        if(curatorLabel->id->text() == task)
        {
            return curatorLabel->sequenceMatcher.compareUserandPerfectSequences();
        }
    }

    qDebug() << "Error: task not found";    //will end up here if the loop executes without finding the task
    return -1;
}

QJsonArray CuratorAnalyticsEditor::readSequenceFromFile()
{
    QFile file(QFileDialog::getOpenFileName(this,
                                                     QObject::tr("Load Sequence"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty()&& !file.fileName().isNull())
    {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug()<<"Cannot open file "<<file.fileName()<<"\n";
            return QJsonArray();
        }
        QString settings = file.readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(settings.toUtf8());

        if(jsonDoc.isArray())
            return jsonDoc.array();
        else
        {
            qDebug() << "JSON file" << file.fileName() << " is invalid";
            return QJsonArray();
        }
    }
    else
    {
        qDebug() << "Load aborted by user";
        return QJsonArray();
    }
}

void CuratorAnalyticsEditor::addSequenceToAllCuratorLabels()
{
    QJsonArray events = readSequenceFromFile();

    foreach (CuratorLabel* curatorLabel, m_curatorLabels)
    {
        addSequenceToSingleCuratorLabel(curatorLabel, events);
    }
}

void CuratorAnalyticsEditor::addSequenceToSingleCuratorLabel(CuratorLabel *curatorLabel, QJsonArray events)
{
    if(!events.empty())
        return;

    QJsonArray clEvents = getSequenceRelatedToCuratorLabel(events, curatorLabel->id->text());

    if(!clEvents.empty())
    {
        curatorLabel->sequenceMatcher.loadPerfectSequence(clEvents);
        curatorLabel->sequenceStatus->setText("Sequence Loaded");
        curatorLabel->sequenceStatus->setStyleSheet("QLabel { color : green }");

        if(!m_saveFullSequenceBtn->isEnabled())
            m_saveFullSequenceBtn->setEnabled(true);
    }
}

QJsonArray CuratorAnalyticsEditor::getSequenceRelatedToCuratorLabel(QJsonArray &array, QString curatorLabel)
{
    QJsonArray newArray;
    bool addtoArray = true;

    for(QJsonArray::iterator mainArrayIt = array.begin(); mainArrayIt != array.end(); ++mainArrayIt)
    {
        if((*mainArrayIt).isObject())
        {
            QJsonObject obj = (*mainArrayIt).toObject();

            if(obj.contains("verb") && obj["verb"] == "started" && obj.contains("object") && obj["object"] == curatorLabel)
            {
                addtoArray = true;
            }
            else
                if(obj.contains("verb") && obj["verb"] == "completed" && obj.contains("object") && obj["object"] == curatorLabel)
                {
                    newArray.append(obj);
                    break;
                }

            if(addtoArray)
                newArray.append(obj);
        }
    }
    return newArray;
}

void CuratorAnalyticsEditor::saveAllPerfectSequencesToFile()
{
    QFile file(QFileDialog::getSaveFileName(this,
                                                     QObject::tr("Save Perfect Sequences"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty()&& !file.fileName().isNull())
    {
        if(file.open(QFile::WriteOnly))
        {
            QJsonDocument newJsonDoc;
            QJsonArray newJsonArray;

            foreach (CuratorLabel* curatorLabel, m_curatorLabels)
            {
                QJsonArray sequence = curatorLabel->sequenceMatcher.getPerfectSequence();
                if(!sequence.empty())
                    newJsonArray.append(sequence);
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

bool CuratorAnalyticsEditor::checkIfAnalyticsLoaded()
{
    bool loadLabels = true;
    bool loadSequences = false;

    if(!m_curatorLabels.empty())
    {
        loadLabels = false;

        foreach (CuratorLabel* curatorLabel, m_curatorLabels)
        {
            if(curatorLabel->sequenceMatcher.getPerfectSequence().empty())
            {
                loadSequences = true;
                break;
            }
        }
    }


    if(loadLabels)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("No Curator Labels Loaded");
        msgBox.setText("Do you want to load the curator labels (in-game tasks) and perfect sequences before starting analytics?");
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
        if(loadSequences)
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Some/All Perfect Sequences Not Loaded");
            msgBox.setText("Do you want to load perfect sequences for in-game tasks before starting analytics?");
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
    return true;
}
