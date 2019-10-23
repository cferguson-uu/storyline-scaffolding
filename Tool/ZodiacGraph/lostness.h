#ifndef LOSTNESS_H
#define LOSTNESS_H

#include <QObject>
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>


class Lostness : public QWidget
{
public:
    Lostness();

    float getLostnessValue(int minSteps, int totalSteps, int uniqueSteps);

    void loadSpatialGraph();
};

#endif // LOSTNESS_H
