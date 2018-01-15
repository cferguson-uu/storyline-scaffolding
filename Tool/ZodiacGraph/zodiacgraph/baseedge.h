//
//    ZodiacGraph - A general-purpose, circular node graph UI module.
//    Copyright (C) 2015  Clemens Sielaff
//
//    The MIT License
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy of
//    this software and associated documentation files (the "Software"), to deal in
//    the Software without restriction, including without limitation the rights to
//    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
//    of the Software, and to permit persons to whom the Software is furnished to do so,
//    subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
//

#ifndef ZODIAC_BASEEDGE_H
#define ZODIAC_BASEEDGE_H

///
/// \file baseedge.h
///
/// \brief Contains the definition of the zodiac::BaseEdge class.
///

#include <QGraphicsObject>
#include <QPen>
#include <QPropertyAnimation>

namespace zodiac {

class EdgeArrow;
class EdgeLabel;
class Scene;

///
/// \brief Baseclass for all edges in the Scene with most of the actual code for displaying the QGraphicsObject.
///
/// The Edge Label
/// --------------
///
/// The EdgeLabel of a BaseEdge is an interesting beast in that it is a free-floating QGraphicsItem -- controled, but
/// not part of a single BaseEdge.
/// Because of z-sorting, where edges are always <b>behind</b> Plug%s, but EdgeLabel%s should be <b>in front</b> of
/// them, I decided against using a standard parent-child relationship.
/// In Qt a parent item can not be stacked behing X while one of its child items is stacked before.
///
/// Also, not all (most most) BaseEdge have an EdgeLabel -- the DrawEdge being the sole exception.
///
/// The BaseEdge class does not grant direct access to the EdgeLabel,
/// however you can modify the EdgeLabel of a BaseEdge with the function setLabelText().
///
class BaseEdge : public QGraphicsObject
{
    Q_OBJECT

    ///
    /// \brief The EdgeArrow of the BaseEdge is a friend class so the arrow can forward mouse events to the edge.
    ///
    /// This is necessary, because we want the BaseEdge to handle mouse events that happen outside of its own shape.
    /// Therefore, if the EdgeArrow ignores the event, the BaseEdge will not receive it at all if it is not inside of
    /// the shape of the edge as well.
    /// But since we want the arrow to be a more exposed, more "clickable" part of the edge - we need to get those
    /// events over to the BaseEdge.
    ///
    friend class EdgeArrow;

    ///
    /// \brief The opacity of the EdgeLabel (and potential other secondary edge items).
    ///
    Q_PROPERTY(qreal secondaryOpacity MEMBER m_secondaryOpacity WRITE updateSecondaryOpacity)

public: // methods

    ///
    /// \brief Constructor.
    ///
    /// \param [in] scene   Scene containing this BaseEdge.
    ///
    explicit BaseEdge(Scene* scene);

    ///
    /// \brief Destructor.
    ///
    /// Takes care of deleting the EdgeLabel, if there is one.
    ///
    virtual ~BaseEdge();

    ///
    /// \brief Sets the text of this edge's EdgeLabel.
    ///
    /// To create a label, call setLabelText() with the text of the new label.
    /// This function can also be used to redefine the text of an existing EdgeLabel.
    /// And lastly, it is also used to delete the EdgeLabel by setting its text to "" (the empty string).
    ///
    /// \param [in] text    Text to set this label to. Remove an existing EdgeLabel by passing "" (the empty string).
    ///
    virtual void setLabelText(const QString& text);

    ///
    /// \brief Defines the visiblity of the BaseEdge.
    ///
    /// Also turns secondary edge items invisible.
    ///
    /// \param [in] visible If <i>true</i>, the item is made visible - if <i>false</i>, it becomes invisible.
    ///
    void setVisible(bool visible);

    ///
    /// \brief Moves the EdgeArrow along the edge to a given fraction of the arclength.
    ///
    /// \param [in] fraction Fraction of arclength at which to place the arrow.
    ///
    virtual void placeArrowAt(qreal fraction) = 0;

    ///
    /// \brief Applies style changes in the class' static members to this instance.
    ///
    /// Is part of the scene-wide cascade of %updateStyle()-calls after a re-styling of the ZodiacGraph.
    ///
    virtual void updateStyle();
    ///
    /// \brief The width of a default edge in the graph in pixels.
    ///
    /// \return Width of an edge.
    ///
    inline qreal getBaseWidth() {return m_width;}

    ///
    /// \brief Sets the width of all BaseEdges in the graph.
    ///
    /// \param [in] width    New edge width in pixels.
    ///
    inline void setBaseWidth(qreal width) {m_width=width; m_pen.setWidthF(m_width);}

    ///
    /// \brief The line color of a default edge in the graph.
    ///
    /// \return Color of an edge.
    ///
    inline QColor getBaseColor() {return m_color;}

    ///
    /// \brief Sets a new line color of all BaseEdges in the graph.
    ///
    /// \param [in] color    New edge color.
    ///
    inline void setBaseColor(const QColor& color) {m_color=color; m_pen.setColor(color);}

    ///
    /// \brief Duration of the secondary edge items to fade-in when the mouse enters the edge.
    ///
    /// \return Secondary items fade-in duration.
    ///
    inline qreal getSecondaryFadeInDuration() {return m_secondaryFadeInDuration;}

    ///
    /// \brief Sets a new duration of the secondary edge items to fade-in when the mouse enters the edge.
    ///
    /// \param [in] duration Duration of the fade-in milliseconds.
    ///
    inline void setSecondaryFadeInDuration(qreal duration) {m_secondaryFadeInDuration=duration;}

