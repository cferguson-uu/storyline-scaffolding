#ifndef NARRATIVEFILESORTER_H
#define NARRATIVEFILESORTER_H

#include <QDialog>
#include <QWidget>
#include <QListWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>

#include <zodiacgraph/nodehandle.h>

namespace Ui {
class NarrativeFileSorter;
}

class NarrativeFileSorter : public QDialog
{
    Q_OBJECT

public:
    explicit NarrativeFileSorter(QWidget *parent = 0);
    void showWindow(QVector<QString> fileNames);
    QVector<QString> getOrderedList(){return m_orderedList;}

signals:
    void loadOrderedNarrative(QVector<QString>);

private:
    QGridLayout *m_mainLayout;
    QListWidget *m_listWidget;
    QPushButton *m_okBtn;
    QPushButton *m_cancelBtn;
    Ui::NarrativeFileSorter *ui;

    void exportSortedList();

    QVector<QString> m_orderedList;

};

#endif // NARRATIVEFILESORTER_H
