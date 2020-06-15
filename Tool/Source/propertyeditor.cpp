#include "propertyeditor.h"

#include <QVBoxLayout>

#include "collapsible.h"
#include "nodeproperties.h"
#include "mainctrl.h"

PropertyEditor::PropertyEditor(QWidget *parent)
    : QScrollArea(parent)
    , m_mainCtrl(nullptr)
{
    // setup the scroll area
    setFrameShape(QFrame::NoFrame);
    setWidgetResizable(true);
    setMaximumWidth(Collapsible::getMaximumWidth()+4);

    // set up the view widget
    QWidget* viewWidget = new QWidget(this);
    setWidget(viewWidget);

    // ... and the layout of the view widget
    m_layout = new QVBoxLayout(viewWidget);
    m_layout->setContentsMargins(QMargins(4,0,0,0));
    m_layout->addStretch();

    //set up the analytics properties
    setupAnalyticsProperties();
}

void PropertyEditor::showNodes(const QList<zodiac::NodeHandle>& selection)
{
    Q_ASSERT(m_mainCtrl);

    // collect all collapsibles to remove
    QVector<zodiac::NodeHandle> removed;
    for(QHash<zodiac::NodeHandle, Collapsible*>::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i){
        //if(!selection.contains(i.key())){
            m_layout->removeWidget(i.value());
            removed.append(i.key());
        //}
    }
    // ... and remove them after the iteration has finished
    for(zodiac::NodeHandle node : removed){
        m_nodes.take(node)->deleteLater();
    }

    // then create the new collapsibles
    for(zodiac::NodeHandle node : selection){
        if(!m_nodes.contains(node)){
            Collapsible* collapsible = new Collapsible(this, this);
            NodeProperties *newNodeProperties = new NodeProperties(m_mainCtrl->getCtrlForHandle(node), collapsible, m_pCommands, m_pUndoStack);
            collapsible->setWidget(newNodeProperties, newNodeProperties);
            m_layout->insertWidget(1, collapsible); // insert the new Collapsible at the top, underneath the analytics (1 rather than 0)
            m_nodes.insert(node, collapsible);
        }
    }
}

Collapsible* PropertyEditor::getCollapsible(zodiac::NodeHandle node)
{
    if(m_nodes.contains(node))
        return m_nodes[node];
    else
    {
        qDebug() << "Error, node does not exist";
        return nullptr;
    }

}

void PropertyEditor::setupAnalyticsProperties()
{
    Collapsible *collapsible = new Collapsible(this, this);
    AnalyticsProperties *properties = new AnalyticsProperties(collapsible);
    collapsible->setWidget(properties, nullptr);

    m_analyticsProperties = qMakePair(properties, collapsible);

    m_layout->insertWidget(0, collapsible); // insert the new Collapsible at the top

    //connect to allow values to be updated
    connect(this, SIGNAL(UpdateLinkerValues(QList<zodiac::NodeHandle>&)),
            properties, SLOT(UpdateLinkerValues(QList<zodiac::NodeHandle>&)));
}
