#ifndef LINKERWINDOW_H
#define LINKERWINDOW_H

#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QCheckBox>
#include <QPushButton>

#include <zodiacgraph/nodehandle.h>

class MainCtrl;

class LinkerWindow : public QDialog
{
    Q_OBJECT
public:
    explicit LinkerWindow(zodiac::NodeHandle &node, QList<zodiac::NodeHandle> &nodeList, QWidget *parent = 0);

signals:
    void linkNarrativeNodes(zodiac::NodeHandle &node, QList<zodiac::NodeHandle> &nodeList, QList<zodiac::NodeHandle> &inverseNodeList);
    void linkStoryNodes(zodiac::NodeHandle &node, QList<zodiac::NodeHandle> &nodeList);

private:
    zodiac::NodeHandle m_mainNode;
    QList<zodiac::NodeHandle> m_narrativeNodeList;
    QList<zodiac::NodeHandle> m_storyNodeList;

    QTabWidget *m_tabWidget;

    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
};

class NodeTab : public QWidget
{
    Q_OBJECT

public:
    explicit NodeTab(QList<zodiac::NodeHandle>, zodiac::NodeType, QWidget *parent = 0);

    QList<zodiac::NodeHandle> getCheckedNodes(QList<zodiac::NodeHandle> &inverseNodes = QList<zodiac::NodeHandle>());

private:
    QList<zodiac::NodeHandle> m_nodeList;
    zodiac::NodeType m_nodeType;
    QList<QCheckBox*> m_checkboxes;
};

#endif // LINKERWINDOW_H
