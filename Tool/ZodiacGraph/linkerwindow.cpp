#include "linkerwindow.h"

#include <QDebug>

LinkerWindow::LinkerWindow(zodiac::NodeHandle &node, QList<zodiac::NodeHandle> &nodeList, QWidget *parent)
    : QDialog(parent)
    , m_mainNode(node)
{
    //separate both types of nodes
    for(QList<zodiac::NodeHandle>::iterator nodeIt = nodeList.begin(); nodeIt != nodeList.end(); ++nodeIt)
    {
        if((*nodeIt).getName() != node.getName())
        {
            if((*nodeIt).getType() == zodiac::NODE_NARRATIVE)
            {
                qDebug() << "narrative:" << (*nodeIt).getName();
                m_narrativeNodeList.push_back((*nodeIt));
            }
            else
                if((*nodeIt).getType() == zodiac::NODE_STORY)
                {
                    qDebug() << "story:" << (*nodeIt).getName();
                    m_storyNodeList.push_back((*nodeIt));
                }
        }
    }

    m_tabWidget = new QTabWidget;
    m_tabWidget->addTab(new NodeTab(m_narrativeNodeList, zodiac::NODE_NARRATIVE, parent), tr("Narrative"));
    m_tabWidget->addTab(new NodeTab(m_storyNodeList, zodiac::NODE_STORY, parent), tr("Story"));

    m_saveBtn = new QPushButton("Save");
    m_cancelBtn = new QPushButton("Cancel");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addWidget(m_saveBtn);
    mainLayout->addWidget(m_cancelBtn);
    setLayout(mainLayout);

    setWindowTitle(tr("Link Nodes"));
}

NodeTab::NodeTab(QList<zodiac::NodeHandle> nodeList, zodiac::NodeType nodeType, QWidget *parent)
    : QWidget(parent)
    , m_nodeList(nodeList)
    , m_nodeType(nodeType)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    for(QList<zodiac::NodeHandle>::iterator nodeIt = nodeList.begin(); nodeIt != nodeList.end(); ++nodeIt)
    {
        QCheckBox *checkBox = new QCheckBox((*nodeIt).getName());
        checkboxes.push_back(checkBox);
        mainLayout->addWidget(checkBox);
    }

    setLayout(mainLayout);
}
