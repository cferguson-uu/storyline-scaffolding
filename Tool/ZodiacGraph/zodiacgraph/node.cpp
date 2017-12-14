#include "node.h"

#include <QEasingCurve>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>
#include <QTextOption>
#include <QtMath>                       // for M_PI
#include <QVector2D>

#include <cfloat>                       // for DBL_MAX

#include "edgegroupinterface.h"
#include "nodelabel.h"
#include "utils.h"
#include "plug.h"
#include "scene.h"
#include "view.h"
#include "perimeter.h"
#include "plugarranger.h"
#include "straightedge.h"

#include <QTimer>
#include <QDebug>

static void fireAnimation(QPropertyAnimation& animation, qreal targetValue, qreal startValue, qreal duration, const QEasingCurve& curve);
static zodiac::Plug* findClosestPlug(const QPointF& pos, const QSet<zodiac::Plug*> &plugs, zodiac::Plug* closest);
static qreal angularDistance(qreal alpha, qreal beta);

namespace zodiac {

//prefixes
//setting
static const QString kPrefix_Characters = "CHAR_";
static const QString kPrefix_Locations = "LOC_";
static const QString kPrefix_Times = "TIM_";
static const QString kPrefix_Detail = "DET_";

//theme
static const QString kPrefix_ThemeEvent = "TEV_";
static const QString kPrefix_ThemeGoal = "TGO_";

//plot
static const QString kPrefix_Episode = "EP_";
static const QString kPrefix_SubGoal = "SUBG_";
static const QString kPrefix_Attempt = "ATT_";
static const QString kPrefix_Outcome = "OUT_";

//resolution
static const QString kPrefix_ResolutionEvent = "REV_";
static const QString kPrefix_ResolutionState = "RST_";

qreal Node::s_coreRadius = 25.;
//QColor Node::s_idleColor = QColor("#4b77a7");
//QColor Node::s_selectedColor = QColor("#62abfa");
//QColor Node::s_outlineColor = QColor("#cdcdcd");
qreal Node::s_outlineWidth = 3.;
QString Node::s_plugSuffix = "_";
qreal Node::s_plugSweep = s_coreRadius*1.3;
qreal Node::s_plugGap = s_plugSweep / 4.;
int Node::s_expandDuration = 400;
int Node::s_collapseDuration = 400;
QEasingCurve Node::s_expandCurve = QEasingCurve::OutQuad;
QEasingCurve Node::s_collapseCurve = QEasingCurve::OutQuad;
//QPen Node::s_linePen = QPen(QBrush(s_outlineColor), s_outlineWidth);
bool Node::s_mouseWasDragged = false;
bool Node::s_inAnalyticsMode = false;

Node::Node(Scene* scene, const QString &displayName, const QString &description, NodeType nodeType, const QUuid &uuid,
           QColor idleColor, QColor selectedColor, QColor outlineColor,
           QColor labelBackgroundColor, QColor labelTextColor, QColor labelLineColor)
    : QGraphicsObject(nullptr)
    , m_scene(scene)
    , m_displayName(displayName)
    , m_displayDescription(description)
    , m_nodeType(nodeType)
    , m_uniqueId(uuid.isNull() ? QUuid::createUuid() : uuid)
    , m_outgoingExpansionFactor(0.)
    , m_incomingExpansionFactor(0.)
    , m_perimeter(nullptr)
    , m_allPlugs(QMap<QString, Plug*>())
    , m_outgoingPlugs(QSet<Plug*>())
    , m_incomingPlugs(QSet<Plug*>())
    , m_straightEdges(QSet<StraightEdge*>())
    , m_label(nullptr)
    , m_expansionState(NodeExpansion::NONE)
    , m_lastExpansionState(NodeExpansion::NONE)
    , m_idleColor(idleColor)
    , m_selectedColor(selectedColor)
    , m_outlineColor(outlineColor)
{    
    setIdleColor(QColor("#4b77a7"));
    setSelectedColor(QColor("#62abfa"));
    setOutlineColor(QColor("#cdcdcd"));
    setOutlineWidth(3);
    m_linePen = QPen(QBrush("#cdcdcd"), s_outlineWidth);

    // set QGraphicsObject flags
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemIsFocusable);
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);

    // create secondary items
    m_perimeter = new Perimeter(this);
    m_label = new NodeLabel(this, labelBackgroundColor, labelTextColor, labelLineColor);

    // initiate members influenced by styling
    updateStyle();

    // setup the animation curves
    m_outgoingExpandAnimation.setTargetObject(this);
    m_outgoingExpandAnimation.setPropertyName("outgoingSpread");
    m_outgoingExpandAnimation.setEndValue(1.);
    m_outgoingCollapseAnimation.setTargetObject(this);
    m_outgoingCollapseAnimation.setPropertyName("outgoingSpread");
    m_outgoingCollapseAnimation.setEndValue(0.);

    m_incomingExpandAnimation.setTargetObject(this);
    m_incomingExpandAnimation.setPropertyName("incomingSpread");
    m_incomingExpandAnimation.setEndValue(1.);
    m_incomingCollapseAnimation.setTargetObject(this);
    m_incomingCollapseAnimation.setPropertyName("incomingSpread");
    m_incomingCollapseAnimation.setEndValue(0.);

    connect(&m_outgoingCollapseAnimation, SIGNAL(finished()), this, SLOT(hasCompletelyCollapsed()));
    connect(&m_incomingCollapseAnimation, SIGNAL(finished()), this, SLOT(hasCompletelyCollapsed()));
}

