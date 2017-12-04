#ifndef ANALYTICSPROPERTIES_H
#define ANALYTICSPROPERTIES_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include "zodiacgraph/nodehandle.h"

class Collapsible;
class QVBoxLayout;

///
/// \brief Analytics Property widget, is a display widget of a Collapsible. Used for showing analytics info
///
class AnalyticsProperties : public QWidget
{
    Q_OBJECT

public:// methods

    ///
    /// \brief Constructor.
    ///
    /// \param [in] parent  Collapsible parent object.
    ///
    AnalyticsProperties(Collapsible *parent);

public slots:
    void UpdateLinkerValues(QList<zodiac::NodeHandle> &nodes);

private:
    QVBoxLayout* m_mainLayout;

    QLabel *m_storyNodesWithConnections;
    QLabel *m_storyNodesWithoutConnections;
    QLabel *m_storyNodesAverageConnections;
    QLabel *m_narrativeNodesWithConnections;
    QLabel *m_narrativeNodesWithoutConnections;
    QLabel *m_narrativeNodesAverageConnections;
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
    CuratorRow(AnalyticsProperties *editor, QLabel *nameLabel);

private:

    ///
    /// \brief Name
    ///
    QLabel* m_nameLabel;

    //will have a lostness indicator and a sequence indicator
};

#endif // ANALYTICSPROPERTIES_H
