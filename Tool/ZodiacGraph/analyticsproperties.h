#ifndef ANALYTICSPROPERTIES_H
#define ANALYTICSPROPERTIES_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include "zodiacgraph/nodehandle.h"
#include <QProgressBar>

class Collapsible;
class QVBoxLayout;
class QGridLayout;
struct CuratorLabel;
struct CuratorObjective;

///
/// \brief Analytics Property widget, is a display widget of a Collapsible. Used for showing analytics info
///
class AnalyticsProperties : public QWidget
{
    Q_OBJECT
    ///
    /// \brief The CuratorRow class is a logical part of the this one, but has a 1-n relationship.
    ///
    friend class CuratorRow;

public:// methods

    ///
    /// \brief Constructor.
    ///
    /// \param [in] parent  Collapsible parent object.
    ///
    AnalyticsProperties(Collapsible *parent);

    ///
    /// \brief Show all of the properties for in-game analytics
    ///
    void StartAnalyticsMode(QList<CuratorLabel*> curatorLabels);

    ///
    /// \brief Remove all of the properties for in-game analytics
    ///
    void StopAnalyticsMode();

    ///
    /// \brief Update total progress of the game
    ///
    void updateFullGameProgress(float fullProgress);

    ///
    /// \brief Update local lostness of the game
    ///
    void updateLocalLostness(float lostness);

    ///
    /// \brief Remove all curator rows from being displayed
    ///
    void removeAllCuratorRows();

    ///
    /// \brief Get the started status of a curator label
    ///
    bool getCuratorLabelStarted(QString curatorLabelName);

    ///
    /// \brief Set the started status of a curator label
    ///
    void setCuratorLabelStarted(QString curatorLabelName, bool started);

    ///
    /// \brief Update the progress of a curator label row
    ///
    void updateProgressOfCuratorLabel(QString curatorLabelName, float progress);

    ///
    /// \brief Update the lostness of a curator label row
    ///
    void updateLostnessOfCuratorLabel(QString curatorLabelName, float newValue);

    ///
    /// \brief Update the similarity of a curator label row
    ///
    void updateSimilarityOfCuratorLabel(QString curatorLabelName, float newValue);

    ///
    /// \brief Update the lostness of a curator label row
    ///
    void updateLostnessOfObjective(QString curatorLabelName, QString objectiveName, float newValue);

    ///
    /// \brief Reset progress, lostness and similarity of all labels
    ///
    void resetAllCuratorLabels();

public slots:
    ///
    /// \brief Updates the values for links between story and narrative nodes
    ///
    /// \param [in] nodes          List of nodes used to do calculations
    ///
    void UpdateLinkerValues(QList<zodiac::NodeHandle> &nodes);

private:
    ///
    /// \brief The layout to be used by CuratorRows to place their widgets.
    ///
    /// \return The layout of the widgets related to the plugs of the node.
    ///
    QGridLayout* getCuratorLayout() const {return m_curatorLayout;}

    ///
    /// \brief Removes a  CuratorRow from the editor.
    ///
    /// \param [in] curatorLabelName    Name of the curator label to remove.
    ///
    void removeCuratorRow(const QString& curatorLabelName);

    ///
    /// \brief Main Layout
    ///
    QVBoxLayout* m_mainLayout;

    ///
    /// \brief Number of story nodes that have narrative node connections
    ///
    QLabel *m_storyNodesWithConnections;

    ///
    /// \brief Number of story nodes that don't have narrative node connections
    ///
    QLabel *m_storyNodesWithoutConnections;

    ///
    /// \brief Average number of narrative node connections per story node
    ///
    QLabel *m_storyNodesAverageConnections;

    ///
    /// \brief Number of narrative nodes that have narrative node connections
    ///
    QLabel *m_narrativeNodesWithConnections;

    ///
    /// \brief Number of narrative nodes that don't have narrative node connections
    ///
    QLabel *m_narrativeNodesWithoutConnections;

    ///
    /// \brief Average number of story node connections per narrative node
    ///
    QLabel *m_narrativeNodesAverageConnections;

    ///
    /// \brief Total number of story-narrative connections
    ///
    QLabel *m_totalConnections;

    ///
    /// \brief Layout of the widgets related to the curator labels.
    ///
    QGridLayout* m_curatorLayout;

    ///
    /// \brief Label for the curator label section
    ///
    QLabel *m_curatorLabelLayoutLabel;

