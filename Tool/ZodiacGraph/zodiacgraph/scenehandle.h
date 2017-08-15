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

#ifndef ZODIAC_SCENEHANDLE_H
#define ZODIAC_SCENEHANDLE_H

/// \file scenehandle.h
///
/// \brief Contains the definition of the zodiac::SceneHandle class.
///

#include <QObject>
#include <QHash>
#include <QUuid>

#include "nodehandle.h"

namespace zodiac {

class PlugHandle;
class Scene;

///
/// \brief A handle object for a zodiac::Node.
///
/// Is a thin wrapper around a pointer but with a much nicer, outward facing interface than the real Node.
///
/// SceneHandle, NodeHandle and PlugHandle should be the main interface for working with a Zodiac Graph.
/// They are designed for ease of use and it should be hard to leave the graph in an inconsistent state using the
/// provided methods.
///
/// There is however a caveat in using handles.<br>
/// It is conceivable (if unlikely) that (in a multi-threated environment) the deletion of a managed object occurs in
/// the middle of the execution of a handle method <b>after</b> the initial validation has succeeded.
/// Consequently, the handle will fail to operate on a dangling pointer.
/// In the future, it might be possible to use weak references and shared pointers to make sure that the managed pointer
/// is always alive when calling a handle method, but since most Zodiac-classes are active participants in Qt's
/// parent-child mechanism, using shared pointers on them is a dangerous thing to do.<br>
/// Keeping a shared pointer on a QGraphicsItem after its QGraphicsScene was removed will cause a segmentation error to
/// occur and although you could completely separate the logical and graphical code (with a complete overhaul of the
/// architecture and rewrite of the code), I will leave this exercise for subsequent versions. <br>
/// Instead each handle listens to its respective QObject and sets its internal pointer to <i>nullptr</i>, as soon as it
/// receives the "deleted()" signal.
/// In the meantime, this approach should cover 99% of all use-cases, as long as you don't access the graph using
/// multiple threads at once.
///
class SceneHandle : public QObject
{

    Q_OBJECT

public: // methods

    ///
    /// \brief Constructor.
    ///
    /// \param [in] scene   Scene to manage through this handle
    ///
    explicit SceneHandle(Scene* scene = nullptr);

    ///
    /// \brief Copy constructor.
    ///
    /// \param [in] other   Other SceneHandle to copy.
    ///
    SceneHandle(const SceneHandle& other)
        : SceneHandle(other.data()) {}

    ///
    /// \brief Assignment operator.
    ///
    /// \param [in] other   Other SceneHandle to copy from.
    ///
    /// \return             This.
    ///
    SceneHandle& operator = (const SceneHandle& other);

    ///
    /// \brief Equality operator.
    ///
    /// \param [in] other   Other SceneHandle to test against.
    ///
    /// \return             <i>true</i> if both handles handle the same object -- <i>false</i> otherwise.
    ///
    bool operator == (const SceneHandle& other) const {return other.data() == data();}

    ///
    /// \brief Direct pointer access.
    ///
    /// \return The pointer managed by this handle.
    ///
    inline Scene* data() const {return m_scene;}

    ///
    /// \brief Used for testing, whether the handle is still alive or not.
    ///
    /// \return <i>true</i>, if the SceneHandle is still managing an existing Scene -- <i>false</i> otherwise.
    ///
    inline bool isValid() const {return m_isValid;}

    ///
    /// \brief Creates and adds a new Node to the zodiac graph.
    ///
    /// \param [in] name    Display name of the new Node.
    /// \param [in] story   Creates a story node if true, otherwise narrative
    /// \param [in] uuid    (optional) The unique identifier of this Node.
    ///
    /// \return             Handle of the new Node.
    ///
    NodeHandle createNode(const QString& name, bool story, const QUuid& uuid = QUuid());

    ///
    /// \brief Returns all Node%s managed by the Scene.
    ///
    /// \return All Node%s managed by the Scene.
    ///
    QList<NodeHandle> getNodes() const;

    ///
    /// \brief Clears the selection of the Scene.
    ///
    void deselectAll() const;

signals:

    ///
    /// \brief Emitted when the selection in the Scene has changed.
    ///
    /// \param [out] selection  Handles to all selected Node%s.
    ///
    void selectionChanged(QList<zodiac::NodeHandle> selection);

private: // methods

    ///
    /// \brief Connects the handle to its managed object.
    ///
    void connectSignals();

private slots:

    ///
    /// \brief Called, when the selection of the Scene was changed.
    ///
    void updateSelection();

    ///
    /// \brief Called, when the mangaged Scene was destroyed.
    ///
    void sceneWasDestroyed();

private: // member

    ///
    /// \brief Managed scene.
    ///
    Scene* m_scene;

    ///
    /// \brief Validity flag.
    ///
    bool m_isValid;
};

} // namespace zodiac

///
/// \brief Returns the hash of a SceneHandle instance.
///
/// The hash is calculated by taking the address of the Scene-pointer.
/// This is also, why the pointer adress in the handle is never changed or set to <i>nullptr</i>.
///
/// \param [in] key SceneHandle instance to hash.
///
/// \return         Unique identifier of a SceneHandle that can be used to determine equality.
///
inline uint qHash(const zodiac::SceneHandle& key)
{
    return qHash(size_t(key.data()));
}

#endif // ZODIAC_SCENEHANDLE_H
