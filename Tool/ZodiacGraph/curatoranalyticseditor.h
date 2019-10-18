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

        foreach (CuratorObjective *dep, narrativeDependenciesList)
        {
            dep->label->deleteLater();
        }
    }

    QLabel* id;
    QLabel* dependenciesLabel;
    QHash<QString, CuratorObjective*> narrativeDependenciesHash;
    QList<CuratorObjective*> narrativeDependenciesList;
    QLabel* minStepsLabel;
    QSpinBox* minSteps;

    QList<QPair<QString, QString>> uniqueNodesVisited;
    int totalNumOfNodesVisited;

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
    bool checkIfAnalyticsLoaded();
    QList<CuratorLabel*> getCuratorLabels(){return m_curatorLabelsList;}
    void resetAll();

    void updateGameProgress();
    float getGameProgress(){return m_gameProgress;}

    float getCuratorLabelProgress(QString curatorId);

    void objectiveFound(QString objectiveId, QString curatorID, int r, int s, int n, float lostness, QString startNode, QString endNode);
    void possibleObjectiveFound(QString objectiveId);

    float getLostnessofObjective(QString curatorId, QString objectiveId);

    void updateLocalLostness();

    float getLocalLostness(){return m_localLostness;}

    bool isEmpty(){return m_curatorLabelsList.empty();}

private:
    void showCuratorLabels();
    void hideCuratorLabels();

    QGridLayout *m_mainLayout;
    QHash<QString, CuratorLabel*> m_curatorLabelsHash;
    QList<CuratorLabel*> m_curatorLabelsList;

    QPushButton *m_saveBtn;
    QPushButton *m_loadBtn;

    QJsonArray m_jsonArray;

    float m_gameProgress;
    float m_localLostness;
};

#endif // CuratorAnalyticsEditor_H
