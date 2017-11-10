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

#ifndef ZODIAC_NODEHANDLE_H
#define ZODIAC_NODEHANDLE_H

/// \file nodehandle.h
///
/// \brief Contains the definition of the zodiac::NodeHandle class.
///

#include <QObject>
#include <QHash>

#include "plughandle.h"
#include "node.h"

namespace zodiac {

//class Node;
class Plug;
class SceneHandle;
//struct NodeCommand;

///
/// \brief A handle object for a zodiac::Node.
///
/// Is a thin wrapper around a pointer but with a much nicer, outward facing interface than the real Node.
///
/// See \ref zodiac::SceneHandle "SceneHandle" for more details on handles.
///
class NodeHandle : public QObject
{

    Q_OBJECT

public: // methods

    ///
    /// \brief Constructor.
    ///
    /// \param [in] node    Node to manage through this handle
    ///
    explicit NodeHandle(Node* node = nullptr);

    ///
    /// \brief Copy constructor.
    ///
    /// \param [in] other   Other NodeHandle to copy.
    ///
    NodeHandle(const NodeHandle& other)
        : NodeHandle(other.data()) {}

    ///
    /// \brief Assignment operator.
    ///
    /// \param [in] other   Other NodeHandle to copy from.
    ///
    /// \return             This.
    ///
    NodeHandle& operator = (const NodeHandle& other);

    ///
    /// \brief Equality operator.
    ///
    /// \param [in] other   Other NodeHandle to test against.
    ///
    /// \return             <i>true</i> if both handles handle the same object -- <i>false</i> otherwise.
    ///
    bool operator == (const NodeHandle& other) const {return other.data() == data();}

    ///
    /// \brief Direct pointer access.
    ///
    /// \return The pointer managed by this handle.
    ///
    inline Node* data() const {return m_node;}

    ///
    /// \brief Used for testing, whether the handle is still alive or not.
    ///
    /// \return <i>true</i>, if the NodeHandle is still managing an existing Node -- <i>false</i> otherwise.
    ///
    inline bool isValid() const {return m_isValid;}

    ///
    /// \brief Only Node%s with no connections can be removed.
    ///
    /// \return <i>true</i> if the Node could be removed -- <i>false</i> otherwise.
    ///
    bool isRemovable() const;

    ///
    /// \brief Tries to remove the Node managed by this handler.
    ///
    /// \return <i>true</i> if the Node could be removed -- <i>false</i> otherwise.
    ///
    bool remove();

    ///
    /// \brief Queries and returns the id of the Node.
    ///
    /// \return Unique id of the Node.
    ///
    const QUuid &getId() const;

    ///
    /// \brief Queries and returns the type of the Node.
    ///
    /// \return Type of the Node.
    ///
    const NodeType getType() const;

    ///
    /// \brief Queries and returns the name of the Node.
    ///
    /// \return Name of the Node.
    ///
    QString getName() const;

    ///
    /// \brief Sets s new display name for the managed Node.
    ///
    /// \param [in] name    New display name of this Node.
    ///
    void rename(const QString& name);

    ///
    /// \brief Queries and returns the description of the Node.
    ///
    /// \return Description of the Node.
    ///
    QString getDescription() const;

    ///
    /// \brief Sets a new description for the managed Node.
    ///
    /// \param [in] description    New description of this Node.
    ///
    void changeDescription(const QString& description);

    ///
    /// \brief Define a new color used to fill the label background.
    ///
    /// \param [in] color    New color used to fill the label background.
    ///
    void setLabelBackgroundColor(const QColor& color);

    ///
    /// \brief The command blocks part of the node.
    ///
    /// \return QHashes of command blocks.
    ///
    QHash<QUuid, NodeCommand> getOnUnlockList();
    QHash<QUuid, NodeCommand> getOnFailList();
    QHash<QUuid, NodeCommand> getOnUnlockedList();

    ///
    /// \brief Adds new element to command block
    ///
    ///
    /// \param [in] key command name
    ///
    void addOnUnlockCommand(const QUuid& key, const QString& value, const QString& description);
    void addOnFailCommand(const QUuid& key, const QString& value, const QString& description);
    void addOnUnlockedCommand(const QUuid& key, const QString& value, const QString& description);

    ///
    /// \brief Removes new element from command block
    ///
    ///
    /// \param [in] key command name
    ///
    void removeOnUnlockCommand(const QUuid& key);
    void removeOnFailCommand(const QUuid& key);
    void removeOnUnlockedCommand(const QUuid& key);