Plug *Node::createPlug(const QString& name, PlugDirection direction)
{
    // plugs must either be INcoming or OUTgoing
#ifdef QT_DEBUG
    Q_ASSERT(direction!=PlugDirection::BOTH);
#else
    if(direction==PlugDirection::BOTH){
        direction = PlugDirection::IN;
    }
#endif

    // create and return the plug
    QString uniqueName = getUniquePlugName(name);
    Plug* newPlug = new Plug(this, uniqueName, direction);
    m_allPlugs.insert(uniqueName, newPlug);

    if(direction==PlugDirection::IN){
        m_incomingPlugs.insert(newPlug);
    } else {
        m_outgoingPlugs.insert(newPlug);
    }

    // update the node repesentation to accomodate the new plug
    updateStyle();

    return newPlug;
}

bool Node::removePlug(Plug *plug)
{
    // make sure that the Plug is actually of this Node
#ifdef QT_DEBUG
    Q_ASSERT((m_allPlugs.value(plug->getName(), nullptr) == plug));
#else
    if((m_allPlugs.value(plug->getName(), nullptr) != plug)){
        return false;
    }
#endif

    // plugs with edges cannot be deleted
    if(!plug->isRemovable()){
        return false;
    }

    // unregister the plug
    if(m_incomingPlugs.contains(plug)){
        m_incomingPlugs.remove(plug);
    } else {
        Q_ASSERT(m_outgoingPlugs.contains(plug));
        m_outgoingPlugs.remove(plug);
    }

    // unregister from every other possible reference
    if(Plug::getDragTargetPlug()==plug){
        Plug::clearDragTargetPlug();
    }
    if(Perimeter::getClosestPlugToMouse()==plug){
        Perimeter::clearClosestPlugToMouse();
    }

    // remove the plug from the scene and memory
    m_allPlugs.remove(plug->getName());
    scene()->removeItem(plug);
    plug->deleteLater();

    // adjust the node display
    updateStyle();

    return true;
}

Plug* Node::getClosestPlugTo(const QPointF& pos, PlugDirection direction)
{
    // to prevent mid-animation selection of plugs, narrow the allowed direction if required
    if(direction==PlugDirection::BOTH){
        if(m_expansionState==NodeExpansion::OUT){
            direction = PlugDirection::OUT;
        } else if (m_expansionState==NodeExpansion::IN){
            direction = PlugDirection::IN;
        }
    }

    // find and return the closest plug
    Plug* closest=nullptr;
    if(direction==PlugDirection::IN){
        closest = findClosestPlug(pos, m_incomingPlugs, closest);
    } else { // OUT or BOTH
        closest = findClosestPlug(pos, m_outgoingPlugs, closest);
    }
    if(direction==PlugDirection::BOTH){
        closest = findClosestPlug(pos, m_incomingPlugs, closest);
    }
    return closest;
}

qreal Node::getPerimeterRadius() const
{
    return m_perimeter->getRadius();
}

void Node::setDisplayName(const QString& displayName)
{
    m_displayName=displayName;

    if(m_nodeType == NODE_STORY)
        m_label->setText(static_cast<StoryNode*>(this)->getStoryNodePrefix() + m_displayName);
    else
        m_label->setText(m_displayName);


    for(Plug* plug : m_allPlugs){
        plug->updateEdgeLabels();
    }
    for(StraightEdge* edge : m_straightEdges){
        edge->updateLabel();
    }
    emit nodeRenamed(m_displayName);
}

void Node::setDisplayDescription(const QString& displayDescription)
{
    if(m_displayDescription == displayDescription){
        return;
    }
    m_displayDescription = displayDescription;
}

void Node::setLabelBackgroundColor(const QColor& color)
{
    m_label->setBackgroundColor(color);
}

QString Node::renamePlug(Plug *plug, const QString& newName)
{
    // make sure that the Plug is actually of this Node
#ifdef QT_DEBUG
    Q_ASSERT((m_allPlugs.value(plug->getName(), nullptr) == plug));
#else
    if((m_allPlugs.value(plug->getName(), nullptr) != plug)){
        return "";
    }
#endif

    // rename the plug and re-insert it with the new name
    m_allPlugs.remove(plug->getName());

    QString uniqueName = getUniquePlugName(newName, plug);
    plug->setName(uniqueName);
    m_allPlugs.insert(uniqueName, plug);

    return uniqueName;
}

bool Node::togglePlugDirection(Plug* plug)
{
    // make sure that the Plug is actually of this Node
#ifdef QT_DEBUG
    Q_ASSERT((m_allPlugs.value(plug->getName(), nullptr) == plug));
#else
    if((m_allPlugs.value(plug->getName(), nullptr) != plug)){
        return false;
    }
#endif

    // only plugs without connections can change direction
    if(plug->getEdgeCount()>0){
        return false;
    }

    // change the plugs's direction
    if(plug->getDirection()==PlugDirection::IN){
        // from IN to OUT
        Q_ASSERT(m_incomingPlugs.contains(plug));
        m_incomingPlugs.remove(plug);
        plug->setDirection(PlugDirection::OUT);
        m_outgoingPlugs.insert(plug);

    } else {
        // from OUT to IN
        Q_ASSERT(m_outgoingPlugs.contains(plug));
        m_outgoingPlugs.remove(plug);
        plug->setDirection(PlugDirection::IN);
        m_incomingPlugs.insert(plug);
    }

    // initiate a complete update of the geometry
    updateStyle();

    return true;
}

void Node::updateStyle()
{
    prepareGeometryChange();

    // apply the static style to this instance
    qreal halfLine = s_outlineWidth*0.5;
    m_boundingRect = quadrat(s_coreRadius).marginsAdded(QMarginsF(halfLine, halfLine, halfLine, halfLine));

    // adjust the style of the label
    m_label->updateStyle();

    // adjust perimeter and plugs
    arrangePlugs();
    adjustRadius();

    // update style of perimeter
    m_perimeter->setOpacity(qMax(m_outgoingExpansionFactor, m_incomingExpansionFactor)*Perimeter::getMaxOpacity());
    m_perimeter->updateStyle();

    // update style of plugs
    for(Plug* plug: m_allPlugs.values()){
        plug->updateStyle();
    }

//    arrangePlugs();
}

