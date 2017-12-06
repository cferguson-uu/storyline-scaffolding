#include "analyticsproperties.h"

#include <QVBoxLayout>
#include <QGridLayout>

#include "collapsible.h"
#include "curatoranalyticseditor.h"

static const QString kName_StoryNodesWithConnection = "Number of Story Nodes With Connections: ";
static const QString kName_StoryNodesWithoutConnection = "Number of Story Nodes Without Connections: ";
static const QString kName_StoryNodesAverageConnection = "Average Number of Narrative Connections: ";
static const QString kName_NarrativeNodesWithConnection = "Number of Narrative Nodes With Connections: ";
static const QString kName_NarrativeNodesWithoutConnection = "Number of Narrative Nodes Without Connections: ";
static const QString kName_NarrativeNodesAverageConnection = "Average Number of Story Connections: ";

AnalyticsProperties::AnalyticsProperties(Collapsible *parent)
: QWidget(parent)
, m_mainLayout(new QVBoxLayout(this))
{
    // define the main layout
    m_mainLayout->setContentsMargins(2,2,2,2);
    m_mainLayout->setSpacing(2);
    setLayout(m_mainLayout);

    m_storyNodesWithConnections = new QLabel(kName_StoryNodesWithConnection + QString::number(0));
    m_storyNodesWithoutConnections = new QLabel(kName_StoryNodesWithoutConnection + QString::number(0));
    m_storyNodesAverageConnections = new QLabel(kName_StoryNodesAverageConnection + QString::number(0));
    m_narrativeNodesWithConnections = new QLabel(kName_NarrativeNodesWithConnection + QString::number(0));
    m_narrativeNodesWithoutConnections = new QLabel(kName_NarrativeNodesWithoutConnection + QString::number(0));
    m_narrativeNodesAverageConnections = new QLabel(kName_NarrativeNodesAverageConnection + QString::number(0));

    m_mainLayout->addWidget(m_storyNodesWithConnections);
    m_mainLayout->addWidget(m_storyNodesWithoutConnections);
    m_mainLayout->addWidget(m_storyNodesAverageConnections);
    m_mainLayout->addWidget(m_narrativeNodesWithConnections);
    m_mainLayout->addWidget(m_narrativeNodesWithoutConnections);
    m_mainLayout->addWidget(m_narrativeNodesAverageConnections);

    // update the title of the collapsible container
    parent->updateTitle("Analytics");
}

