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

                //QString first =  jsonLinksArray[0].toString();
                //QString second = jsonLinksArray[1].toString();
                addEdge(jsonLinksArray[0].toString(), jsonLinksArray[1].toString());
            }

        }
    }
}

void Lostness::addEdge(QString left, QString right)
{
    if(!m_edges.contains(left))
        m_edges.insert(left, QVector<QString>());

    if(!m_edges.contains(right))
        m_edges.insert(right, QVector<QString>());

    //adj[v].push_back(w); // Add w to v’s list.
    m_edges[left].push_back(right);
}

void Lostness::getShortestPath(QString firstNode, QString lastNode)
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
}

/*HintsSearchResult Lostness::shortestPath(QString start, QString end, QVector<QString>& path)
        {
            QVector<_HintsSubResult> nodeQueue = QVector<_HintsSubResult>();
            int queueIndex = 0;
            QSet<QString> visited = QSet<QString>();

            QVector<QString> neighbors = m_edges[start];

            for (const QString &node : neighbors)
                nodeQueue.push_back(_HintsSubResult(node, 1, node, ""));

            for (queueIndex = 0; queueIndex < nodeQueue.size(); queueIndex++)
            {
                auto nodeSet = nodeQueue[queueIndex];

                if (end == nodeSet.endNode) //|| (end == -1 && isTarget(nodeSet.endNode)))
                {
                    _HintsSubResult sub = nodeSet;
                    path.clear();
                    while (sub.previous != -1)
                    {
                        path.push_back(sub.endNode);
                        sub = nodeQueue[sub.previous];
                    }

                    path.push_back(sub.endNode);
                    sub = nodeQueue[sub.previous];

                    //Adjust for artifact
                    //	nodeSet.length += 1;
                    return nodeSet;
                }

                //location = end, startLoc = start
                neighbors = m_edges[nodeSet.endNode];


                //Remove visited nodes
                std::sort(neighbors.begin(), neighbors.end());
                QVector<QString> filteredNeighbors;
                std::set_difference(neighbors.begin(), neighbors.end(), visited.begin(), visited.end(), std::inserter(filteredNeighbors, filteredNeighbors.begin()));

                visited.insert(filteredNeighbors.begin(), filteredNeighbors.end());

                // Skip logic and trigger nodes for calculating path length.
                if (location->get_type() == GamePlay::ESpatialNodeType::kLogic || location->get_type() == GamePlay::ESpatialNodeType::kTrigger)
                {
                    std::vector<_HintsSubResult> tempSubResults;
                    for (auto neighbor : filteredNeighbors)
                    {
                        tempSubResults.push_back(_HintsSubResult(nodeSet.firstStep, nodeSet.length, neighbor, nodeSet.previous));
                    }
                    nodeQueue.insert(nodeQueue.begin() + queueIndex + 1, tempSubResults.begin(), tempSubResults.end());
                }
                else
                {
                    for (auto neighbor : filteredNeighbors)
                    {
                        nodeQueue.push_back(_HintsSubResult(nodeSet.firstStep, nodeSet.length + 1, neighbor, queueIndex));
                    }
                }
            }

            return HintsSearchResult(-1, -1, -1);
        }*/