    ///
    /// \brief get parameter element from command block
    ///
    ///
    /// \param [in] cmdKey command name
    /// \param [in] paramKey parameter name
    ///
    QString getParameterFromOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey);
    QString getParameterFromOnFailCommand(const QUuid& cmdKey, const QString& paramKey);
    QString getParameterFromOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey);

    ///
    /// \brief Add parameter element to command block
    ///
    ///
    /// \param [in] key parameter name
    /// \param [in] value parameter value
    ///
    void addParameterToOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value);
    void addParameterToOnFailCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value);
    void addParameterToOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value);

    ///
    /// \brief Delete parameter element to command block
    ///
    ///
    /// \param [in] key parameter name
    ///
    void removeParameterFromOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey);
    void removeParameterFromOnFailCommand(const QUuid& cmdKey, const QString& paramKey);
    void removeParameterFromOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey);

    ///
    /// \brief Delete all parameter elements from command block
    ///
    ///
    void removeAllParametersFromOnUnlockCommand(const QUuid& cmdKey);
    void removeAllParametersFromOnFailCommand(const QUuid& cmdKey);
    void removeAllParametersFromOnUnlockedCommand(const QUuid& cmdKey);

    ///
    /// \brief edit parameter element in command block
    ///
    ///
    /// \param [in] key parameter name
    /// \param [in] value new parameter value
    ///
    void editParameterInOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value);
    void editParameterInOnFailCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value);
    void editParameterInOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value);

    ///
    /// \brief set the locked/unlocked status of the node
    ///
    ///
    /// \param [in] bool to set status
    ///
    void setLockedStatus(bool status);

    ///
    /// \brief get the locked/unlocked status of the node
    ///
    bool getLockedStatus();

    ///
    /// \brief Gets the type of story node
    ///
    ///
    StoryNodeType getStoryNodeType();

    ///
    /// \brief Returns the prefix for the story node
    ///
    ///
    QString getStoryNodePrefix();

    ///
    /// \brief Sets the expansion of the node only if it is not currently forced open.
    ///
    /// \param [in] newState New state to expand.
    ///
    void softSetExpansion(NodeExpansion newState);
    ///
    /// \brief Creates a new incoming Plug for the managed Node.
    ///
    /// \param [in] name        The proposed name of the plug.
    ///
    /// \return                 Handle of the created Plug.
    ///
    PlugHandle createIncomingPlug(const QString& name);

    ///
    /// \brief Creates a new outgoing Plug for the managed Node.
    ///
    /// \param [in] name        The proposed name of the plug.
    ///
    /// \return                 Handle of the created Plug.
    ///
    PlugHandle createOutgoingPlug(const QString& name);

    ///
    /// \brief Returns handles for all Plugs of the managed Node.
    ///
    /// \return All Plugs of the managed Node.
    ///
    QList<PlugHandle> getPlugs() const;

    ///
    /// \brief Returns a single Plug of the managed Node by name.
    ///
    /// \param [in] name    Name of the requested Plug.
    ///
    /// \return             PlugHandle for the requested Plug, or an invalid one -- if there is no Plug with the name.
    ///
    PlugHandle getPlug(const QString& name) const;

    ///
    /// \brief setSelected
    ///
    /// \param [in] isSelected  <i>true</i> to select the Node -- <i>false</i> to unselect it.
    ///
    void setSelected(bool isSelected);

    ///
    /// \brief SceneHandle of the Scene containing the managed Node.
    ///
    /// \return Scene that contains the managed Node.
    ///
    SceneHandle getScene() const;

    ///
    /// \brief Returns the position of the Node in the Scene.
    ///
    /// \return The position of the Node in the Scene.
    ///
    QPointF getPos() const;

    ///
    /// \brief Returns the position of the Node in the Scene.
    ///
    /// \param [in] x   The position of the Node along the x axis.
    /// \param [in] y   The position of the Node along the y axis.
    /// \param [in] updateChildren   Should the children of the node be updated?
    ///
    void setPos(qreal x, qreal y, bool updateChildren = false, bool updateParents = false);

    ///
    /// \brief Fill color of an idle Node core.
    ///
    /// \return Idle fill color.
    ///
    const QColor& getIdleColor();

    ///
    /// \brief Sets a new fill color of idle Node cores.
    ///
    /// \param [in] color   New idle color of the node core.
    ///
    void setIdleColor(const QColor& color);

    ///
    /// \brief Fill color of a selected Node core.
    ///
    /// \return Selected fill color.
    ///
    const QColor& getSelectedColor();

    ///
    /// \brief Sets a new fill color of selected Node cores.
    ///
    /// \param [in] color   New selected color of the node core.
    ///
    void setSelectedColor(const QColor& color);

    ///
    /// \brief The color used to draw the outline around the node core.
    ///
    /// \return Core outline color.
    ///
    const QColor& getOutlineColor();

    ///
    /// \brief Sets a new color used to draw the outline around the node core.
    ///
    /// \param [in] color   New core outline color.
    ///
    void setOutlineColor(const QColor& color);

    ///
    /// \brief Sets a new width of the outline around the node core in pixels.
    ///
    /// \param [in] width   New core outline width;
    ///
    void setOutlineWidth(qreal width);