QRectF Node::boundingRect() const
{
    return m_boundingRect;
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /* widget */)
{
    painter->setClipRect(option->exposedRect);

    // draw the node a different color, if it is selected
    if(isSelected()){
        painter->setBrush(m_selectedColor);
    } else {
        painter->setBrush(m_idleColor);
    }

    // draw core
    //painter->setPen(s_linePen);
    painter->setPen(m_linePen);
    painter->drawEllipse(quadrat(s_coreRadius));
}

QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(m_boundingRect);
    return path;
}

void Node::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    // expand the outgoing plugs first
    if(m_expansionState==NodeExpansion::NONE){
        setExpansion(NodeExpansion::OUT);
    }

    // raise the node to the front
    setZValue(zStack::NODE_ACTIVE);

    return QGraphicsItem::hoverEnterEvent(event);
}

void Node::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    // automatically collapse all plugs, if the node is not forced open
    if(m_expansionState!=NodeExpansion::BOTH){
        setExpansion(NodeExpansion::NONE);
    }

    // drop the node back with the others
    setZValue(zStack::NODE_EXPANDED);

    return QGraphicsItem::hoverLeaveEvent(event);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // reset the mouse drag detection
    s_mouseWasDragged = false;

    if(event->buttons() & View::getSelectionButton()){
        // select the node
        QGraphicsObject::mousePressEvent(event);

    } else if(event->buttons() & View::getRemovalButton()){
        // remove the node
        event->accept();
        emit removalRequested();
        return;
    }else if(event->buttons() & View::getMoveButton()){   //if not scroll dragging then bring up a context menu
        if(m_nodeType == NODE_STORY)
        {
            StoryNode *sNode = static_cast<StoryNode*>(this);

            QContextMenuEvent fakeEvent(QContextMenuEvent::Mouse, event->pos().toPoint(), event->screenPos());
            sNode->contextMenuEvent(&fakeEvent);
        }
        else
            if(m_nodeType == NODE_NARRATIVE)
            {
                NarrativeNode *nNode = static_cast<NarrativeNode*>(this);

                QContextMenuEvent fakeEvent(QContextMenuEvent::Mouse, event->pos().toPoint(), event->screenPos());
                nNode->contextMenuEvent(&fakeEvent);
            }
    }
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if(s_inAnalyticsMode)   //don't allow the nodes to be moved when in analytics mode
        return;

    // if this method is called, then the mouse is being dragged
    s_mouseWasDragged = true;

    // update the edges of all selected nodes, including yourself
    for(QGraphicsItem* selectedItem : scene()->selectedItems()){
        Node* selectedNode = qobject_cast<Node*>(selectedItem->toGraphicsObject());
        if(selectedNode){
            selectedNode->updateConnectedEdges();
        }
    }
    QGraphicsObject::mouseMoveEvent(event);
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    // toggle between IN and OUT expansion on single click
    if((event->button() == View::getSelectionButton()) && (!s_mouseWasDragged)){
        toggleExpansion();
    }
    QGraphicsObject::mouseReleaseEvent(event);
}

void Node::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // toggle forced expansion on double-click
    if(event->buttons() & View::getSelectionButton()){
        toggleForcedExpansion();
    }
    QGraphicsObject::mouseDoubleClickEvent(event);
}

void Node::keyPressEvent(QKeyEvent* event)
{
    // activate all selected nodes
    if(event->key() == View::getActivationKey()){
        event->accept();
        for(QGraphicsItem* selectedItem : scene()->selectedItems()){
            Node* selectedNode = qobject_cast<Node*>(selectedItem->toGraphicsObject());
            if((selectedNode) && (selectedNode!=this)){
                emit selectedNode->nodeActivated();
            }
        }
        emit nodeActivated(); // make sure that this node is activated last
    }
    QGraphicsObject::keyPressEvent(event);
}

QString Node::getUniquePlugName(const QString& name, const Plug *plug)
{
    // if the name is already unique, just return it
    if(!m_allPlugs.contains(name)){
        return name;
    }

    // increase the suffix index until the name is unique, then return it
    uint index = 2;
    Plug* testPlug;
    QString uniqueName;
    do{
        uniqueName = name + s_plugSuffix + QString::number(index++);
        testPlug = m_allPlugs.value(uniqueName, nullptr);
        if(testPlug==plug){ // if the user wants to rename x_2 to x while x already exists, that's a no-op
            return uniqueName;
        }
    } while(testPlug);
    return uniqueName;
}

void Node::hasCompletelyCollapsed()
{
    // this slot is called when the incoming OR outgoing plugs have collapsed but we want only the second call
    if((m_outgoingExpansionFactor>0) || (m_incomingExpansionFactor>0)){
        return;
    }
    setZValue(zStack::NODE_CLOSED);
    setExpansion(NodeExpansion::NONE);
}

void Node::aboutToExpandAfresh()
{
    arrangePlugs();
}

