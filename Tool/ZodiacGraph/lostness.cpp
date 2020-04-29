#include "lostness.h"
#include <QMessageBox>

Lostness::Lostness()
{

}

float Lostness::getLostnessValue(int minSteps, int totalSteps, int uniqueSteps)
{
    /*qDebug() << "Minimum number of nodes (R): " << minSteps;
    qDebug() << "Total number of nodes visited (S): " << totalSteps;
    qDebug() << "Number of different nodes visited (N): " << uniqueSteps;
    qDebug() << "N/S: " << (float)uniqueSteps/(float)totalSteps;
    qDebug() << "R/N: " << (float)minSteps/(float)uniqueSteps;*/

    //avoid div 0 errors
    if(totalSteps == 0)   //no nodes visited so lostness cannot be determined
        return -1;

    if(minSteps == -1)   //error with pathfinding so lostness cannot be determined
        return -1;

    float firstHalf = (float)uniqueSteps/(float)totalSteps - 1; //(N/S – 1)²
    firstHalf *= firstHalf;

    float secondHalf = (float)minSteps/(float)uniqueSteps - 1;   //(R/N – 1)²
    secondHalf *= secondHalf;

    float lostness = firstHalf + secondHalf;    //sqrt[(N/S – 1)² + (R/N – 1)²]
    lostness = sqrt(lostness);

    //qDebug() << "Lostness" << lostness;
    return lostness;
}

bool Lostness::loadEdges()
{
    QFile file(QFileDialog::getOpenFileName(this,
                                                     QObject::tr("Load Edges"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty() && !file.fileName().isNull())
    {
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString settings = file.readAll();
            file.close();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(settings.toUtf8());

            if(jsonDoc.isNull() || !jsonDoc.isObject() || jsonDoc.isEmpty())
            {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
                messageBox.setFixedSize(500,200);
                return false;
            }

            QJsonObject jsonObj = jsonDoc.object();

            if(!jsonObj.contains("edges"))
            {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","Edges not found in file, please ensure that it is the correct format.");
                messageBox.setFixedSize(500,200);
                return false;
            }

            QJsonArray jsonEdgesArray = jsonObj["edges"].toArray();

            foreach (const QJsonValue &v, jsonEdgesArray)
            {
                if(!v.isObject())
                {
                    QMessageBox messageBox;
                    messageBox.critical(0,"Error","Error loading edge, please ensure that it is the correct format.");
                    messageBox.setFixedSize(500,200);
                    m_edges.clear();
                    return false;
                }

                QJsonObject jsonEdgeObject = v.toObject();

                if(!jsonEdgeObject.contains("links") || !jsonEdgeObject["links"].isArray() || (jsonEdgeObject["links"].isArray() && jsonEdgeObject["links"].toArray().count() != 2))
                {
                    QMessageBox messageBox;
                    messageBox.critical(0,"Error","Error loading edge, please ensure that it is the correct format.");
                    messageBox.setFixedSize(500,200);
                    m_edges.clear();
                    return false;
                }

                QJsonArray jsonLinksArray = jsonEdgeObject["links"].toArray();

                if(!jsonLinksArray[0].isString() || !jsonLinksArray[0].isString())
                {
                    QMessageBox messageBox;
                    messageBox.critical(0,"Error","Error loading edge, please ensure that it is the correct format.");
                    messageBox.setFixedSize(500,200);
                    m_edges.clear();
                    return false;
                }

                //QString first =  jsonLinksArray[0].toString();
                //QString second = jsonLinksArray[1].toString();
                addEdge(jsonLinksArray[0].toString(), jsonLinksArray[1].toString());
            }
            return true;
        }
        else
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","File could not be loaded. Ensure that you have the correct permissions");
            messageBox.setFixedSize(500,200);
            m_edges.clear();
            return false;
        }
    }
    else
        return false;
}

void Lostness::addEdge(QString left, QString right)
{
    if(!m_edges.contains(left))
        m_edges.insert(left, QVector<QString>());

    if(!m_edges.contains(right))
        m_edges.insert(right, QVector<QString>());

    //adj[v].push_back(w); // Add w to v’s list.QString start, QString end
    m_edges[left].push_back(right);
}

bool Lostness::loadNodes()
{
    QFile file(QFileDialog::getOpenFileName(this,
                                                     QObject::tr("Load Nodes"), "",
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
                return false;
            }

            QJsonArray jsonArray = jsonDoc.array();


            foreach (const QJsonValue &v, jsonArray)
            {
                if(!v.isObject())
                {
                    QMessageBox messageBox;
                    messageBox.critical(0,"Error","Error loading node, please ensure that it is the correct format.");
                    messageBox.setFixedSize(500,200);
                    m_nodes.clear();
                    return false;
                }

                QJsonObject jsonNodeObj = v.toObject();

                if(!jsonNodeObj.contains("name") || !jsonNodeObj.contains("type") || !jsonNodeObj["name"].isString() || !jsonNodeObj["type"].isString())
                {
                    QMessageBox messageBox;
                    messageBox.critical(0,"Error","Error loading node, please ensure that it is the correct format.");
                    messageBox.setFixedSize(500,200);
                    m_edges.clear();
                    return false;
                }

                addNode(jsonNodeObj["name"].toString(), jsonNodeObj["type"].toString());
            }
            return true;
        }
        else
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","File could not be loaded. Ensure that you have the correct permissions");
            messageBox.setFixedSize(500,200);
            m_edges.clear();
            return false;
        }
    }
    else
        return false;
}