void AnalyticsProperties::UpdateLinkerValues(QList<zodiac::NodeHandle> &nodes)
{
    float sNodesWithConnections = 0;
    float sNodesWithoutConnections = 0;
    float sNodeAverageConnections = 0;
    float nNodesWithConnections = 0;
    float nNodesWithoutConnections = 0;
    float nNodeAverageConnections = 0;
    int numOfNNodes = 0;
    int numOfSNodes = 0;

    foreach(zodiac::NodeHandle node, nodes)
    {
        if(node.getType() == zodiac::NODE_STORY)
        {
            if(node.getStoryNodeType() != zodiac::STORY_NAME && node.getStoryNodeType() != zodiac::STORY_SETTING && node.getStoryNodeType() != zodiac::STORY_SETTING_CHARACTER_GROUP &&
                    node.getStoryNodeType() != zodiac::STORY_SETTING_LOCATION_GROUP && node.getStoryNodeType() != zodiac::STORY_SETTING_TIME_GROUP && node.getStoryNodeType() != zodiac::STORY_THEME&&
                    node.getStoryNodeType() != zodiac::STORY_THEME_EVENT_GROUP && node.getStoryNodeType() != zodiac::STORY_THEME_GOAL_GROUP && node.getStoryNodeType() != zodiac::STORY_PLOT &&
                    node.getStoryNodeType() != zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP && node.getStoryNodeType() != zodiac::STORY_PLOT_EPISODE_OUTCOME_GROUP &&
                    node.getStoryNodeType() != zodiac::STORY_RESOLUTION && node.getStoryNodeType() != zodiac::STORY_RESOLUTION_EVENT_GROUP &&
                    node.getStoryNodeType() != zodiac::STORY_RESOLUTION_STATE_GROUP)
            {
                ++numOfSNodes;

                int connections = node.getPlug("narrativeIn").connectionCount();

                if(connections > 0)
                {
                    ++sNodesWithConnections;
                    sNodeAverageConnections += connections;
                }
                else
                    ++sNodesWithoutConnections;
            }
        }
        else
            if(node.getType() == zodiac::NODE_NARRATIVE)
            {
                if(node.getName() != "SEQ" && node.getName() != "INV")
                {
                    ++numOfNNodes;

                    int connections = node.getPlug("storyOut").connectionCount();

                    if(connections > 0)
                    {
                        ++nNodesWithConnections;
                        nNodeAverageConnections += connections;
                    }
                    else
                        ++nNodesWithoutConnections;
                }
            }
    }

    if(numOfSNodes > 0)
        sNodeAverageConnections /= numOfSNodes;

    if(numOfNNodes > 0)
        nNodeAverageConnections /= numOfNNodes;

    m_storyNodesWithConnections->setText(kName_StoryNodesWithConnection + QString::number(sNodesWithConnections));
    m_storyNodesWithoutConnections->setText(kName_StoryNodesWithoutConnection + QString::number(sNodesWithoutConnections));
    m_storyNodesAverageConnections->setText(kName_StoryNodesAverageConnection + QString::number(sNodeAverageConnections));
    m_narrativeNodesWithConnections->setText(kName_NarrativeNodesWithConnection + QString::number(nNodesWithConnections));
    m_narrativeNodesWithoutConnections->setText(kName_NarrativeNodesWithoutConnection + QString::number(nNodesWithoutConnections));
    m_narrativeNodesAverageConnections->setText(kName_NarrativeNodesAverageConnection + QString::number(nNodeAverageConnections));

}

void AnalyticsProperties::StartAnalyticsMode(QList<CuratorLabel*> curatorLabels)
{
    if(curatorLabels.empty())
        return;

    // define the curator layout
    m_curatorLayout = new QGridLayout();
    m_curatorLabelLayoutLabel = new QLabel("Curator Labels", this);
    m_curatorLayout->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
    m_curatorLayout->setColumnStretch(1,1); // so the add-plug button always stays on the far right
    m_curatorLayout->addWidget(m_curatorLabelLayoutLabel, 0, 0, 1, 2, Qt::AlignLeft);

    //parse the list of curator labels
    foreach (CuratorLabel *curatorLabel, curatorLabels)
    {

    }
}

void AnalyticsProperties::addCuratorLabelRow(CuratorLabel *curatorLabel)
{
    /*
    QLabel* id;
    QList<QLabel*> narrativeDependencies;
    QLineEdit* minSteps;
    SequenceMatcher sequenceMatcher;
    */

    //qMakePair<QString, bool>("", false); // for dependencies

    int row = m_curatorLayout->rowCount();

    QGridLayout *rowLayout = new QGridLayout();

    QLabel* plugName = new QLabel(curatorLabel->id->text(), this);

    rowLayout->addWidget(plugName, 0, 0);

    m_curatorLayout->addLayout(rowLayout, row, 0);

    m_curatorRows.insert(plugName->text(), new CuratorRow(this, plugName, rowLayout));
}

void AnalyticsProperties::removePlugRow(const QString& curatorLabelName)
{
    Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    m_curatorRows.remove(curatorLabelName);
}

CuratorRow::CuratorRow(AnalyticsProperties *editor, QLabel *nameLabel, QGridLayout *rowLayout)
    : QObject(editor)
    , m_nameLabel(nameLabel)
{
    int row = rowLayout->rowCount();
}
