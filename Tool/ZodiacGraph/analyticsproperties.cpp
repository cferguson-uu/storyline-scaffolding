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
static const QString kName_NumConnection = "Total Number of Narrative-Story Connections: ";

static float maxLostness = sqrt(2);

AnalyticsProperties::AnalyticsProperties(Collapsible *parent)
: QWidget(parent)
, m_pCollapsible(parent)
, m_mainLayout(new QVBoxLayout(this))
, m_curatorLayout(nullptr)
, m_curatorLabelLayoutLabel(nullptr)
, m_fullGameProgressLabel(nullptr)
, m_fullGameProgressBar(nullptr)
, m_localLostnessLabel(nullptr)
, m_localLostnessBar(nullptr)
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
    m_totalConnections = new QLabel(kName_NarrativeNodesAverageConnection + QString::number(0));

    m_mainLayout->addWidget(m_storyNodesWithConnections);
    m_mainLayout->addWidget(m_storyNodesWithoutConnections);
    m_mainLayout->addWidget(m_storyNodesAverageConnections);
    m_mainLayout->addWidget(m_narrativeNodesWithConnections);
    m_mainLayout->addWidget(m_narrativeNodesWithoutConnections);
    m_mainLayout->addWidget(m_narrativeNodesAverageConnections);
    m_mainLayout->addWidget(m_totalConnections);

    // update the title of the collapsible container
    m_pCollapsible->updateTitle("Analytics - Disconnected");
}

void AnalyticsProperties::ConnectedtoServer(QString address)
{
    m_pCollapsible->updateTitle("Analytics - Connected to: " + address);
}

void AnalyticsProperties::DisconnectedFromServer()
{
    m_pCollapsible->updateTitle("Analytics - Disconnected");
}

void AnalyticsProperties::UpdateLinkerValues(QList<zodiac::NodeHandle> &nodes)
{
    float sNodesWithConnections = 0;
    float sNodesWithoutConnections = 0;
    float sNodeAverageConnections = 0;
    float nNodesWithConnections = 0;
    float nNodesWithoutConnections = 0;
    float nNodeNumConnections = 0;
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
                        nNodeNumConnections += connections;
                    }
                    else
                        ++nNodesWithoutConnections;
                }
            }
    }

    if(numOfSNodes > 0)
        sNodeAverageConnections /= numOfSNodes;

    if(numOfNNodes > 0)
        nNodeAverageConnections = nNodeNumConnections/numOfNNodes;

    m_storyNodesWithConnections->setText(kName_StoryNodesWithConnection + QString::number(sNodesWithConnections));
    m_storyNodesWithoutConnections->setText(kName_StoryNodesWithoutConnection + QString::number(sNodesWithoutConnections));
    m_storyNodesAverageConnections->setText(kName_StoryNodesAverageConnection + QString::number(sNodeAverageConnections));
    m_narrativeNodesWithConnections->setText(kName_NarrativeNodesWithConnection + QString::number(nNodesWithConnections));
    m_narrativeNodesWithoutConnections->setText(kName_NarrativeNodesWithoutConnection + QString::number(nNodesWithoutConnections));
    m_narrativeNodesAverageConnections->setText(kName_NarrativeNodesAverageConnection + QString::number(nNodeAverageConnections));
    m_totalConnections->setText(kName_NumConnection + QString::number(nNodeNumConnections));

}

void AnalyticsProperties::StartAnalyticsMode(QList<CuratorLabel*> curatorLabels)
{
    if(curatorLabels.empty())
        return;

    // define the curator layout
    m_curatorLayout = new QGridLayout(this);
    m_fullGameProgressLayout = new QGridLayout(this);
    m_fullGameProgressLabel = new QLabel("Game Progress", this);
    m_fullGameProgressBar = new QProgressBar(this);
    m_fullGameProgressBar->setValue(0);
    m_localLostnessLabel = new QLabel("Local Lostness", this);
    m_localLostnessBar = new QProgressBar(this);
    m_localLostnessBar->setValue(0);
    m_curatorLabelLayoutLabel = new QLabel("<b>Curator Labels</b>", this);
    m_curatorLayout->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
    m_curatorLayout->setColumnStretch(1,1); // so the add-plug button always stays on the far right

    m_fullGameProgressLayout->addWidget(m_fullGameProgressLabel, 0, 0, 1, 1, Qt::AlignLeft);
    m_fullGameProgressLayout->addWidget(m_fullGameProgressBar, 0, 1, 1, 1, Qt::AlignLeft);

    m_fullGameProgressLayout->addWidget(m_localLostnessLabel, 1, 0, 1, 1, Qt::AlignLeft);
    m_fullGameProgressLayout->addWidget(m_localLostnessBar, 1, 1, 1, 1, Qt::AlignLeft);

    m_curatorLayout->addLayout(m_fullGameProgressLayout, 0, 0);
    m_curatorLayout->addWidget(m_curatorLabelLayoutLabel, 1, 0, 1, 2, Qt::AlignLeft);
    m_mainLayout->addLayout(m_curatorLayout);

    //parse the list of curator labels
    foreach (CuratorLabel *curatorLabel, curatorLabels)
    {
        addCuratorLabelRow(curatorLabel);
    }

    m_curatorLayout->setAlignment(Qt::AlignLeft);
}