void Lostness::addNode(QString name, QString type)
{
    if(type == "locomotion")
        m_nodes.insert(name, TYPE_LOCO);
    else
        if(type == "trigger")
            m_nodes.insert(name, TYPE_TRIGGER);
        else
            if(type == "artifact")
                m_nodes.insert(name, TYPE_ARTIFACT);
            else
                if(type == "logic")
                    m_nodes.insert(name, TYPE_LOGIC);
}

/*void Lostness::getShortestPath(QString firstNode, QString lastNode)
{
    // Mark all the vertices as not visited
    QHash<QString, bool> visited;
    foreach(const QString &key, m_edges.keys())
        visited.insert(key, false);

    // Create a queue for BFS
    QList<QString> queue;

    // Mark the first node as visited and enqueue it
    visited[firstNode] = true;
    queue.push_back(firstNode);

    // 'i' will be used to get all adjacent
    // vertices of a vertex
    QVector<QString>::iterator i;

    QString currentNode;
    qDebug() << "Calculating path from " << firstNode << "to " << lastNode;
    while(!queue.empty())
    {
        // Dequeue a vertex from queue and print it
        currentNode = queue.front();
        qDebug() << "   current node: " << currentNode;
        queue.pop_front();

        // Get all adjacent vertices of the dequeued
        // vertex s. If a adjacent has not been visited,
        // then mark it visited and enqueue it
        for (i = m_edges[currentNode].begin(); i != m_edges[currentNode].end(); ++i)
        {
            if (!visited[*i])
            {
                visited[*i] = true;
                queue.push_back(*i);
            }
        }
    }
}*/

int Lostness::shortestPath(QString start, QString end)
{
    QVector<QString> p;
    return shortestPath(start, end, p);
}

int Lostness::shortestPath(QString start, QString end, QVector<QString>& path)
        {
            QVector<_HintsSubResult> nodeQueue;
            int queueIndex = 0;
            QSet<QString> visited = QSet<QString>();

            QVector<QString> neighbors = m_edges[start];

            for (const QString &node : neighbors)
                nodeQueue.push_back(_HintsSubResult(node, 1, node, -1));

            for (queueIndex = 0; queueIndex < nodeQueue.size(); queueIndex++)
            {
                auto nodeSet = nodeQueue[queueIndex];

                if (end == nodeSet.endNode) //|| (end == -1 && isTarget(nodeSet.endNode)))
                {
                    _HintsSubResult sub = nodeSet;
                    path.clear();
                    while (sub.previous != -1)
                    {
                        if(!sub.endNode.contains("Logic"))
                            path.push_back(sub.endNode);
                        sub = nodeQueue[sub.previous];
                    }

                    path.push_back(sub.endNode);
                    //sub = nodeQueue[sub.previous];

                    //Adjust for artifact
                    //	nodeSet.length += 1;
                    return path.length();//nodeSet;
                }

                //location = end, startLoc = start
                neighbors = m_edges[nodeSet.endNode];


                //Remove visited nodes
                std::sort(neighbors.begin(), neighbors.end());
                QSet<QString> filteredNeighbors;
                //std::set_difference(neighbors.begin(), neighbors.end(), visited.begin(), visited.end(), std::inserter(filteredNeighbors, filteredNeighbors.begin()));

                foreach (const QString& val, neighbors) //very inefficient but set_difference won't compile and this may need to be used later
                {
                    if(!visited.contains(val))
                    {
                        filteredNeighbors.insert(val);
                        visited.insert(val);
                    }
                }

                /*foreach (const QString& val, filteredNeighbors)   //uncomment if loop above not used
                {
                    visited.insert(val);
                }*/

                // Skip logic and trigger nodes for calculating path length.
                /*if (m_nodes[end] == TYPE_LOGIC || m_nodes[end] == TYPE_TRIGGER)
                {
                    std::vector<_HintsSubResult> tempSubResults;
                    for (auto neighbor : filteredNeighbors)
                    {
                        //tempSubResults.push_back(_HintsSubResult(nodeSet.firstNode, nodeSet.length, neighbor, nodeSet.previous));
                    }
                    nodeQueue.insert(nodeQueue.begin() + queueIndex + 1, tempSubResults.begin(), tempSubResults.end());
                }
                else
                {*/
                    for (auto neighbor : filteredNeighbors)
                    {
                        nodeQueue.push_back(_HintsSubResult(nodeSet.firstNode, nodeSet.length + 1, neighbor, queueIndex));
                    }
                //}
            }

            return -1;// HintsSearchResult("", -1, "");
        }

float Lostness::getLostnessForObjective(const QString &startNode, const QString &endNode, int &totalSteps, const int &uniqueSteps, int &minSteps)
{
    minSteps = std::min(shortestPath(startNode, endNode), uniqueSteps);

    return getLostnessValue(minSteps, totalSteps, uniqueSteps);
}
