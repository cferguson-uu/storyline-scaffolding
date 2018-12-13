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

#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include <QObject>
#include <QHash>
#include <QUndoStack>
#include <QSet>

#include "zodiacgraph/plugedge.h"
#include "zodiacgraph/nodehandle.h"
#include "zodiacgraph/scenehandle.h"

#include "saveandload.h"
#include "linkerwindow.h"
#include "analyticshandler.h"
#include "narrativefilesorter.h"

class NodeCtrl;
class PropertyEditor;
namespace zodiac {
    class Scene;
}

///
/// \brief Controller Manager managing all NodeCtrls.
///
/// This is the main class for controlling the business logic of the application.
///
class MainCtrl : public QObject
{
    Q_OBJECT

public: // methods

    ///
    /// \brief Constructor.
    ///
    /// \param [in] parent          Qt parent.
    /// \param [in] scene           Handle of a zodiac::Scene.
    /// \param [in] propertyEditor  Property editor.
    /// \param [in] analyticsHandler  Handler for analytics
    /// \param [in] newStoryNodeAction  Action for creating story node
    /// \param [in] undoStack       Undo stack for undo and redo.
    ///
    explicit MainCtrl(QObject *parent, zodiac::Scene* scene, PropertyEditor* propertyEditor, AnalyticsHandler* analyticsHandler, QAction* newStoryNodeAction, QUndoStack *undoStack);

    ///
    /// \brief Creates a new node in the graph.
    ///
    /// \param [in] name    Name of the new node or empty to create a default name.
    ///
    /// \return Control for new node.
    ///
    NodeCtrl* createNode(zodiac::StoryNodeType storyType, const QString& name = "", const QString& description = "", bool load = false);

    ///
    /// \brief Deletes an existing node from the graph.
    ///
    /// \param [in] node    Node to delete.
    ///
    /// \return             <i>true</i> if the node was removed -- <i>false</i> otherwise.
    ///
    bool deleteNode(NodeCtrl* node);

    ///
    /// \brief Returns the NodeCtrl that manages a given NodeHandle.
    ///
    /// If there is no corresponding NodeCtrl known to this manager, the <i>nullptr</i> is returned.
    ///
    /// \param [in] handle  NodeHandle for which to find the corresponding NodeCtrl.
    ///
    /// \return             Corresponding NodeCtrl or the <i>nullptr</i> if none was found.
    ///
    inline NodeCtrl* getCtrlForHandle(zodiac::NodeHandle handle) const { return m_nodes.value(handle, nullptr); }

    ///
    /// \brief Must be called before closing the application.
    ///
    /// \return             <i>true</i> if the manager was shut down successfully -- <i>false</i> if the user objected.
    ///
    bool shutdown();

    ///
    /// \brief Changes the visibility of links between narrative nodes
    ///
    /// \param [in] show  Boolean of whether to show or hide links
    ///
    void changeReqVisibility(bool show);

    ///
    /// \brief Changes the visibility of links between narrative/story and story nodes
    ///
    /// \param [in] show  Boolean of whether to show or hide links
    /// \param [in] nodeType  node type of which group of nodes to target
    ///
    void changeStoryVisibility(bool show, zodiac::NodeType type);

public slots:

    ///
    /// \brief Creates a new node in the graph selects and activates it.
    ///
    void createDefaultNode();
    ///
    /// \brief Creates a new story graph
    ///
    NodeCtrl* createStoryNode(NodeCtrl *parent, zodiac::StoryNodeType type, QString name, QString description, QPoint &pos, bool relative = true, bool load = false);
    ///
    /// \brief Creates a new story graph
    ///
    void createStoryGraph(QString storyName = "Story");

    ///
    /// \brief Save the story graph to a JSON file
    ///
    void loadStoryGraph();

    ///
    /// \brief Loads the story graph from a JSON file
    ///
    void saveStoryGraph();

    ///
    /// \brief Save the narrative graph to a JSON file
    ///
    void loadNarrativeGraph();

    ///
    /// \brief Loads the narrative graph from a JSON file
    ///
    void saveNarrativeGraph();

    ///
    /// \brief Shows the window for linking nodes
    ///
    void showLinkerWindow(zodiac::NodeHandle &node);

    ///
    /// \brief Shows a node and associated ones being unlocked
    ///
    void unlockNode(QString nodeName);

    ///
    /// \brief Checks that a graph is loaded before starting a process
    ///
    void checkGraphLoaded(zodiac::NodeType type);

    ///
    /// \brief Updates the analytics collapsible
    ///
    void updateAnalyticsProperties();