    ///
    /// \brief Duration of the secondary edge items to fade-out after the mouse has left the edge.
    ///
    /// \return Secondary items fade-out duration.
    ///
    inline qreal getSecondaryFadeOutDuration() {return m_secondaryFadeOutDuration;}

    ///
    /// \brief Sets a new duration of the secondary edge items to fade-out after the mouse has left the edge.
    ///
    /// \param [in] duration Duration of the fade-in milliseconds.
    ///
    inline void setSecondaryFadeOutDuration(qreal duration) {m_secondaryFadeOutDuration=duration;}

    ///
    /// \brief Ease animation curve for the secondary edge items' fade-in animation.
    ///
    /// \return Fade-in easing curve.
    ///
    inline QEasingCurve getSecondaryFadeInCurve() {return m_secondaryFadeInCurve;}

    ///
    /// \brief Sets a new ease animation curve for the secondary edge items' fade-in animation.
    ///
    /// \param [in] curve    New animation curve for the fade-in animation.
    ///
    inline void setSecondaryFadeInCurve(QEasingCurve curve) {m_secondaryFadeInCurve=curve;}

    ///
    /// \brief Ease animation curve for the secondary edge items' fade-out animation.
    ///
    /// \return Fade-out easing curve.
    ///
    inline QEasingCurve getSecondaryFadeOutCurve() {return m_secondaryFadeOutCurve;}

    ///
    /// \brief Sets a new ease animation curve for the secondary edge items' fade-out animation.
    ///
    /// \param [in] curve    New animation curve for the fade-out animation.
    ///
    inline void setSecondarylFadeOutCurve(QEasingCurve curve) {m_secondaryFadeOutCurve=curve;}

protected: // methods

    ///
    /// \brief Rectangular outer bounds of the item, used for redraw testing.
    ///
    /// \return Boundary rectangle of the item.
    ///
    QRectF boundingRect() const;

    ///
    /// \brief Paints this item.
    ///
    /// \param [in] painter  Painter used to paint the item.
    /// \param [in] option   Provides style options for the item.
    /// \param [in] widget   Optional widget that this item is painted on.
    ///
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    ///
    /// \brief Exact boundary of the item used for collision detection among other things.
    ///
    /// \return Shape in local coordinates.
    ///
    QPainterPath shape() const;

    ///
    /// \brief Called when the mouse enteres the shape of the edge.
    ///
    /// \param [in] event    Qt event object
    ///
    void hoverEnterEvent(QGraphicsSceneHoverEvent * event);

    ///
    /// \brief Called when the mouse leaves the shape of the edge.
    ///
    /// \param [in] event    Qt event object
    ///
    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);

    ///
    /// \brief Sets a new opacity value [0-1] for the secondary edge items.
    ///
    /// Is called automatically by the two QPropertyAnimations of this BaseEdge.
    ///
    /// The EdgeLabel is currently the only secondary item, but I left the naming general in case that changes.
    ///
    /// \param [in] opacity  New opacity for the secondary edge items.
    ///
    void updateSecondaryOpacity(qreal opacity);

    ///
    /// \brief Updates the shape of this BaseEdge.
    ///
    /// Is pure virtual in BaseEdge.
    ///
    virtual void updateShape() = 0;

protected: // members

    ///
    /// \brief Scene containing this BaseEdge.
    ///
    Scene* m_scene;

    ///
    /// \brief The EdgeArrow of this BaseEdge, is a Qt child item.
    ///
    EdgeArrow* m_arrow;

    ///
    /// \brief Path used to draw the edge.
    ///
    QPainterPath m_path;

    ///
    /// \brief Fade-in animation of this edge's secondary items.
    ///
    QPropertyAnimation m_secondaryFadeIn;

    ///
    /// \brief Fade-out animation of this edge's secondary items.
    ///
    QPropertyAnimation m_secondaryFadeOut;

    ///
    /// \brief Current opacity of this edge's secondary items.
    ///
    qreal m_secondaryOpacity;

protected: // static members
/*
    ///
    /// \brief Width of the edge line.
    ///
    static qreal s_width;

    ///
    /// \brief Color of the edge line.
    ///
    static QColor s_color;

    ///
    /// \brief Duration of the secondary items' fade-in animation in milliseconds.
    ///
    static qreal s_secondaryFadeInDuration;

    ///
    /// \brief Duration of the secondary items' fade-out animation in milliseconds.
    ///
    static qreal s_secondaryFadeOutDuration;

    ///
    /// \brief Easing animation curve for the secondary items fade-in animation.
    ///
    static QEasingCurve s_secondaryFadeInCurve;

    ///
    /// \brief Easing animation curve for the secondary items fade-out animation.
    ///
    static QEasingCurve s_secondaryFadeOutCurve;

    ///
    /// \brief Pen used to draw all DrawEdge%s.
    ///
    static QPen s_pen;*/

    qreal m_width;
    QColor m_color;
    qreal m_secondaryFadeInDuration;
    qreal m_secondaryFadeOutDuration;
    QEasingCurve m_secondaryFadeInCurve;
    QEasingCurve m_secondaryFadeOutCurve;
    QPen BaseEdge::m_pen;

private: // members

    ///
    /// \brief Label of this BaseEdge, can be <i>nullptr</i>.
    ///
    EdgeLabel* m_label;
};

} // namespace zodiac

#endif // ZODIAC_BASEEDGE_H
