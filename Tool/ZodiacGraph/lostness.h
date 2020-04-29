#ifndef LOSTNESS_H
#define LOSTNESS_H

#include <QObject>
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

enum SpatialNodeType
{
    TYPE_LOCO,
    TYPE_TRIGGER,
    TYPE_ARTIFACT,
    TYPE_LOGIC
};

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
    float getLostnessForObjective(const QString &startNode, const QString &endNode, int &totalSteps, const int &uniqueSteps, int &minSteps);

    bool loadEdges();
    bool loadNodes();

    int getNumEdges(){return m_edges.count();}
    int getNumNodes(){return m_nodes.count();}

private:

    void addEdge(QString left, QString right);

    void addNode(QString name, QString type);

    //void getShortestPath(QString firstNode, QString lastNode);

    int shortestPath(QString start, QString end);
    int shortestPath(QString start, QString end, QVector<QString>& path);

    QHash<QString, QVector<QString>> m_edges;

    QHash<QString, SpatialNodeType> m_nodes;
};

#endif // LOSTNESS_H