    ///
    /// \brief All CuratorRows contained in this editor.
    ///
    QHash<QString, CuratorRow*> m_curatorRows;

    ///
    /// \brief Add a curator label to the layout
    ///
    void addCuratorLabelRow(CuratorLabel *curatorLabel);

    ///
    /// \brief Layout for the full game progress widgets
    ///
    QGridLayout* m_fullGameProgressLayout;

    ///
    /// \brief Progress label for the full game
    ///
    QLabel* m_fullGameProgressLabel;

    ///
    /// \brief Progress Bar, gives visual feedback of the completion of the game
    ///
    QProgressBar* m_fullGameProgressBar;

    ///
    /// \brief Local label for the full game
    ///
    QLabel* m_localLostnessLabel;

    ///
    /// \brief Lostness Bar, gives visual feedback of the lostness value
    ///
    QProgressBar* m_localLostnessBar;
};

class ObjectiveRow : public QObject
{
    Q_OBJECT

public: // methods
    ///
    /// \brief Constructor.
    ///
    /// \param [in] nameLabel       Objective name.
    ///
    ObjectiveRow(QObject *object, QString name, QGridLayout *rowLayout);

    ///
    /// \brief Remove all widgets from the row
    ///
    void removeRow();

    ///````
    /// \brief Set the lostness of the objective
    ///
    void setLostness(float lostness);

    ///
    /// \brief Reset all bars
    ///
    void reset();
private:
    ///
    /// \brief Layout for this objective row
    ///
    QGridLayout* m_rowLayout;

    ///
    /// \brief Name
    ///
    QLabel* m_nameLabel;

    ///
    /// \brief Lostness Bar, gives visual feedback on how "lost the user is"
    ///
    QProgressBar* m_lostnessBar;
};

class CuratorRow : public QObject
{
    Q_OBJECT

public: // methods

    ///
    /// \brief Constructor.
    ///
    /// \param [in] editor          AnalyticsProperties that this CuratorRow is part of.
    /// \param [in] nameLabel       Curator label name.
    ///
    CuratorRow(AnalyticsProperties *editor, QLabel *nameLabel, QGridLayout *rowLayout, QGridLayout *dependencyRowLayout, QHash<QString, ObjectiveRow*> &dependenciesList);

    ///
    /// \brief Remove all widgets from the row
    ///
    void removeRow();

    ///
    /// \brief Has the task been started?
    ///
    bool getStarted(){return m_started;}

    ///
    /// \brief Set the task as started
    ///
    void setStarted(bool started);

    ///
    /// \brief Update the progress bar if there is a dependency match
    ///
    void updateProgress(float progress);

    ///
    /// \brief Update the lostness bar
    ///
    void updateLostness(float newValue);

    ///
    /// \brief Update the similarity bar
    ///
    void updateSimilarity(float newValue);

    ///
    /// \brief Update the lostness bar
    ///
    void updateLostnessOfObjective(QString objectiveName, float newValue);

    ///
    /// \brief Reset all bars
    ///
    void reset();

private:
    ///
    /// \brief Layout for this curator label row
    ///
    QGridLayout* m_rowLayout;
    ///
    /// \brief Layout for the objectives
    ///
    QGridLayout* m_dependencyRowLayout;

    ///
    /// \brief Name
    ///
    QLabel* m_nameLabel;

    ///
    /// \brief Progress label
    ///
    QLabel* m_progressLabel;

    ///
    /// \brief Progress Bar, gives visual feedback of the completion of the task
    ///
    QProgressBar* m_progressBar;

    ///
    /// \brief Lostness label
    ///
    QLabel* m_lostnessLabel;

    ///
    /// \brief Lostness Bar, gives visual feedback on how "lost the user is"
    ///
    QProgressBar* m_lostnessBar;

    ///
    /// \brief Similarity label
    ///
    QLabel* m_similarityLabel;

    ///
    /// \brief Similarity Bar, gives visual feedback on how well the player is matching the perfect sequence
    ///
    QProgressBar* m_similarityBar;

    ///
    /// \brief List of narrative dependencies (bool to show if achieved etc. accessed through these)
    ///
    QHash<QString, ObjectiveRow*> m_dependencies;

    ///
    /// \brief Whether or not the task has been started
    ///
    bool m_started;

    ///
    /// \brief Whether or not the task has been completed
    ///
    bool m_completed;
};

#endif // ANALYTICSPROPERTIES_H
