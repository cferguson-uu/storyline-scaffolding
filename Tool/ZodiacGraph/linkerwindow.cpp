#include "linkerwindow.h"
#include "mainctrl.h"

#include <QScrollArea>

LinkerWindow::LinkerWindow(zodiac::NodeHandle &node, QList<zodiac::NodeHandle> &nodeList, MainCtrl *controller,
                           void (MainCtrl::*linkNarrative) (zodiac::NodeHandle&, QList<zodiac::NodeHandle>&),
                           void (MainCtrl::*linkStory) (zodiac::NodeHandle&, QList<zodiac::NodeHandle>&), QWidget *parent)
    : QDialog(parent)
    , m_mainNode(node)
    , m_pController(controller)
    , m_pLinkNarrative(linkNarrative)
    , m_pLinkStory(linkStory)
{
    //separate both types of nodes
    for(QList<zodiac::NodeHandle>::iterator nodeIt = nodeList.begin(); nodeIt != nodeList.end(); ++nodeIt)
    {
        if((*nodeIt).getName() != node.getName())
        {
            if((*nodeIt).getType() == zodiac::NODE_NARRATIVE)
            {
                if((*nodeIt).getName() != "SEQ" && (*nodeIt).getName() != "SEQ")
                    m_narrativeNodeList.push_back((*nodeIt));
            }
            else
                if((*nodeIt).getType() == zodiac::NODE_STORY)
                {
                    if((*nodeIt).getStoryNodeType() != zodiac::STORY_NAME && (*nodeIt).getStoryNodeType() != zodiac::STORY_SETTING && (*nodeIt).getStoryNodeType() != zodiac::STORY_SETTING_CHARACTER_GROUP &&
                            (*nodeIt).getStoryNodeType() != zodiac::STORY_SETTING_LOCATION_GROUP && (*nodeIt).getStoryNodeType() != zodiac::STORY_SETTING_TIME_GROUP && (*nodeIt).getStoryNodeType() != zodiac::STORY_THEME&&
                            (*nodeIt).getStoryNodeType() != zodiac::STORY_THEME_EVENT_GROUP && (*nodeIt).getStoryNodeType() != zodiac::STORY_THEME_GOAL_GROUP && (*nodeIt).getStoryNodeType() != zodiac::STORY_PLOT &&
                            (*nodeIt).getStoryNodeType() != zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP && (*nodeIt).getStoryNodeType() != zodiac::STORY_PLOT_EPISODE_OUTCOME_GROUP &&
                            (*nodeIt).getStoryNodeType() != zodiac::STORY_RESOLUTION && (*nodeIt).getStoryNodeType() != zodiac::STORY_RESOLUTION_EVENT_GROUP &&
                            (*nodeIt).getStoryNodeType() != zodiac::STORY_RESOLUTION_STATE_GROUP)
                    m_storyNodeList.push_back((*nodeIt));
                }
        }
    }

    m_tabWidget = new QTabWidget;

    NodeTab* narrativeTab = new NodeTab(m_narrativeNodeList, zodiac::NODE_NARRATIVE, parent);
    NodeTab* storyTab = new NodeTab(m_storyNodeList, zodiac::NODE_STORY, parent);
    m_tabWidget->addTab(narrativeTab, tr("Narrative"));
    m_tabWidget->addTab(storyTab, tr("Story"));

    m_saveBtn = new QPushButton("Save");
    m_cancelBtn = new QPushButton("Cancel");

    connect(m_saveBtn, &QPushButton::released, [=]{m_pController->linkNarrativeNodes(m_mainNode, narrativeTab->getSelectedNodes()); m_pController->linkStoryNodes(m_mainNode, storyTab->getSelectedNodes()); this->hide();});
    connect(m_cancelBtn, &QPushButton::released, [=]{this->hide();});

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addWidget(m_saveBtn);
    mainLayout->addWidget(m_cancelBtn);
    setLayout(mainLayout);

    setWindowTitle(tr("Link Nodes to ") + node.getName());
}

NodeTab::NodeTab(QList<zodiac::NodeHandle> nodeList, zodiac::NodeType nodeType, QWidget *parent)
    : QWidget(parent)
    , m_nodeList(nodeList)
    , m_nodeType(nodeType)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QScrollArea *scrollArea = new QScrollArea;
    QGridLayout *nodeLayout = new QGridLayout;

    int x = 0;
    int y = 0;

    for(QList<zodiac::NodeHandle>::iterator nodeIt = nodeList.begin(); nodeIt != nodeList.end(); ++nodeIt)
    {
        QCheckBox *checkBox = new QCheckBox((*nodeIt).getName());
        m_checkboxes.push_back(checkBox);
        nodeLayout->addWidget(checkBox, x, y);

        ++x;

        if(x > 12)
        {
            x = 0;
            ++y;
        }
    }

    nodeLayout->setSizeConstraint(QLayout::SetFixedSize);
    QWidget *widg = new QWidget();
    widg->setLayout(nodeLayout);
    scrollArea->setWidget(widg);

    nodeLayout->setAlignment(Qt::AlignTop);

    mainLayout->addWidget(scrollArea);
    setLayout(mainLayout);


}

QList<zodiac::NodeHandle> NodeTab::getSelectedNodes()
{
    QList<zodiac::NodeHandle> checkedNodes;

    for(QList<QCheckBox*>::iterator checkboxIt = m_checkboxes.begin(); checkboxIt != m_checkboxes.end(); ++checkboxIt)
    {
        if((*checkboxIt)->isChecked())
            for(QList<zodiac::NodeHandle>::iterator nodeIt = m_nodeList.begin(); nodeIt != m_nodeList.end(); ++nodeIt)
            {
                if((*nodeIt).getName() == (*checkboxIt)->text())
                {
                    checkedNodes.push_back((*nodeIt));
                }
            }
    }

    return checkedNodes;
}
