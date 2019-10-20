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
