#include "lostness.h"

Lostness::Lostness()
{

}

float Lostness::getLostnessValue(int minSteps, int totalSteps, int uniqueSteps)
{
    qDebug() << "Minimum number of nodes (R): " << minSteps;
    qDebug() << "Total number of nodes visited (S): " << totalSteps;
    qDebug() << "Number of different nodes visited (N): " << uniqueSteps;
    qDebug() << "N/S: " << (float)uniqueSteps/(float)totalSteps;
    qDebug() << "R/N: " << (float)minSteps/(float)uniqueSteps;

    //avoid div 0 errors
    if(totalSteps == 0)   //no nodes visited so lostness cannot be determined
        return -1;

    float firstHalf = (float)uniqueSteps/(float)totalSteps - 1; //(N/S – 1)²
    firstHalf *= firstHalf;

    float secondHalf = (float)minSteps/(float)uniqueSteps - 1;   //(R/N – 1)²
    secondHalf *= secondHalf;

    float lostness = firstHalf + secondHalf;    //sqrt[(N/S – 1)² + (R/N – 1)²]
    lostness = sqrt(lostness);

    qDebug() << "Lostness" << lostness;
    return lostness;
}

void Lostness::loadSpatialGraph()
{
    QFile file(QFileDialog::getOpenFileName(this,
                                                     QObject::tr("Load Spatial Graph"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty() && !file.fileName().isNull())
    {
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString settings = file.readAll();
            file.close();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(settings.toUtf8());
            QJsonObject jsonObj = jsonDoc.object();
            QJsonArray jsonEdgesArray = jsonObj["edges"].toArray();

            foreach (const QJsonValue &v, jsonEdgesArray)
            {
                QJsonArray jsonLinksArray = v.toArray();

                QString first =  jsonLinksArray[0].toString();
                QString second = jsonLinksArray[1].toString();

                //if node zero not in vector/whatever, add it
                // if node one not in vector/whatever, add it
                //add node one to node 0 node list
                //Breadth-first search - https://www.geeksforgeeks.org/breadth-first-search-or-bfs-for-a-graph/
            }

        }
    }
}