void Node::setExpansion(NodeExpansion newState)
{
    if(newState == m_expansionState){
        return;
    }

    if(newState==NodeExpansion::NONE){
        m_outgoingExpandAnimation.stop();
        m_incomingExpandAnimation.stop();
        fireAnimation(m_incomingCollapseAnimation, 0.0, m_incomingExpansionFactor, s_collapseDuration, s_collapseCurve);
        fireAnimation(m_outgoingCollapseAnimation, 0.0, m_outgoingExpansionFactor, s_collapseDuration, s_collapseCurve);
    } else {

        // callback for expanding afresh
        if((m_expansionState==NodeExpansion::NONE) && (m_incomingExpansionFactor==0.) && (m_outgoingExpansionFactor==0.)){
            aboutToExpandAfresh();
        }

        // move the node upwards in z
        if(zValue()<zStack::NODE_EXPANDED){
            setZValue(zStack::NODE_EXPANDED);
        }

        if(newState==NodeExpansion::IN){
            m_outgoingExpandAnimation.stop();
            m_incomingCollapseAnimation.stop();
            fireAnimation(m_incomingExpandAnimation, 1.0, m_incomingExpansionFactor, s_expandDuration, s_expandCurve);
            fireAnimation(m_outgoingCollapseAnimation, 0.0, m_outgoingExpansionFactor, s_collapseDuration, s_collapseCurve);

        } else if(newState==NodeExpansion::OUT){
            m_outgoingCollapseAnimation.stop();
            m_incomingExpandAnimation.stop();
            fireAnimation(m_incomingCollapseAnimation, 0.0, m_incomingExpansionFactor, s_collapseDuration, s_collapseCurve);
            fireAnimation(m_outgoingExpandAnimation, 1.0, m_outgoingExpansionFactor, s_expandDuration, s_expandCurve);

        } else { // BOTH
            m_outgoingCollapseAnimation.stop();
            m_incomingCollapseAnimation.stop();
            fireAnimation(m_incomingExpandAnimation, 1.0, m_incomingExpansionFactor, s_expandDuration, s_expandCurve);
            fireAnimation(m_outgoingExpandAnimation, 1.0, m_outgoingExpansionFactor, s_expandDuration, s_expandCurve);
        }
    }

    m_lastExpansionState = m_expansionState;
    m_expansionState = newState;
}

void Node::updateConnectedEdges()
{
    // plugs
    for(Plug* plug: m_allPlugs){
        plug->updateEdges();
    }

    // straight edges
    for(StraightEdge* straightEdge : m_straightEdges){
        straightEdge->nodePositionHasChanged();
    }
}

void Node::updateIncomingSpread(qreal expansion)
{
    // store the new value
    m_incomingExpansionFactor = expansion;

    // update the perimeter opacity
    m_perimeter->setOpacity(qMax(m_outgoingExpansionFactor, m_incomingExpansionFactor)*Perimeter::getMaxOpacity());

    // update the plugs
    for(Plug* plug : m_incomingPlugs){
        plug->updateExpansion(m_incomingExpansionFactor);
    }
}

void Node::updateOutgoingSpread(qreal expansion)
{
    // store the new value
    m_outgoingExpansionFactor = expansion;

    // update the perimeter opacity
    m_perimeter->setOpacity(qMax(m_outgoingExpansionFactor, m_incomingExpansionFactor)*Perimeter::getMaxOpacity());

    // update the plugs
    for(Plug* plug : m_outgoingPlugs){
        plug->updateExpansion(m_outgoingExpansionFactor);
    }
}

void Node::arrangePlugs()
{
    // return early if there are no plugs to arrange
    int plugCount = m_allPlugs.size();
    if(plugCount==0){
        return;
    }

    // at first, there are as many zones above as are below the label
    int evenZoneCount = plugCount+(plugCount%2);
    int halfZoneCount = evenZoneCount/2;
    qreal gapAngle = getGapAngle();

    // the deadzone is calculated at the height of the label at the inner radius of an incoming plug
    // it is the angular part of the perimeter that could be taken up by the node's label
    qreal halfDeadAngle = getDeadZoneAngle() * 0.5;

    // calculate the zone directions
    QVector<qreal>zoneDirections(evenZoneCount);
    {
        // the angular span of the plugs is determined by their number
        qreal zoneSpan = ( M_PI - (2.*halfDeadAngle) - ((halfZoneCount+1)*gapAngle)) / halfZoneCount;

        // upper half
        qreal currentAngle = halfDeadAngle + gapAngle + (zoneSpan/2);
        for(int index = 0; index < halfZoneCount; ++index){
            zoneDirections[index] = currentAngle;
            currentAngle += gapAngle + zoneSpan;
        }
        // lower half
        currentAngle = -M_PI + halfDeadAngle + gapAngle + (zoneSpan/2);
        for(int index = halfZoneCount; index < evenZoneCount; ++index){
            zoneDirections[index] = currentAngle;
            currentAngle += gapAngle + zoneSpan;
        }
    }

    // get the plugs' target directions
    QList<Plug*> plugs = m_allPlugs.values();
    QList<QPair<Plug*, qreal>> plugDirections;
    for(Plug* plug : plugs){
        if(plug->getEdgeCount()==0){
            continue;
        }

        // calling atan2 with (-y, x) turns the direction from Qt's screen coordinates (with an inverted y-axis) to
        // the one used by me, where positive x is right, positive y is up and the zero-angle is on positive x.
        QVector2D plugTarget = plug->getTargetNormal();
        plugDirections.append(QPair<Plug*, qreal>(plug, qAtan2(-plugTarget.y(), plugTarget.x())));
    }

    // calculate a trivial path for all unconnected Plugs
    int connectedPlugCount = plugDirections.count();
    QVector<int> optimalPath;
    for(int i = 0; i < plugCount; ++i){
        optimalPath.append(i);
    }

    // if there are connected plugs, calculcate their semi-optimal placement
    if(connectedPlugCount>0){

        // build the cost table
        QVector<qreal>costTable(connectedPlugCount*evenZoneCount);
        for(int row = 0; row < connectedPlugCount; ++row){
            Plug* plug = plugDirections.at(row).first;
            qreal plugDirection = plugDirections.at(row).second;
            for(int column = 0; column < evenZoneCount; ++column){
                qreal cost = angularDistance(plugDirection, zoneDirections[column]) * plug->getArrangementPriority();
                costTable[(row*evenZoneCount) + column] = cost * cost;
            }
        }

        // modify the trivial path to the semi-optimal one
        int i = 0;
        for(int zoneIndex : zodiac::arrangePlugs(costTable, connectedPlugCount, evenZoneCount)){
            int plugIndex = plugs.indexOf(plugDirections.at(i).first);
            int tZoneIndex = optimalPath[plugIndex];
            int occupiedIndex = optimalPath.indexOf(zoneIndex);
            optimalPath[plugIndex] = zoneIndex;
            if(occupiedIndex != -1){
                optimalPath[occupiedIndex] = tZoneIndex;
            }
            ++i;
        }
    }

    // collapse the unused zone, if it exists
    if(plugCount<evenZoneCount){

        // find the empty zone index
        int emptyZoneIndex = -1;
        for(int i = 0; i<evenZoneCount; ++i){
            if(!optimalPath.contains(i)){
                emptyZoneIndex = i;
                break;
            }
        }

        // find out if the empty zone is in the top or bottom half
        qreal offset;
        int startIndex, endIndex;
        if(emptyZoneIndex < halfZoneCount){
            // empty zone is in top half
            offset = 0.;
            startIndex = 0;
            endIndex = halfZoneCount;
        } else {
            // empty zone is in bottom half
            offset = -M_PI;
            startIndex = halfZoneCount;
            endIndex = evenZoneCount;
        }

        // collapse the empty zone by extending the span of the other zones in its half
        qreal newZoneSpan = ( M_PI - (2.*halfDeadAngle) - ((halfZoneCount)*gapAngle)) / (halfZoneCount-1);
        qreal currentAngle = offset + halfDeadAngle + gapAngle + (newZoneSpan/2);
        for(int index = startIndex; index < endIndex; ++index){
            if(index==emptyZoneIndex){
                continue;
            }
            zoneDirections[index] = currentAngle;
            currentAngle += gapAngle + newZoneSpan;
        }
    }

    // apply the plug placement
    qreal plugSweepAngle = getPlugAngle();
    for(int plugIndex = 0; plugIndex < plugCount; ++plugIndex){
        qreal angle = zoneDirections.at(optimalPath.at(plugIndex));
        plugs.at(plugIndex)->defineShape(QVector2D(qCos(angle), -qSin(angle)), plugSweepAngle);
    }
}

