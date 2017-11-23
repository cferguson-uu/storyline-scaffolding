#include "lostnesseditor.h"

LostnessEditor::LostnessEditor(QWidget *parent)
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

void LostnessEditor::showWindow()
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

    show();
}

void LostnessEditor::loadCuratorLabels()
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

                        if(mainObj.contains("screen_id") && mainObj["screen_id"].isString())
                            curatorLabel->id = new QLabel(mainObj["screen_id"].toString());
                            //qDebug() << mainObj["screen_id"].toString();

                        /*if(mainObj.contains("element_id") && mainObj["element_id"].isString())
                            qDebug() << mainObj["element_id"].toString();

                        if(mainObj.contains("text_id") && mainObj["text_id"].isString())
                            qDebug() << mainObj["text_id"].toString();

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

                        if(mainObj.contains("min_steps") && mainObj["min_steps"].isString())
                            curatorLabel->minSteps = new QLineEdit(mainObj["min_steps"].toString());
                            //qDebug() << mainObj["screen_id"].toString();
                        else
                            curatorLabel->minSteps = new QLineEdit();
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

void LostnessEditor::saveCuratorLabels()
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
                            mainObj["min_steps"] = curatorLabel->minSteps->text();
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
                messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
                messageBox.setFixedSize(500,200);
        }
    }
    else
        qDebug() << "Save aborted by user";
}

void LostnessEditor::showCuratorLabels()
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
    }

    m_saveBtn->setEnabled(true);
}

void LostnessEditor::hideCuratorLabels()
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
}

void LostnessEditor::nodeVisited(QString task, QString node)
{
    foreach (CuratorLabel* curatorLabel, m_curatorLabels)
    {
        if(curatorLabel->id->text() == task)
        {
            bool found = false;

            foreach (QString visitedNode, curatorLabel->uniqueNodesVisited)
            {
                if(visitedNode == node)
                {
                    found = true;
                    break;
                }
            }

            if(!found)
                curatorLabel->uniqueNodesVisited.push_back(node);
            ++curatorLabel->totalNumOfNodesVisited;
            break;
        }
    }
}

float LostnessEditor::getLostnessValue(QString task)
{
    //R – Minimum number of nodes which need to be visited to complete a task
    //S – Total number of nodes visited whilst searching
    //N – Number of different nodes visited whilst searching
    // L = sqrt[(N/S – 1)² + (R/N – 1)²]

    foreach (CuratorLabel* curatorLabel, m_curatorLabels)
    {
        if(curatorLabel->id->text() == task)
        {
            float firstHalf = curatorLabel->uniqueNodesVisited.size()/curatorLabel->totalNumOfNodesVisited - 1; //(N/S – 1)²
            firstHalf *= firstHalf;

            float secondHalf = curatorLabel->minSteps->text().toInt()/curatorLabel->uniqueNodesVisited.size() - 1;   //(R/N – 1)²
            secondHalf *= secondHalf;

            float lostness = firstHalf + secondHalf;    //sqrt[(N/S – 1)² + (R/N – 1)²]
            lostness = sqrt(lostness);

            return lostness;
        }
    }

    qDebug() << "Error: task not found";    //will end up here if the loop executes without finding the task
    return -1;
}
