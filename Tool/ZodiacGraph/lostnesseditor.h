#ifndef LOSTNESSEDITOR_H
#define LOSTNESSEDITOR_H

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

/*struct CuratorNarrativeDependency
{
    QString id;
    QString subtitle;
    QString screenId;
    QString elementFrom;
    QString elementTo;
};

struct CuratorLabel
{
    QList<CuratorNarrativeDependency> narrativeDependencies;
    QString screenId;
    QString elementId;
    QString textId;
    QString subtitle;
    QString targetAlpha;
    QString beginDep;
    QString CompleteDep;
    QString onProgressCompleted;
};*/

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
};

class LostnessEditor : public QDialog
{
public:
    LostnessEditor(QWidget *parent = 0);
    void loadCuratorLabels();
    void saveCuratorLabels();
    void showWindow();
    void curatorTaskStarted(QString task);

private:
    void showCuratorLabels();
    void hideCuratorLabels();

    QGridLayout *m_mainLayout;
    QList<CuratorLabel*> m_curatorLabels;

    QPushButton *m_saveBtn;
    QPushButton *m_loadBtn;

    QJsonArray m_jsonArray;
};

#endif // LOSTNESSEDITOR_H