qreal Node::getPlugAngle() const
{
    return s_plugSweep/m_perimeter->getRadius();
}

qreal Node::getGapAngle() const
{
    return s_plugGap/m_perimeter->getRadius();
}

qreal Node::getDeadZoneAngle() const
{
    return m_label->getHeight() / (m_perimeter->getRadius() - (Plug::getWidth() * 1.5));
}

void Node::adjustRadius()
{
    //
    // return early, if there are no plugs on this node
    int plugCount = m_allPlugs.count();
    if(plugCount==0){
        m_perimeter->setRadius(0.); // resets the perimeter's radius to its minimal radius
        return;
    }

    //
    // set the required radius of the perimeter
    qreal deadArcLength = getDeadZoneAngle() * m_perimeter->getRadius();
    qreal requiredArcLength
            = (s_plugSweep * plugCount)
            + (s_plugGap * (plugCount + 2))
            + (deadArcLength * 2);
    qreal radius = requiredArcLength/(2 * M_PI);
    m_perimeter->setRadius(radius); // perimeter does not shrink pass its minimal radius

    //
    // update the shape and position of all plugs
    for(Plug* plug : m_incomingPlugs){
        plug->defineShape(plug->getNormal(), getPlugAngle());
        plug->updateExpansion(m_incomingExpansionFactor);
    }
    for(Plug* plug : m_outgoingPlugs){
        plug->defineShape(plug->getNormal(), getPlugAngle());
        plug->updateExpansion(m_outgoingExpansionFactor);
    }
}

void Node::updateOutlinePen()
{
    //s_linePen = QPen(QBrush(s_outlineColor), s_outlineWidth);
    m_linePen = QPen(QBrush(m_outlineColor), s_outlineWidth);
}

bool Node::isNodeDecorator()
{
    if(m_nodeType == zodiac::NODE_NARRATIVE)
    {
        return(m_displayName == "SEQ" || m_displayName == "INV");
    }
    else
        if(m_nodeType == zodiac::NODE_STORY)
        {
             StoryNode *sNode = static_cast<StoryNode*>(this);

            return(sNode->getStoryNodeType() == zodiac::STORY_NAME || sNode->getStoryNodeType() == zodiac::STORY_SETTING || sNode->getStoryNodeType() == zodiac::STORY_SETTING_CHARACTER_GROUP ||
                    sNode->getStoryNodeType() == zodiac::STORY_SETTING_LOCATION_GROUP || sNode->getStoryNodeType() == zodiac::STORY_SETTING_TIME_GROUP || sNode->getStoryNodeType() == zodiac::STORY_THEME||
                    sNode->getStoryNodeType() == zodiac::STORY_THEME_EVENT_GROUP || sNode->getStoryNodeType() == zodiac::STORY_THEME_GOAL_GROUP || sNode->getStoryNodeType() == zodiac::STORY_PLOT ||
                    sNode->getStoryNodeType() == zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP || sNode->getStoryNodeType() == zodiac::STORY_PLOT_EPISODE_OUTCOME_GROUP ||
                    sNode->getStoryNodeType() == zodiac::STORY_RESOLUTION || sNode->getStoryNodeType() == zodiac::STORY_RESOLUTION_EVENT_GROUP ||
                    sNode->getStoryNodeType() == zodiac::STORY_RESOLUTION_STATE_GROUP);
        }

    qDebug() << "Error, not a narrative or story node";
    return false;
}