void AnalyticsProperties::StopAnalyticsMode()
{
    //remove all of the curator labels, the main label and layout
    removeAllCuratorRows();

    if(m_curatorLabelLayoutLabel)
        m_curatorLabelLayoutLabel->deleteLater();

    if(m_curatorLayout)
        m_curatorLayout->deleteLater();
}

void AnalyticsProperties::addCuratorLabelRow(CuratorLabel *curatorLabel)
{
    int row = m_curatorLayout->rowCount();

    QGridLayout *rowLayout = new QGridLayout();

    QLabel* curatorLabelName = new QLabel(curatorLabel->id->text(), this);

    //Add a row for each dependency
    QGridLayout *dependencyRowLayout = new QGridLayout();
    QHash<QString, ObjectiveRow*> dependenciesList;
    foreach (CuratorObjective *dependency, curatorLabel->narrativeDependenciesList)
    {
        dependenciesList.insert(dependency->label->text(), new ObjectiveRow(this, dependency->label->text(), dependencyRowLayout));
    }

    rowLayout->addWidget(curatorLabelName, 0, 0);

    m_curatorLayout->addLayout(rowLayout, row, 0);

    m_curatorRows.insert(curatorLabelName->text(), new CuratorRow(this, curatorLabelName, rowLayout, dependencyRowLayout, dependenciesList));
}

void AnalyticsProperties::removeCuratorRow(const QString& curatorLabelName)
{
    //if it exists, delete all of the widgets and remove from the list
    Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    m_curatorRows[curatorLabelName]->removeRow();
    m_curatorRows[curatorLabelName]->deleteLater();
    m_curatorRows.remove(curatorLabelName);
}

void AnalyticsProperties::removeAllCuratorRows()
{
    for(QHash<QString, CuratorRow*>::iterator curatorIt = m_curatorRows.begin(); curatorIt != m_curatorRows.end(); ++curatorIt)
    {
        curatorIt.value()->removeRow();
        curatorIt.value()->deleteLater();
    }

    m_curatorRows.clear();  //clear list of pointers
}

bool AnalyticsProperties::getCuratorLabelStarted(QString curatorLabelName)
{
    Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    return m_curatorRows[curatorLabelName]->getStarted();
}

void AnalyticsProperties::setCuratorLabelStarted(QString curatorLabelName, bool started)
{
    Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    m_curatorRows[curatorLabelName]->setStarted(started);
}

void AnalyticsProperties::updateProgressOfCuratorLabel(QString curatorLabelName, float progress)
{
    Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    m_curatorRows[curatorLabelName]->updateProgress(progress);
}

void AnalyticsProperties::updateFullGameProgress(float fullProgress)
{
    m_fullGameProgressBar->setValue(round(fullProgress*100));
}

void AnalyticsProperties::updateLocalLostness(float lostness)
{
    m_localLostnessBar->setValue((round(lostness/maxLostness*100)));   //round to nearest integer to be shown on the bar correctly
}

void AnalyticsProperties::updateLostnessOfCuratorLabel(QString curatorLabelName, float newValue)
{
   Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    m_curatorRows[curatorLabelName]->updateLostness(newValue);
}

void AnalyticsProperties::updateLostnessOfObjective(QString curatorLabelName, QString objectiveName, float newValue)
{
   Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    m_curatorRows[curatorLabelName]->updateLostnessOfObjective(objectiveName, newValue);
}

void AnalyticsProperties::resetAllCuratorLabels()
{
    foreach (CuratorRow *curatorRow, m_curatorRows)
    {
        curatorRow->reset();
    }

    if(m_fullGameProgressBar)
        m_fullGameProgressBar->setValue(0.0f);

    if(m_localLostnessBar)
        m_localLostnessBar->setValue(0.0f);
}

