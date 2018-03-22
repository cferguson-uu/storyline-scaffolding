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

AnalyticsProperties::AnalyticsProperties(Collapsible *parent)
: QWidget(parent)
, m_mainLayout(new QVBoxLayout(this))
, m_curatorLayout(nullptr)
, m_curatorLabelLayoutLabel(nullptr)
, m_fullGameProgressLabel(nullptr)
, m_fullGameProgressBar(nullptr)
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
    parent->updateTitle("Analytics");
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
    m_curatorLabelLayoutLabel = new QLabel("<b>Curator Labels</b>", this);
    m_curatorLayout->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
    m_curatorLayout->setColumnStretch(1,1); // so the add-plug button always stays on the far right

    m_fullGameProgressLayout->addWidget(m_fullGameProgressLabel, 0, 0, 1, 1, Qt::AlignLeft);
    m_fullGameProgressLayout->addWidget(m_fullGameProgressBar, 0, 1, 1, 1, Qt::AlignLeft);

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

    QLabel* plugName = new QLabel(curatorLabel->id->text(), this);

    // add names of all dependencies, set to false as not accessed yet
    QHash<QString, bool> dependenciesList;
    foreach (QLabel* dependency, curatorLabel->narrativeDependencies)
    {
        dependenciesList[dependency->text()] = false;
    }

    rowLayout->addWidget(plugName, 0, 0);

    m_curatorLayout->addLayout(rowLayout, row, 0);

    m_curatorRows.insert(plugName->text(), new CuratorRow(this, plugName, rowLayout, dependenciesList));
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
    if(m_curatorRows.isEmpty())
        return false;

    Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    return m_curatorRows[curatorLabelName]->getStarted();
}

void AnalyticsProperties::setCuratorLabelStarted(QString curatorLabelName, bool started)
{
    if(m_curatorRows.isEmpty())
        return;

   Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    m_curatorRows[curatorLabelName]->setStarted(started);

}

void AnalyticsProperties::updateProgressOfCuratorLabel(QString curatorLabelName, QString dependencyName)
{
    if(m_curatorRows.isEmpty())
        return;

   Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    m_curatorRows[curatorLabelName]->updateProgress(dependencyName);
    updateFullGameProgress();
}

void AnalyticsProperties::updateFullGameProgress()
{
    float averageProgress = 0;

    foreach (CuratorRow *curatorRow, m_curatorRows)
    {
        averageProgress += curatorRow->getProgress();
    }

    averageProgress /= m_curatorRows.size();

    m_fullGameProgressBar->setValue(averageProgress);
}

void AnalyticsProperties::updateLostnessOfCuratorLabel(QString curatorLabelName, float newValue)
{
    if(m_curatorRows.isEmpty())
        return;

   Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    m_curatorRows[curatorLabelName]->updateLostness(newValue);
}

void AnalyticsProperties::updateSimilarityOfCuratorLabel(QString curatorLabelName, float newValue)
{
    if(m_curatorRows.isEmpty())
        return;

   Q_ASSERT(m_curatorRows.contains(curatorLabelName));
    m_curatorRows[curatorLabelName]->updateSimilarity(newValue);
}

void AnalyticsProperties::resetAllCuratorLabels()
{
    foreach (CuratorRow *curatorRow, m_curatorRows)
    {
        curatorRow->reset();
    }

    if(m_fullGameProgressBar)
        m_fullGameProgressBar->setValue(0.0f);
}

CuratorRow::CuratorRow(AnalyticsProperties *editor, QLabel *nameLabel, QGridLayout *rowLayout, QHash<QString, bool> &dependenciesList)
    : QObject(editor)
    , m_rowLayout(rowLayout)
    , m_nameLabel(nameLabel)
    , m_progressLabel(new QLabel("Progress:"))
    , m_progressBar(new QProgressBar())
    , m_dependencies(dependenciesList)
    , m_lostnessLabel(new QLabel("Lostness:"))
    , m_lostnessBar(new QProgressBar())
    , m_similarityLabel(new QLabel("Similarity:"))
    , m_similarityBar(new QProgressBar())
    , m_started(false)
    , m_completed(false)
{
    int row = m_rowLayout->rowCount();

    //add all widgets to the layout
    m_rowLayout->addWidget(m_progressLabel, row, 0, 1, 1, Qt::AlignLeft);
    m_rowLayout->addWidget(m_progressBar, row, 1, 1, 5, Qt::AlignRight);

    m_rowLayout->addWidget(m_lostnessLabel, ++row, 0, 1, 1, Qt::AlignLeft);
    m_rowLayout->addWidget(m_lostnessBar, row, 1, 1, 5, Qt::AlignRight);

    m_rowLayout->addWidget(m_similarityLabel, ++row, 0, 1, 1, Qt::AlignLeft);
    m_rowLayout->addWidget(m_similarityBar, row, 1, 1, 5, Qt::AlignRight);

    //make labels minimum size to fit text
    m_progressLabel->setMaximumSize(m_progressLabel->fontMetrics().width(m_progressLabel->text()), m_progressLabel->fontMetrics().height());
    m_lostnessLabel->setMaximumSize(m_lostnessLabel->fontMetrics().width(m_lostnessLabel->text()), m_lostnessLabel->fontMetrics().height());
    m_similarityLabel->setMaximumSize(m_similarityLabel->fontMetrics().width(m_similarityLabel->text()), m_similarityLabel->fontMetrics().height());

    //set values so percentage text shows on progress bars
    m_progressBar->setValue(0.0f);
    m_lostnessBar->setValue(0.0f);
    m_similarityBar->setValue(0.0f);

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
    m_similarityLabel->deleteLater();
    m_similarityBar->deleteLater();
}

void CuratorRow::setStarted(bool started)
{
    m_started = started;
    m_nameLabel->setStyleSheet("QLabel { color : yellow; }");
}

void CuratorRow::updateProgress(QString dependencyName)
{
    if(m_dependencies.contains(dependencyName))
    {
        m_dependencies[dependencyName] = true;

        float progress(0);

        for(QHash<QString,bool>::iterator depIt = m_dependencies.begin(); depIt != m_dependencies.end(); ++depIt)
        {
            if(depIt.value())   //if dependency has been found, increase progress
                ++progress;
        }

        progress = progress/m_dependencies.size() * 100;    //get progress and update bar
        m_progressBar->setValue(progress);

        if(progress == 100) //change colour to green to show completed
            m_nameLabel->setStyleSheet("QLabel { color : green; }");
    }
}


int CuratorRow::getProgress()
{
    return m_progressBar->value();
}

void CuratorRow::updateLostness(float newValue)
{
    if(newValue >= 0) //don't set value if error has occurred
        m_lostnessBar->setValue(newValue);
}

void CuratorRow::updateSimilarity(float newValue)
{
    if(newValue >= 0) //don't set value if error has occurred
        m_similarityBar->setValue(newValue);
}

void CuratorRow::reset()
{
    //set values back to 0
    m_progressBar->setValue(0.0f);
    m_lostnessBar->setValue(0.0f);
    m_similarityBar->setValue(0.0f);

    m_nameLabel->setStyleSheet("QLabel { color : red; }");  //red to show that it hasn't started
}