NarrativeNode::NarrativeNode(Scene* scene, const QString& displayName, const QString &description, NodeType nodeType, bool load, const QUuid& uuid, QColor idleColor, QColor selectedColor, QColor outlineColor,
              QColor labelBackgroundColor, QColor labelTextColor, QColor labelLineColor)
        : Node(scene, displayName, description, nodeType, uuid, idleColor, selectedColor, outlineColor, labelBackgroundColor, labelTextColor, labelLineColor)
        , m_locked(true)
{
    if(load)
    {
        //behaviour to be carried out when a node is being loaded
    }
}

void NarrativeNode::addOnUnlockCommand(const QUuid& key, const QString& value, const QString& description)
{
    m_onUnlock.insert(key, NodeCommand(value, description));
}

void NarrativeNode::addOnFailCommand(const QUuid& key, const QString& value, const QString& description)
{
    m_onFail.insert(key, NodeCommand(value, description));
}

void NarrativeNode::addOnUnlockedCommand(const QUuid& key, const QString& value, const QString& description)
{
    m_onUnlocked.insert(key, NodeCommand(value, description));
}

void NarrativeNode::removeOnUnlockCommand(const QUuid& key)
{
    m_onUnlock.remove(key);
}
void NarrativeNode::removeOnFailCommand(const QUuid& key)
{
    m_onFail.remove(key);
}

void NarrativeNode::removeOnUnlockedCommand(const QUuid& key)
{
    m_onUnlocked.remove(key);
}

QString NarrativeNode::getParameterFromOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey)
{
    return m_onUnlock[cmdKey].parameters[paramKey];
}

QString NarrativeNode::getParameterFromOnFailCommand(const QUuid& cmdKey, const QString& paramKey)
{
    return m_onFail[cmdKey].parameters[paramKey];
}

QString NarrativeNode::getParameterFromOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey)
{
    return m_onUnlocked[cmdKey].parameters[paramKey];
}

void NarrativeNode::addParameterToOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_onUnlock[cmdKey].parameters.insert(paramKey, value);
}

void NarrativeNode::addParameterToOnFailCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_onFail[cmdKey].parameters.insert(paramKey, value);
}

void NarrativeNode::addParameterToOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_onUnlocked[cmdKey].parameters.insert(paramKey, value);
}

void NarrativeNode::removeParameterFromOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey)
{
    m_onUnlock[cmdKey].parameters.remove(paramKey);
}

void NarrativeNode::removeParameterFromOnFailCommand(const QUuid& cmdKey, const QString& paramKey)
{
    m_onFail[cmdKey].parameters.remove(paramKey);
}

void NarrativeNode::removeParameterFromOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey)
{
    m_onUnlocked[cmdKey].parameters.remove(paramKey);
}

void NarrativeNode::removeAllParametersFromOnUnlockCommand(const QUuid& cmdKey)
{
    m_onUnlock[cmdKey].parameters.clear();
}

void NarrativeNode::removeAllParametersFromOnFailCommand(const QUuid& cmdKey)
{
    m_onFail[cmdKey].parameters.clear();
}

void NarrativeNode::removeAllParametersFromOnUnlockedCommand(const QUuid& cmdKey)
{
    m_onUnlocked[cmdKey].parameters.clear();
}

void NarrativeNode::editParameterInOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_onUnlock[cmdKey].parameters[paramKey] = value;
}

void NarrativeNode::editParameterInOnFailCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_onFail[cmdKey].parameters[paramKey] = value;
}

void NarrativeNode::editParameterInOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_onUnlocked[cmdKey].parameters[paramKey] = value;
}

void NarrativeNode::setLockedStatus(bool status)
{
    m_locked = status;
}

bool NarrativeNode::getLockedStatus()
{
    return m_locked;
}


void NarrativeNode::contextMenuEvent(QContextMenuEvent *event)
{
    //QMenu menu(this);
    QMenu contextMenu((QWidget*)getScene()->getParent());

    QAction* openLinkerWindow = new QAction(tr("&Link Nodes"), this);
    connect(openLinkerWindow, &QAction::triggered, [=]{openLinker();});
    contextMenu.addAction(openLinkerWindow);

    contextMenu.exec(event->globalPos());
}

StoryNode::StoryNode(Scene* scene, const QString& displayName, const QString &description, NodeType nodeType, StoryNodeType storyType, bool load, const QUuid& uuid,
          QColor idleColor, QColor selectedColor, QColor outlineColor,
          QColor labelBackgroundColor, QColor labelTextColor, QColor labelLineColor)
    : Node(scene, displayName, description, nodeType, uuid, idleColor, selectedColor, outlineColor, labelBackgroundColor, labelTextColor, labelLineColor)
      , m_storyNodeType(storyType)
{
    if(!load)
        if(m_storyNodeType == STORY_PLOT_EPISODE)
        {
            QTimer::singleShot(0, (QWidget*)getScene()->getParent(), [=] { createStoryChild(STORY_PLOT_EPISODE_ATTEMPT_GROUP, "Attempt", "Attempt", QPoint(-100,100)); } );
            QTimer::singleShot(0, (QWidget*)getScene()->getParent(), [=] { createStoryChild(STORY_PLOT_EPISODE_OUTCOME_GROUP, "Outcome", "Outcome", QPoint(0,100)); } );
            QTimer::singleShot(0, (QWidget*)getScene()->getParent(), [=] { createStoryChild(STORY_PLOT_EPISODE_SUBGOAL, "SubGoal", "SubGoal", QPoint(100,100)); } );
        }

    setDisplayName(getDisplayName());   //this will add prefix to the current name
}