signals:

    ///
    /// \brief Emitted, when the \ref zodiac::View::s_activationKey "activation key" is pressed while the managed Node
    /// is selected.
    ///
    void nodeActivated();

    ///
    /// \brief Emitted, when the display name of the Node was changed.
    ///
    /// \param [out] name   New display name of the Node.
    ///
    void nodeRenamed(const QString& name);

    ///
    /// \brief Emitted, when the \ref zodiac::View::s_removalButton "removal button" is pressed on the managed Node.
    ///
    void removalRequested();

    ///
    /// \brief Emitted, when context menu is used to create another story node
    ///
    void createStoryChild(zodiac::StoryNodeType, QString, QString, QPoint&);

    ///
    /// \brief Emitted, when context menu is used to clink narrative nodes
    ///
    void openLinker();

    ///
    /// \brief Is emitted, when an incoming plug of the managed Node has been connected to an outgoing plug of another
    /// Node.
    ///
    /// \param [out] myInput        The input plug of the managed Node.
    /// \param [out] otherOutput    The output plug of the source Node.
    ///
    void inputConnected(zodiac::PlugHandle myInput, zodiac::PlugHandle otherOutput);

    ///
    /// \brief Is emitted, when an outgoing plug of the managed Node has been connected to an incoming plug of another
    /// Node.
    ///
    /// \param [out] myOutput   The output plug of the managed Node.
    /// \param [out] otherInput The input plug of the target Node.
    ///
    void outputConnected(zodiac::PlugHandle myOutput, zodiac::PlugHandle otherInput);

    ///
    /// \brief Is emitted, when a connection from an incoming plug of the managed Node was removed.
    ///
    /// \param [out] myInput        The input plug of the managed Node.
    /// \param [out] otherOutput    The output plug of the source Node.
    ///
    void inputDisconnected(zodiac::PlugHandle myInput, zodiac::PlugHandle otherOutput);

    ///
    /// \brief Is emitted, when a connection from an outgoing plug of the managed Node was removed.
    ///
    /// \param [out] myOutput   The output plug of the managed Node.
    /// \param [out] otherInput The input plug of the target Node.
    ///
    void outputDisconnected(zodiac::PlugHandle myOutput, zodiac::PlugHandle otherInput);

private: // methods

    ///
    /// \brief Connects the handle to its managed object.
    ///
    void connectSignals();

private slots:

    ///
    /// \brief Passes the 'inputConnected'-signal from the managed Node.
    ///
    /// \param [in] myInput     The input Plug of the managed Node.
    /// \param [in] otherOutput The output Plug of the source Node.
    ///
    void passInputConnected(Plug* myInput, Plug* otherOutput);

    ///
    /// \brief Passes the 'outputConnected'-signal from the managed Node.
    ///
    /// \param [in] myOutput    The output Plug of the managed Node.
    /// \param [in] otherInput  The input Plug of the target Node.
    ///
    void passOutputConnected(Plug* myOutput, Plug* otherInput);

    ///
    /// \brief Passes the 'inputDisconnected'-signal from the managed Node.
    ///
    /// \param [in] myInput     The input Plug of the managed Node.
    /// \param [in] otherOutput The output Plug of the source Node.
    ///
    void passInputDisconnected(Plug* myInput, Plug* otherOutput);

    ///
    /// \brief Passes the 'outputDisconnected'-signal from the managed Node.
    ///
    /// \param [in] myOutput    The output Plug of the managed Node.
    /// \param [in] otherInput  The input Plug of the target Node.
    ///
    void passOutputDisconnected(Plug* myOutput, Plug* otherInput);

    ///
    /// \brief Called, when the mangaged Node was destroyed.
    ///
    void nodeWasDestroyed();

private: // member

    ///
    /// \brief Managed node.
    ///
    Node* m_node;

    ///
    /// \brief Validity flag.
    ///
    bool m_isValid;

};

} // namespace zodiac

///
/// \brief Returns the hash of a NodeHandle instance.
///
/// The hash is calculated by taking the address of the Node-pointer.
/// This is also, why the pointer adress in the handle is never changed or set to <i>nullptr</i>.
///
/// \param [in] key NodeHandle instance to hash.
///
/// \return         Unique identifier of a NodeHandle that can be used to determine equality.
///
inline uint qHash(const zodiac::NodeHandle& key)
{
    return qHash(size_t(key.data()));
}

#endif // ZODIAC_NODEHANDLE_H
