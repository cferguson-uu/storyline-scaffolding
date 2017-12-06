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
    /// \param [in] plugName    Name of the plug to remove.
    ///
    void removeCuratorRow(const QString& plugName);

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

    void addCuratorLabelRow(CuratorLabel *curatorLabel);
    void removePlugRow(const QString& curatorLabelName);
};

class CuratorRow : public QObject
{
    Q_OBJECT

public: // methods

    ///
    /// \brief Constructor.
    ///
    /// \param [in] editor          AnalyticsProperties that this CuratorRow is part of.
    /// \param [in] nameLabel        Curator label name.
    ///
    CuratorRow(AnalyticsProperties *editor, QLabel *nameLabel, QGridLayout *rowLayout);

private:

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
    QLabel* m_LostnessLabel;

    ///
    /// \brief Lostness Bar, gives visual feedback on how "lost the user is"
    ///
    QProgressBar* m_LostnessBar;

    ///
    /// \brief Similarity label
    ///
    QLabel* m_similarityLabel;

    ///
    /// \brief Similarity Bar, gives visual feedback on how well the player is matching the perfect sequence
    ///
    QProgressBar* m_similarityBar;

    ///
    /// \brief List of narrative dependencies with bool to show if achieved
    ///
    QList<QPair<QString, bool>> m_dependencies;
};

#endif // ANALYTICSPROPERTIES_H