QString StoryNode::getStoryNodePrefix()
{
    switch(m_storyNodeType)
    {
        case STORY_SETTING_CHARACTER:
            return kPrefix_Characters;
        case STORY_SETTING_LOCATION:
            return kPrefix_Locations;
        case STORY_SETTING_TIME:
            return kPrefix_Times;
        case STORY_ITEM_DETAILS:
            return kPrefix_Detail;
        case STORY_THEME_EVENT:
            return kPrefix_ThemeEvent;
        case STORY_THEME_GOAL:
            return kPrefix_ThemeGoal;
        case STORY_PLOT_EPISODE:
            return kPrefix_Episode;
        case STORY_PLOT_EPISODE_ATTEMPT:
            return kPrefix_Attempt;
        case STORY_PLOT_EPISODE_OUTCOME:
            return kPrefix_Outcome;
        case STORY_PLOT_EPISODE_SUBGOAL:
            return kPrefix_SubGoal;
        case STORY_RESOLUTION_EVENT:
            return kPrefix_ResolutionEvent;
        case STORY_RESOLUTION_STATE:
            return kPrefix_ResolutionState;
        default:
            return "";
    }
}

void StoryNode::contextMenuEvent(QContextMenuEvent *event)
{
    //QMenu menu(this);
    QMenu contextMenu((QWidget*)getScene()->getParent());
    QAction* addCharacterAction;
    QAction* addLocationAction;
    QAction* addTimeAction;
    QAction* addDetailAction;
    QAction* addEventAction;
    QAction* addGoalAction;
    QAction* addEpisodeAction;
    QAction* addAttemptAction;
    QAction* addOutcomeAction;
    QAction* addStateAction;

    switch (m_storyNodeType)
    {
        case STORY_SETTING:
            addCharacterAction = new QAction(tr("&Add Characters"), this);    //add item group then item
            addLocationAction = new QAction(tr("&Add Locations"), this);
            addTimeAction = new QAction(tr("&Add Times"), this);

            connect(addCharacterAction, &QAction::triggered, [=]{createStoryChild(STORY_SETTING_CHARACTER_GROUP, "Characters", "", QPoint(0,100)); });
            connect(addLocationAction, &QAction::triggered, [=]{createStoryChild(STORY_SETTING_CHARACTER_GROUP, "Locations", "", QPoint(0,100)); });
            connect(addTimeAction, &QAction::triggered, [=]{createStoryChild(STORY_SETTING_CHARACTER_GROUP, "Times", "", QPoint(0,100)); });

            contextMenu.addAction(addCharacterAction);
            contextMenu.addAction(addLocationAction);
            contextMenu.addAction(addTimeAction);
            break;
        case STORY_SETTING_CHARACTER_GROUP:
            addCharacterAction = new QAction(tr("&Add Character"), this);

            connect(addCharacterAction, &QAction::triggered, [=]{createStoryChild(STORY_SETTING_CHARACTER, "CHAR_", "", QPoint(0,100)); });

            contextMenu.addAction(addCharacterAction);
            break;
        case STORY_SETTING_LOCATION_GROUP:
            addLocationAction = new QAction(tr("&Add Location"), this);

            connect(addLocationAction, &QAction::triggered, [=]{createStoryChild(STORY_SETTING_LOCATION, "LOC_", "", QPoint(0,100)); });

            contextMenu.addAction(addLocationAction);
            break;
        case STORY_SETTING_TIME_GROUP:
            addTimeAction = new QAction(tr("&Add Time"), this);

            connect(addTimeAction, &QAction::triggered, [=]{createStoryChild(STORY_SETTING_TIME, "TIM_", "", QPoint(0,100)); });

            contextMenu.addAction(addTimeAction);
            break;
        case STORY_SETTING_CHARACTER:
        case STORY_SETTING_LOCATION:
        case STORY_SETTING_TIME:
            addDetailAction = new QAction(tr("&Add Detail"), this);

            connect(addDetailAction, &QAction::triggered, [=]{createStoryChild(STORY_ITEM_DETAILS, "DET_", "", QPoint(0,100)); });

            contextMenu.addAction(addDetailAction);
            break;
        case STORY_THEME:
            addEventAction = new QAction(tr("&Add Events"), this);    //add event group then event
            addGoalAction = new QAction(tr("&Add Goals"), this);      //add goal group then goal

            connect(addEventAction, &QAction::triggered, [=]{createStoryChild(STORY_THEME_EVENT_GROUP, "EVENT", "", QPoint(0,100)); });
            connect(addGoalAction, &QAction::triggered, [=]{createStoryChild(STORY_THEME_GOAL_GROUP, "GOAL", "", QPoint(0,100)); });

            contextMenu.addAction(addEventAction);
            contextMenu.addAction(addGoalAction);
            break;
        case STORY_THEME_EVENT_GROUP:
            addEventAction = new QAction(tr("&Add Event"), this);

            connect(addEventAction, &QAction::triggered, [=]{createStoryChild(STORY_THEME_EVENT, "EV_", "", QPoint(0,100)); });

            contextMenu.addAction(addEventAction);
            break;
        case STORY_THEME_GOAL_GROUP:
            addGoalAction = new QAction(tr("&Add Goal"), this);

            connect(addGoalAction, &QAction::triggered, [=]{createStoryChild(STORY_THEME_GOAL, "GOAL_", "", QPoint(0,100)); });

            contextMenu.addAction(addGoalAction);
            break;
        case STORY_THEME_EVENT:
            addEventAction = new QAction(tr("&Add Sub-Event"), this);

            connect(addEventAction, &QAction::triggered, [=]{createStoryChild(STORY_THEME_EVENT, "EV_", "", QPoint(0,100)); });

            contextMenu.addAction(addEventAction);
            break;
        case STORY_THEME_GOAL:
            addGoalAction = new QAction(tr("&Add Sub-Goal"), this);

            connect(addGoalAction, &QAction::triggered, [=]{createStoryChild(STORY_THEME_GOAL, "GOAL_", "", QPoint(0,100)); });

            contextMenu.addAction(addGoalAction);
            break;
        case STORY_PLOT:
            addEpisodeAction = new QAction(tr("&Add Episode"), this); //add attempt group, add outcome group, add subgoal

            connect(addEpisodeAction, &QAction::triggered, [=]{createStoryChild(STORY_PLOT_EPISODE, "EP_", "", QPoint(0,100)); });

            contextMenu.addAction(addEpisodeAction);
            break;
        case STORY_PLOT_EPISODE_ATTEMPT_GROUP:
            addAttemptAction = new QAction(tr("&Add Attempt"), this);
            addEpisodeAction = new QAction(tr("&Add Sub-Episode"), this);

            connect(addAttemptAction, &QAction::triggered, [=]{createStoryChild(STORY_PLOT_EPISODE_ATTEMPT, "ATT_", "", QPoint(0,100)); });
            connect(addEpisodeAction, &QAction::triggered, [=]{createStoryChild(STORY_PLOT_EPISODE, "SUBEP_", "", QPoint(0,100)); });

            contextMenu.addAction(addAttemptAction);
            contextMenu.addAction(addEpisodeAction);
            break;
        case STORY_PLOT_EPISODE_OUTCOME_GROUP:
            addOutcomeAction = new QAction(tr("&Add Outcome"), this);
            addEpisodeAction = new QAction(tr("&Add Sub-Episode"), this);

            connect(addOutcomeAction, &QAction::triggered, [=]{createStoryChild(STORY_PLOT_EPISODE_OUTCOME, "OUT_", "", QPoint(0,100)); });
            connect(addEpisodeAction, &QAction::triggered, [=]{createStoryChild(STORY_PLOT_EPISODE, "SUBEP_", "", QPoint(0,100)); });

            contextMenu.addAction(addOutcomeAction);
            contextMenu.addAction(addEpisodeAction);
            break;
        case STORY_RESOLUTION:
            addEventAction = new QAction(tr("&Add Event"), this);    //add event group then event
            addStateAction = new QAction(tr("&Add State"), this);    //add state group then state

            connect(addEventAction, &QAction::triggered, [=]{createStoryChild(STORY_RESOLUTION_EVENT_GROUP, "EVENT", "", QPoint(0,100)); });
            connect(addStateAction, &QAction::triggered, [=]{createStoryChild(STORY_RESOLUTION_STATE_GROUP, "STATE", "", QPoint(0,100)); });

            contextMenu.addAction(addEventAction);
            contextMenu.addAction(addStateAction);
            break;
        case STORY_RESOLUTION_EVENT_GROUP:
            addEventAction = new QAction(tr("&Add Event"), this);

            connect(addEventAction, &QAction::triggered, [=]{createStoryChild(STORY_RESOLUTION_EVENT, "EV_", "", QPoint(0,100)); });

            contextMenu.addAction(addEventAction);
            break;
        case STORY_RESOLUTION_STATE_GROUP:
            addStateAction = new QAction(tr("&Add State"), this);

            connect(addStateAction, &QAction::triggered, [=]{createStoryChild(STORY_RESOLUTION_STATE_GROUP, "STA_", "", QPoint(0,100)); });

            contextMenu.addAction(addStateAction);
            break;
        default:
            break;
    }


    contextMenu.exec(event->globalPos());
}

} // namespace zodiac

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// HELPER
//

