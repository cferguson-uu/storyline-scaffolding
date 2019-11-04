#ifndef LOSTNESS_H
#define LOSTNESS_H

#include <QObject>
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

struct _HintsSubResult
        {
        public:
            _HintsSubResult(){}
            _HintsSubResult(QString first, int length, QString end, int prev) : firstNode(first), length(length), endNode(end), previous(prev)
            {}
            QString firstNode;
            int length;
            QString endNode;
            int previous;
        };

        struct HintsSearchResult
        {
        public:
            HintsSearchResult(QString first, int length, QString end) : firstNode(first), length(length), endNode(end)
            {}
            HintsSearchResult(_HintsSubResult fromSubResult) : firstNode(fromSubResult.firstNode), length(fromSubResult.length), endNode(fromSubResult.endNode)
            {}
            QString firstNode;
            int length;
            QString endNode;
        };


class Lostness : public QWidget
{
public:
    Lostness();

    float getLostnessValue(int minSteps, int totalSteps, int uniqueSteps);
    float getLostnessForObjective(const QString &startNode, const QString &endNode, const int &totalSteps, const int &uniqueSteps, int &minSteps);

    void loadSpatialGraph();

private:

    void addEdge(QString left, QString right);

    //void getShortestPath(QString firstNode, QString lastNode);

    int shortestPath(QString start, QString end);
    HintsSearchResult shortestPath(QString start, QString end, QVector<QString>& path);

    QHash<QString, QVector<QString>> m_edges;
};

#endif // LOSTNESS_H