    void linkNarrativeNodes(zodiac::NodeHandle &node, QList<zodiac::NodeHandle> &nodeList, QList<zodiac::NodeHandle> &inverseNodeList);

    void linkStoryNodes(zodiac::NodeHandle &node, QList<zodiac::NodeHandle> &nodeList);

    void spaceOutNarrative(QVector<QString> fileNames);

private slots:

    ///
    /// \brief Called when the selection in the managed scene has changed.
    ///
    /// \param [in] selection   Handles to all selected nodes.
    ///
    void selectionChanged(QList<zodiac::NodeHandle> selection);

private: // members

    ///
    /// \brief Handle to the zodiac::Scene representing the graph.
    ///
    zodiac::SceneHandle m_scene;

    ///
    /// \brief The Property Editor widget.
    ///
    PropertyEditor* m_propertyEditor;

    ///
    /// \brief NodeCtrls managed by this manager (but deleted through Qt).
    ///
    QHash<zodiac::NodeHandle, NodeCtrl*> m_nodes;

    ///
    /// \brief Ever increasing index value for default names of the nodes in this manager.
    ///
    uint m_nodeIndex;

    ///
    /// \brief Handle saving and loading story and narrative graphs
    ///
    saveandload m_saveAndLoadManager;

    ///
    /// \brief For undoing and redoing actions
    ///
    QUndoStack *m_pUndoStack;

    ///
    /// \brief For linking narrative and story nodes
    ///
    LinkerWindow *m_linkWindow = nullptr;

    ///
    /// \brief For linking narrative and story nodes
    ///
    NarrativeFileSorter *m_narrativeSorter = nullptr;

    ///
    /// \brief For analytics, "lighting up graph" etc.
    ///
    AnalyticsHandler *m_analytics;

    ///
    /// \brief For creating story node, only used to enable and disable
    ///
    QAction *m_createStoryAction;

    ///
    /// \brief handle the setting items (characters, locations, times)
    ///
    void loadSettingItem(NodeCtrl *parentNode, QList<SettingItem> items, zodiac::StoryNodeType childType);

    ///
    /// \brief handle the theme items (events, goals)
    ///
    void loadThemeItem(NodeCtrl *parentNode, QList<EventGoal> items, zodiac::StoryNodeType childType);

    ///
    /// \brief handle the plot items (episodes)
    ///
    void loadEpisodes(NodeCtrl *parentNode, QList<Episode> episodes);

    ///
    /// \brief handle the resolution items (events, states)
    ///
    void loadResolution(NodeCtrl *resolutionNode, QList<EventGoal> events, QList<SimpleNode> states);

    void spaceOutStory();

    float spaceOutChildNodes(zodiac::NodeHandle &node, float &xPos, float &yPos);

    void saveSettingItem(zodiac::NodeHandle &settingGroup);
    void saveThemeItem(zodiac::NodeHandle &parent, EventGoal *parentItem = nullptr);
    void savePlotItem(zodiac::NodeHandle &parent, Episode *parentItem = nullptr);
    void saveResolution(zodiac::NodeHandle &parent);

    void loadNarrativeCommands(NarNode &loadedNode, NodeCtrl* sceneNode);
    void loadRequirements(NarRequirements &requirements, zodiac::PlugHandle &parentReqOutPlug, QList<NodeCtrl*> &sceneNodes, QList<zodiac::NodeHandle> &currentNarSceneNodes);
    void loadStoryTags(NodeCtrl* narrativeNode, QList<QString> storyTags);

    void spaceOutFullNarrative();
    void spaceOutNarrativeChildren(NodeCtrl* sceneNode, float &maxY, float &maxX);

    void saveCommands(NarNode *narNode, zodiac::NodeHandle &sceneNode);
    void saveRequirements(NarNode *narNode, zodiac::NodeHandle &sceneNode);
    void saveRequirementsChildren(NarRequirements *narReq, zodiac::NodeHandle &sceneNode);
    void saveStoryTags(NarNode *narNode, zodiac::NodeHandle &sceneNode);

    void lockAllNodes();
    void resetAllNodes();

    void showUnlockableNodes(QList<zodiac::NodeHandle> &nodes);
    bool areAllNodesUnlocked(QList<zodiac::NodeHandle> &nodes);

    void saveNarrativeNode(zodiac::NodeHandle &node);

private: // static members

    ///
    /// \brief Default node name. "Node_" will result in a default name of "Node_12" for example.
    ///
    static QString s_defaultName;
};

#endif // NODEMANAGER_H
