#include "linkerwindow.h"
#include "mainctrl.h"

#include <QScrollArea>

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
                m_narrativeNodeList.push_back((*nodeIt));
            }
        else
            if((*nodeIt).getType() == zodiac::NODE_STORY)
            {
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

    connect(m_saveBtn, &QPushButton::released, [=]
    {
        QList<zodiac::NodeHandle> inverseNodes;
        QList<zodiac::NodeHandle> leafNodes = narrativeTab->getCheckedNodes(inverseNodes);
        linkNarrativeNodes(m_mainNode, leafNodes, inverseNodes);
        linkStoryNodes(m_mainNode, storyTab->getCheckedNodes());
        this->hide();
    });

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

    if(m_nodeType == zodiac::NODE_NARRATIVE)
        addNarrativeCheckboxes(nodeList, x, y, nodeLayout);
    else
        if(m_nodeType == zodiac::NODE_STORY)
        {
            //find the main nodes
            zodiac::NodeHandle settingNode;
            zodiac::NodeHandle themeNode;
            zodiac::NodeHandle plotNode;
            zodiac::NodeHandle resolutionNode;
            //iterate through the list to find the nodes
            for(QList<zodiac::NodeHandle>::iterator it = nodeList.begin(); it != nodeList.end(); ++it)
            {
                if((*it).getType() == zodiac::NODE_STORY)
                {
                    if((*it).getStoryNodeType() == zodiac::STORY_SETTING)
                        settingNode = (*it); //get a pointer to the handle of the settings node
                    else
                        if((*it).getStoryNodeType() == zodiac::STORY_THEME)
                            themeNode = (*it); //get a pointer to the handle of the theme node
                        else
                            if((*it).getStoryNodeType() == zodiac::STORY_PLOT)
                                plotNode = (*it); //get a pointer to the handle of the plot node
                            else
                                if((*it).getStoryNodeType() == zodiac::STORY_RESOLUTION)
                                    resolutionNode = (*it); //get a pointer to the handle of the resolution node
              }
            }

            //only load story if it exists
            if(settingNode.isValid() && themeNode.isValid() && plotNode.isValid() && resolutionNode.isValid())
            {
                addStoryCheckboxes(settingNode, x, y, nodeLayout);
                addStoryCheckboxes(themeNode, x, y, nodeLayout);
                addStoryCheckboxes(plotNode, x, y, nodeLayout);
                addStoryCheckboxes(resolutionNode, x, y, nodeLayout);
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

void NodeTab::addStoryCheckboxes(zodiac::NodeHandle &node, int &x, int &y, QGridLayout *nodeLayout)
{
    if(!node.isNodeDecorator())
    {
        QCheckBox *checkBox = new QCheckBox(node.getStoryNodePrefix() + node.getName());
        m_checkboxes.push_back(qMakePair(checkBox, node));
        nodeLayout->addWidget(checkBox, x, y);

        ++x;

        if(x > 15)
        {
            x = 0;
            ++y;
        }
    }

    if(node.getPlug("storyOut").isValid())
    {
        QList<zodiac::PlugHandle> connectedPlugs = node.getPlug("storyOut").getConnectedPlugs();

        if(!connectedPlugs.empty())
        {
            QList<zodiac::NodeHandle> childNodes;
            for(QList<zodiac::PlugHandle>::iterator plugIt = connectedPlugs.begin(); plugIt != connectedPlugs.end(); ++plugIt)
            {
                addStoryCheckboxes((*plugIt).getNode(), x, y, nodeLayout);
            }
        }
    }
}

void NodeTab::addNarrativeCheckboxes(QList<zodiac::NodeHandle> &nodeList, int& x, int& y, QGridLayout *nodeLayout)
{
    for(QList<zodiac::NodeHandle>::iterator nodeIt = nodeList.begin(); nodeIt != nodeList.end(); ++nodeIt)
    {
        if(!(*nodeIt).isNodeDecorator())
        {
            QCheckBox *checkBox = new QCheckBox((*nodeIt).getName());

            checkBox->setTristate(true);

            m_checkboxes.push_back(qMakePair(checkBox, (*nodeIt)));
            nodeLayout->addWidget(checkBox, x, y);

            ++x;

            if(x > 12)
            {
                x = 0;
                ++y;
            }
        }
    }
}

QList<zodiac::NodeHandle> NodeTab::getCheckedNodes(QList<zodiac::NodeHandle> &inverseNodes)
{
    QList<zodiac::NodeHandle> checkedNodes;

    for(QList<QPair<QCheckBox*, zodiac::NodeHandle>>::iterator checkboxIt = m_checkboxes.begin(); checkboxIt != m_checkboxes.end(); ++checkboxIt)
    {
        if((*checkboxIt).second.isNodeDecorator())
            continue;

        if((*checkboxIt).first->checkState() == Qt::Checked)
            checkedNodes.push_back((*checkboxIt).second);
        else
            if((*checkboxIt).first->checkState() == Qt::PartiallyChecked)
                inverseNodes.push_back((*checkboxIt).second);
    }

    return checkedNodes;
}
