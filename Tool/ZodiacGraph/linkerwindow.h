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
    explicit LinkerWindow(zodiac::NodeHandle &node, QList<zodiac::NodeHandle> &nodeList, MainCtrl *controller,
                          void (MainCtrl::*linkNarrative) (zodiac::NodeHandle&, QList<zodiac::NodeHandle>&),
                          void (MainCtrl::*linkStory) (zodiac::NodeHandle&, QList<zodiac::NodeHandle>&), QWidget *parent = 0);

private:
    zodiac::NodeHandle m_mainNode;
    QList<zodiac::NodeHandle> m_narrativeNodeList;
    QList<zodiac::NodeHandle> m_storyNodeList;

    QTabWidget *m_tabWidget;

    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;

    MainCtrl *m_pController;
    void (MainCtrl::*m_pLinkNarrative) (zodiac::NodeHandle&, QList<zodiac::NodeHandle>&);
    void (MainCtrl::*m_pLinkStory) (zodiac::NodeHandle&, QList<zodiac::NodeHandle>&);
};

class NodeTab : public QWidget
{
    Q_OBJECT

public:
    explicit NodeTab(QList<zodiac::NodeHandle>, zodiac::NodeType, QWidget *parent = 0);

    QList<zodiac::NodeHandle> getSelectedNodes();

private:
    QList<zodiac::NodeHandle> m_nodeList;
    zodiac::NodeType m_nodeType;
    QList<QCheckBox*> m_checkboxes;
};

#endif // LINKERWINDOW_H
