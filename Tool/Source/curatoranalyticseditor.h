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
#include <QCheckBox>
#include <QRadioButton>>

#include "lostness.h"

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
    QLabel* startDependencyLabel;
    CuratorObjective* startDependency;
    QLabel* minStepsLabel;
    QSpinBox* minSteps;

    QList<QPair<QString, QString>> uniqueNodesVisited;
    int totalNumOfNodesVisited;
    int totalNumUniqueNodesVisited;

    QString startNode;
    QString endNode;

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
    void updatePath(QString object, QString verb);
    void nodeVisited(QString task, QString object, QString verb);
    float getLostnessofCuratorLabel(QString id);
    float getLostnessofCuratorLabelFromObjectives(QString id);
    bool checkIfAnalyticsLoaded();
    QList<CuratorLabel*> getCuratorLabels(){return m_curatorLabelsList;}
    void resetAll();

    void updateGameProgress();
    float getGameProgress(){return m_gameProgress;}

    float getCuratorLabelProgress(QString curatorId);

    void objectiveFound(QString objectiveId, QString curatorId, int r, int s, int n, float lostness, QString startNode, QString endNode);
    bool possibleObjectiveFound(QString objectiveId);

    float getLostnessofObjective(QString curatorId, QString objectiveId);
    float getLostnessofObjective(QString curatorId, QString objectiveId, int &r, int &s, int &n, QString &startNode, QString &endNode);

    void updateLocalLostness();

    float getLocalLostness(){return m_localLostness;}

    bool isEmpty(){return m_curatorLabelsList.empty();}

    bool getUseLostnessInTool() { if(m_useTool) return m_useTool->isChecked(); else return false;}

    QString getParentId(QString objectiveId);

    QString getSpecifiedStartNode(){return m_startNodeInput->text();}

    void setFirstNodeToStartNode(){m_firstNode = m_startNodeInput->text();}

private slots:
    void onLocalSelected(bool checked);
    void onGlobalSelected(bool checked);

private:
    void showCuratorLabels();
    void hideCuratorLabels();

    void loadSpatialGraph();

    QGridLayout *m_mainLayout;
    QHash<QString, CuratorLabel*> m_curatorLabelsHash;
    QList<CuratorLabel*> m_curatorLabelsList;

    QPushButton *m_saveCuratorBtn;
    QPushButton *m_loadCuratorBtn;

    QPushButton *m_saveEdgesBtn;
    QPushButton *m_loadEdgesBtn;

    QJsonArray m_jsonArray;

    float m_gameProgress;
    float m_localLostness;

    Lostness m_lostnessHandler;

    QCheckBox *m_useTool;

    QLabel *m_loadedLabel;
    QLabel* m_startNodeLabel;
    QLineEdit *m_startNodeInput;

    QString m_firstNode;
    QString m_endNode;
    QString m_lastLocomotionNode;
    int m_totalNodes;
    QList<QPair<QString, QString>> m_uniqueNodes;
    QRadioButton *m_useLocalLostness;
    QRadioButton *m_useGlobalLostness;
    QRadioButton *m_useNoLostness;
};

#endif // CuratorAnalyticsEditor_H
