#ifndef LOSTNESS_H
#define LOSTNESS_H

#include <QDebug>


class Lostness
{
public:
    Lostness();

    float getLostnessValue(int minSteps, int totalSteps, int uniqueSteps);
};

#endif // LOSTNESS_H
