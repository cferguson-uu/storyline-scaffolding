#include "analyticsproperties.h"

#include <QVBoxLayout>

#include "collapsible.h"

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

    m_storyNodesWithConnections = new QLabel(kName_StoryNodesWithConnection);
    m_storyNodesWithoutConnections = new QLabel(kName_StoryNodesWithoutConnection);
    m_storyNodesAverageConnections = new QLabel(kName_StoryNodesAverageConnection);
    m_narrativeNodesWithConnections = new QLabel(kName_NarrativeNodesWithConnection);
    m_narrativeNodesWithoutConnections = new QLabel(kName_NarrativeNodesWithoutConnection);
    m_narrativeNodesAverageConnections = new QLabel(kName_NarrativeNodesAverageConnection);

    m_mainLayout->addWidget(m_storyNodesWithConnections);
    m_mainLayout->addWidget(m_storyNodesWithoutConnections);
    m_mainLayout->addWidget(m_storyNodesAverageConnections);
    m_mainLayout->addWidget(m_narrativeNodesWithConnections);
    m_mainLayout->addWidget(m_narrativeNodesWithoutConnections);
    m_mainLayout->addWidget(m_narrativeNodesAverageConnections);

    // update the title of the collapsible container
    parent->updateTitle("Analytics");
}

void AnalyticsProperties::UpdateLinkerValues()
{
    float sNodesWithConnections = 0;
    float sNodesWithoutConnections = 0;
    float sNodeAverageConnections = 0;
    float nNodesWithConnections = 0;
    float nNodesWithoutConnections = 0;
    float nNodeAverageConnections = 0;

    m_storyNodesWithConnections->setText(kName_StoryNodesWithConnection + QString::number(sNodesWithConnections));
    m_storyNodesWithoutConnections->setText(kName_StoryNodesWithoutConnection + QString::number(sNodesWithoutConnections));
    m_storyNodesAverageConnections->setText(kName_StoryNodesAverageConnection + QString::number(sNodeAverageConnections));
    m_narrativeNodesWithConnections->setText(kName_NarrativeNodesWithConnection + QString::number(nNodesWithConnections));
    m_narrativeNodesWithoutConnections->setText(kName_NarrativeNodesWithoutConnection + QString::number(nNodesWithoutConnections));
    m_narrativeNodesAverageConnections->setText(kName_NarrativeNodesAverageConnection + QString::number(nNodeAverageConnections));

}
