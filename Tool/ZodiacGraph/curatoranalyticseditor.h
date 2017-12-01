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

#include "sequencematcher.h"

struct CuratorLabel
{
    ~CuratorLabel()
    {
        id->deleteLater();
        dependenciesLabel->deleteLater();
        minSteps->deleteLater();

        foreach (QLabel* label, narrativeDependencies)
            label->deleteLater();
    }

    QLabel* id;
    QLabel* dependenciesLabel;
    QList<QLabel*> narrativeDependencies;
    QLabel* minStepsLabel;
    QLineEdit* minSteps;

    QList<QString> uniqueNodesVisited;
    int totalNumOfNodesVisited;

    QPushButton *addSequenceBtn;
    QLabel *sequenceStatus;
    SequenceMatcher sequenceMatcher;
};

class CuratorAnalyticsEditor : public QDialog
{
public:
    CuratorAnalyticsEditor(QWidget *parent = 0);
    void loadCuratorLabels();
    void saveCuratorLabels();
    void showWindow();
    void nodeVisited(QString task, QJsonObject event);
    float getLostnessValue(QString task);
    QJsonArray readSequenceFromFile();
    void addSequenceToAllCuratorLabels();
    void addSequenceToSingleCuratorLabel(CuratorLabel *curatorLabel, QJsonArray events);
    QJsonArray getSequenceRelatedToCuratorLabel(QJsonArray &array, QString curatorLabel);
    void saveAllPerfectSequencesToFile();

private:
    void showCuratorLabels();
    void hideCuratorLabels();

    QGridLayout *m_mainLayout;
    QList<CuratorLabel*> m_curatorLabels;

    QPushButton *m_saveBtn;
    QPushButton *m_loadBtn;
    QPushButton *m_loadFullSequenceBtn;
    QPushButton *m_saveFullSequenceBtn;

    QJsonArray m_jsonArray;
};

#endif // CuratorAnalyticsEditor_H