CuratorRow::CuratorRow(AnalyticsProperties *editor, QLabel *nameLabel, QGridLayout *rowLayout, QGridLayout *dependencyRowLayout, QHash<QString, ObjectiveRow*> &dependenciesList)
    : QObject(editor)
    , m_rowLayout(rowLayout)
    , m_dependencyRowLayout(dependencyRowLayout)
    , m_nameLabel(nameLabel)
    , m_progressLabel(new QLabel("Progress:"))
    , m_progressBar(new QProgressBar())
    , m_dependencies(dependenciesList)
    , m_lostnessLabel(new QLabel("Lostness:"))
    , m_lostnessBar(new QProgressBar())
    , m_started(false)
    , m_completed(false)
{
    int row = m_rowLayout->rowCount();

    //add all widgets to the layout
    m_rowLayout->addWidget(m_progressLabel, row, 0, 1, 1, Qt::AlignLeft);
    m_rowLayout->addWidget(m_progressBar, row, 1, 1, 5, Qt::AlignRight);

    m_rowLayout->addWidget(m_lostnessLabel, ++row, 0, 1, 1, Qt::AlignLeft);
    m_rowLayout->addWidget(m_lostnessBar, row, 1, 1, 5, Qt::AlignRight);

    m_rowLayout->addLayout(m_dependencyRowLayout,++row, 0, 1, 5, Qt::AlignRight);

    //make labels minimum size to fit text
    m_progressLabel->setMaximumSize(m_progressLabel->fontMetrics().width(m_progressLabel->text()), m_progressLabel->fontMetrics().height());
    m_lostnessLabel->setMaximumSize(m_lostnessLabel->fontMetrics().width(m_lostnessLabel->text()), m_lostnessLabel->fontMetrics().height());

    //set values so percentage text shows on progress bars
    m_progressBar->setValue(0.0f);
    m_lostnessBar->setValue(0.0f);

    m_nameLabel->setStyleSheet("QLabel { color : red; }");  //red to show that it hasn't started
}

void CuratorRow::removeRow()
{
    m_rowLayout->deleteLater();
    m_nameLabel->deleteLater();
    m_progressLabel->deleteLater();
    m_progressBar->deleteLater();
    m_lostnessLabel->deleteLater();
    m_lostnessBar->deleteLater();

    for(QHash<QString, ObjectiveRow*>::iterator objectiveIt = m_dependencies.begin(); objectiveIt != m_dependencies.end(); ++objectiveIt)
    {
        objectiveIt.value()->removeRow();
        objectiveIt.value()->deleteLater();
    }
    m_dependencyRowLayout->deleteLater();

    m_dependencies.clear();  //clear list of pointers
}

void CuratorRow::setStarted(bool started)
{
    m_started = started;
    m_nameLabel->setStyleSheet("QLabel { color : yellow; }");
}

void CuratorRow::updateProgress(float progress)
{
    m_progressBar->setValue(round(progress*100));   //round to nearest integer to be shown on the bar correctly

}

void CuratorRow::updateLostness(float newValue)
{
    if(newValue >= 0) //don't set value if error has occurred
        m_lostnessBar->setValue((round(newValue/maxLostness*100)));   //round to nearest integer to be shown on the bar correctly
}

void CuratorRow::updateLostnessOfObjective(QString objectiveName, float newValue)
{
    if(newValue >= 0) //don't set value if error has occurred
        m_dependencies[objectiveName]->setLostness(newValue);   //round to nearest integer to be shown on the bar correctly
}

void CuratorRow::reset()
{
    m_started = false;
    m_completed = false;

    //set values back to 0
    m_progressBar->setValue(0.0f);
    m_lostnessBar->setValue(0.0f);

    //reset dependencies    
    for(QHash<QString, ObjectiveRow*>::iterator depIt = m_dependencies.begin(); depIt != m_dependencies.end(); ++depIt)
    {
        (*depIt)->reset();
    }

    m_nameLabel->setStyleSheet("QLabel { color : red; }");  //red to show that it hasn't started
}

ObjectiveRow::ObjectiveRow(QObject *object, QString name, QGridLayout *rowLayout)
    : QObject(object)
    , m_rowLayout(rowLayout)
    , m_nameLabel(new QLabel(name))
    , m_lostnessBar(new QProgressBar())
{
    int row = m_rowLayout->rowCount();

    //add all widgets to the layout
    m_rowLayout->addWidget(m_nameLabel, row, 0, 1, 1, Qt::AlignLeft);
    m_rowLayout->addWidget(m_lostnessBar, row, 1, 1, 5, Qt::AlignRight);
}
void ObjectiveRow::removeRow()
{
    m_nameLabel->deleteLater();
    m_lostnessBar->deleteLater();
}

void ObjectiveRow::setLostness(float lostness)
{
    if(lostness >= 0) //don't set value if error has occurred
        m_lostnessBar->setValue(round(lostness/maxLostness*100));   //round to nearest integer to be shown on the bar correctly
}

void ObjectiveRow::reset()
{
    //reset the lostness bar
    m_lostnessBar->setValue(0.0f);
}
