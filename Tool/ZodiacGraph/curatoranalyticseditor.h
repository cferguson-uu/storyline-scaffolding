#ifndef CURATORANALYTICSEDITOR_H
#define CURATORANALYTICSEDITOR_H

#include <QDialog>
#include <QGridLayout>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QSpinBox>

#include "sequencematcher.h"

struct CuratorObjective
{
    CuratorObjective(QString objectiveName)
    {
        label = new QLabel(objectiveName);
        found = false;
    }

    int minSteps;
    int totalNumOfNodesVisited;
    int totalNumUniqueNodesVisited;
    float lostness;
    QString startNode;
    QString endNode;
    QLabel* label;
    bool found;
};

struct CuratorLabel
{
    ~CuratorLabel()
    {
        id->deleteLater();
        dependenciesLabel->deleteLater();
        minSteps->deleteLater();

        foreach (CuratorObjective *dep, narrativeDependencies)
        {
            dep->label->deleteLater();
        }
    }

    QLabel* id;
    QLabel* dependenciesLabel;
    QHash<QString, CuratorObjective*> narrativeDependencies;
    QLabel* minStepsLabel;
    QSpinBox* minSteps;

    QList<QPair<QString, QString>> uniqueNodesVisited;
    int totalNumOfNodesVisited;

    QPushButton *addSequenceBtn;
    QLabel *sequenceStatus;
    SequenceMatcher sequenceMatcher;

    float progress;
    float lostness;
};

class CuratorAnalyticsEditor : public QDialog
{
public:
    CuratorAnalyticsEditor(QWidget *parent = 0);
    void loadCuratorLabels();
    void saveCuratorLabels();
    void showWindow();
    void nodeVisited(QString task, QJsonObject event);
    float getLostnessofCuratorLabel(QString task);
    float getSimilarityValue(QString task);
    QVector<QJsonArray> readSequencesFromFiles();
    void addSequenceToAllCuratorLabels();
    void addSequencesToSingleCuratorLabel(CuratorLabel *curatorLabel, QVector<QJsonArray> eventsList);
    QJsonArray getSequenceRelatedToCuratorLabel(QJsonArray &array, QString curatorLabel);
    void saveAllPerfectSequencesToFile();
    bool checkIfAnalyticsLoaded();
    QList<CuratorLabel*> getCuratorLabels();
    void resetAll();

    void updateGameProgress();
    float getGameProgress(){return m_gameProgress;}

    float getCuratorLabelProgress(QString curatorId);

    void objectiveFound(QString objectiveId, QString curatorID, int r, int s, int n, float lostness, QString startNode, QString endNode);
    void possibleObjectiveFound(QString objectiveId);

    float getLostnessofObjective(QString curatorId, QString objectiveId);

    void updateLocalLostness();

    float getLocalLostness(){return m_localLostness;}

    bool isEmpty(){return m_curatorLabels.empty();}

private:
    void showCuratorLabels();
    void hideCuratorLabels();

    QGridLayout *m_mainLayout;
    QHash<QString, CuratorLabel*> m_curatorLabels;

    QPushButton *m_saveBtn;
    QPushButton *m_loadBtn;
    QPushButton *m_loadFullSequenceBtn;
    QPushButton *m_saveFullSequenceBtn;

    QJsonArray m_jsonArray;

    float m_gameProgress;
    float m_localLostness;
};

#endif // CuratorAnalyticsEditor_H