///
/// \brief Sets up and fires an animation for a property animation targeting a qreal.
///
/// \param animation    Animation to setup and fire.
/// \param targetValue  Target value of the animation.
/// \param startValue   Start value of the animation.
/// \param duration     Duration of the animation.
/// \param curve        Easing curve to use for the animation
///
static void fireAnimation(QPropertyAnimation& animation, qreal targetValue, qreal startValue, qreal duration,
                   const QEasingCurve& curve)
{
    // return early if the animation is already running or has reached its target
    if((startValue==targetValue) ||
        (animation.state() == QAbstractAnimation::Running)){
        return;
    }

    // setup and start the animation
    animation.setEasingCurve(curve);
    animation.setStartValue(startValue);
    animation.setDuration(qMax(0.,qAbs(targetValue-startValue)*duration));
    animation.start();
}

///
/// \brief Finds the closest plug to a given coordiante.
///
/// If the plug is an incoming one, it is only considered, if it doesn not have any edges connected yet.
///
/// \param pos          Position from which to measure the distance.
/// \param plugs        Vector of all potential plugs to search.
/// \param closest      Currently closest plug, can be <i>nullptr</i>.
///
/// \return             New closest plug or the one passed in.
///
static zodiac::Plug* findClosestPlug(const QPointF& pos, const QSet<zodiac::Plug*> &plugs, zodiac::Plug* closest)
{
    // find current minimal distance
    qreal minDistSq;
    if(closest){
        minDistSq = QVector2D((closest->getNormal()).toPointF()-pos).lengthSquared();
    } else {
        minDistSq = DBL_MAX;
    }

    // find closest plug to pos
    for(zodiac::Plug* plug : plugs){

        // skip incoming plugs that already have an edge connected
        if((plug->getDirection()==zodiac::PlugDirection::IN)&&(plug->getEdgeCount()>0)){
            continue;
        }

        qreal distSq = QVector2D((plug->getNormal()).toPointF()-pos).lengthSquared();
        if(distSq < minDistSq){
            minDistSq = distSq;
            closest = plug;
        }
    }
    return closest;
}

///
/// \brief Calculates the angular distance between two angles, always returning a positive result.
///
/// \param [in] alpha   First angle.
/// \param [in] beta    Second angle.
///
/// \return             Angular distance in postive radian [0 -> pi].
///
static qreal angularDistance(qreal alpha, qreal beta)
{
    static const qreal TWO_PI = M_PI*2;

    qreal result = qAbs(alpha-beta);
    if(result>M_PI){
        return TWO_PI-result;
    } else {
        return result;
    }
}
