#include "baseedge.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "edgearrow.h"
#include "edgelabel.h"
#include "utils.h"
#include "scene.h"

namespace zodiac {

BaseEdge::BaseEdge(Scene* scene, QColor color, bool useArrow)
    : QGraphicsObject(nullptr)
    , m_scene(scene)
    , m_arrow(nullptr)
    , m_path(QPainterPath())
    , m_secondaryOpacity(0.)
    , m_color(color)
    , m_label(nullptr)
{
    m_scene->addItem(this);

    m_width = 2.5;
    //m_color = QColor("#cc5d4e");
    m_secondaryFadeInDuration = 200.;
    m_secondaryFadeOutDuration = 400.;
    m_secondaryFadeInCurve = QEasingCurve::OutQuart;
    m_secondaryFadeOutCurve = QEasingCurve::InCubic;
    m_pen = QPen(QBrush(m_color), m_width, Qt::SolidLine, Qt::RoundCap);

    // edges are always behind nodes
    setZValue(zStack::EDGE);

    // edges deform too much to be cached meaningfully
    setCacheMode(NoCache);

    // by default, edges react to hover events
    setAcceptHoverEvents(true);

    // construct edge arrow
    if(useArrow)
        m_arrow = new EdgeArrow(this, color);
    else
        m_arrow = new EdgeArrow(this, QColor("transparent"));

    // set up animations
    m_secondaryFadeIn.setTargetObject(this);
    m_secondaryFadeIn.setPropertyName("secondaryOpacity");
    m_secondaryFadeIn.setEndValue(1.);
    m_secondaryFadeOut.setTargetObject(this);
    m_secondaryFadeOut.setPropertyName("secondaryOpacity");
    m_secondaryFadeOut.setEndValue(0.);
}

BaseEdge::~BaseEdge()
{
    setLabelText("");
}

void BaseEdge::setLabelText(const QString& text)
{
    if(text.isEmpty()){
        // remove an existing label
        if(m_label){
            m_arrow->setLabel(nullptr);
            m_scene->removeItem(m_label);
            delete m_label;
            m_label = nullptr;
        }
    } else {
        // create a new or modify an existing label
        if(!m_label){
            m_label = new EdgeLabel();
            m_scene->addItem(m_label);
            m_arrow->setLabel(m_label);
        }
        m_label->setText(text);
    }
}

void BaseEdge::setVisible(bool visible)
{
    // if you turn invisible, make sure all secondaries are invisible too
    if(!visible){
        m_secondaryFadeIn.stop(); // in case the secondaries are currently fading in
        updateSecondaryOpacity(0.);
    }
    return QGraphicsObject::setVisible(visible);
}

void BaseEdge::updateStyle()
{
    if(m_label){
        m_label->updateStyle();
    }
    placeArrowAt(0.5);
    update();
}

QRectF BaseEdge::boundingRect() const
{
    qreal overdraw = m_width/2.;
    return m_path.boundingRect().marginsAdded(QMarginsF(overdraw,overdraw,overdraw,overdraw));
}

void BaseEdge::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /* widget */)
{
    painter->setClipRect(option->exposedRect);
    painter->setPen(m_pen);
    painter->drawPath(m_path);
}

QPainterPath BaseEdge::shape() const
{
    return QPainterPathStroker(m_pen).createStroke(m_path);
}

void BaseEdge::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    if(m_label){
        m_secondaryFadeIn.setStartValue(m_secondaryOpacity);
        m_secondaryFadeIn.setDuration((1.0-m_secondaryOpacity)*m_secondaryFadeInDuration);
        m_secondaryFadeIn.setEasingCurve(m_secondaryFadeInCurve);
        m_secondaryFadeIn.start();
    }
    QGraphicsObject::hoverEnterEvent(event);
}

void BaseEdge::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    if(m_label){
        m_secondaryFadeOut.setStartValue(m_secondaryOpacity);
        m_secondaryFadeOut.setDuration(m_secondaryOpacity*m_secondaryFadeOutDuration);
        m_secondaryFadeOut.setEasingCurve(m_secondaryFadeOutCurve);
        m_secondaryFadeOut.start();
    }
    QGraphicsObject::hoverLeaveEvent(event);
}

void BaseEdge::updateSecondaryOpacity(qreal opacity)
{
    if(m_label){
        m_label->setOpacity(opacity);
    }
    m_secondaryOpacity=opacity;
}

void BaseEdge::setBaseColor(const QColor& color)
{
    m_color=color;
    m_pen.setColor(color);
    if(m_arrow->getArrowColor() != QColor("transparent"))
        m_arrow->setArrowColor(color);
}
} // namespace zodiac
