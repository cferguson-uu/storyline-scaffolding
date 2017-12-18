#include "mainctrl.h"

#include <QDebug>
#include <cstdlib>

#include "nodectrl.h"
#include "propertyeditor.h"
#include "zodiacgraph/nodehandle.h"


QString MainCtrl::s_defaultName = "Node ";

MainCtrl::MainCtrl(QObject *parent, zodiac::Scene* scene, PropertyEditor* propertyEditor, AnalyticsHandler* analyticsHandler, QAction* newStoryNodeAction, QUndoStack *undoStack)
    : QObject(parent)
    , m_scene(zodiac::SceneHandle(scene))
    , m_propertyEditor(propertyEditor)
    , m_nodes(QHash<zodiac::NodeHandle, NodeCtrl*>())
    , m_nodeIndex(1)            // name suffixes start at 1
    , m_analytics(analyticsHandler)
    , m_createStoryAction(newStoryNodeAction)
    , m_pUndoStack(undoStack)
{
    m_saveAndLoadManager.LoadNarrativeParamsAndCommands(qobject_cast<QWidget*>(parent));

    m_propertyEditor->setMainCtrl(this);
    m_analytics->setAnalyticsProperties(m_propertyEditor->getAnalyticsProperties());

    m_propertyEditor->setCommandList(m_saveAndLoadManager.GetCommandListPointer());
    m_propertyEditor->setUndoStack(undoStack);

    connect(&m_scene, SIGNAL(selectionChanged(QList<zodiac::NodeHandle>)),
            this, SLOT(selectionChanged(QList<zodiac::NodeHandle>)));

    connect(&m_scene, SIGNAL(updateAnalyticsProperties()),
            this, SLOT(updateAnalyticsProperties()));

    connect(m_analytics, SIGNAL(unlockNode(QString)),
            this, SLOT(unlockNode(QString)));

    connect(m_analytics, SIGNAL(checkForGraphs()),
            this, SLOT(checkNarrativeAndStoryGraphsLoaded()));

    connect(m_analytics, &AnalyticsHandler::closeNodeProperties,
        &m_scene, &zodiac::SceneHandle::deselectAll);

    connect(m_analytics, &AnalyticsHandler::lockAllNodes,
        this, &MainCtrl::lockAllNodes);
}

NodeCtrl* MainCtrl::createNode(zodiac::StoryNodeType storyType, const QString& name, const QString& description, bool load)
{
    // the newly created Node is the only selected one to avoid confusion
    m_scene.deselectAll();

    // use the given name or construct a default one
    QString nodeName = name;
    if(nodeName.isEmpty()){
        nodeName = s_defaultName + QString::number(m_nodeIndex++);
    }

    // create the node
    NodeCtrl* nodeCtrl = new NodeCtrl(this, m_scene.createNode(nodeName, description, storyType, load));
    m_nodes.insert(nodeCtrl->getNodeHandle(), nodeCtrl);

    //set up plugs for links between nodes
    if(nodeCtrl->getType() == zodiac::NODE_NARRATIVE)
    {
        //add incoming plugs
        nodeCtrl->addIncomingPlug("reqIn");

        //add outgoing plugs
        nodeCtrl->addOutgoingPlug("reqOut");
        nodeCtrl->addOutgoingPlug("storyOut");
    }
    else
        if(nodeCtrl->getType() == zodiac::NODE_STORY)
        {
            //add incoming plugs
            nodeCtrl->addIncomingPlug("storyIn");
            nodeCtrl->addIncomingPlug("narrativeIn");

            //add outgoing plugs
            nodeCtrl->addOutgoingPlug("storyOut");
        }

    m_propertyEditor->UpdateLinkerValues(m_scene.getNodes());

    return nodeCtrl;
}

bool MainCtrl::deleteNode(NodeCtrl* node)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_nodes.contains(node->getNodeHandle()));
#else
    if(!m_nodes.contains(node->getNodeHandle())){
        return false;
    }
#endif

    if(!node->isRemovable()){
        // nodes with storyOut or reqIn connections cannot be deleted
        return false;
    }

    // disconnected all plugs, disconnect and delete the node
    zodiac::NodeHandle handle = node->getNodeHandle();

    QList<zodiac::PlugHandle> plugs = handle.getPlugs();
    foreach (zodiac::PlugHandle plug, plugs)
    {
        plug.disconnectAll();
    }

    node->disconnect();
    m_nodes.remove(handle);
    bool result = handle.remove();

    //if no more story nodes, allow a new graph to be created
    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();
    bool noStoryNodes = true;

    foreach (zodiac::NodeHandle node, nodes)
    {
        if(node.getType() == zodiac::NODE_STORY)
        {
            noStoryNodes = false;
            break;
        }
    }

    if(noStoryNodes)
         m_createStoryAction->setEnabled(true);

    Q_ASSERT(result);
    return result;
}

bool MainCtrl::shutdown()
{
    // do not receive any more signals from the scene handle
    m_scene.disconnect();

    return true;
}

void MainCtrl::createDefaultNode()
{
    NodeCtrl* newNode = createNode(zodiac::STORY_NONE);

//    int plugCount = (qreal(qrand())/qreal(RAND_MAX))*12;
//    for(int i = 0; i < plugCount + 4; ++i){
//        if((qreal(qrand())/qreal(RAND_MAX))<0.5){
//            newNode->addIncomingPlug("plug");
//        } else {
//            newNode->addOutgoingPlug("plug");
//        }
//    }

    newNode->setSelected(true);
}

void MainCtrl::selectionChanged(QList<zodiac::NodeHandle> selection)
{
    m_propertyEditor->showNodes(selection);
    m_propertyEditor->UpdateLinkerValues(m_scene.getNodes());
}

NodeCtrl* MainCtrl::createStoryNode(NodeCtrl *parent, zodiac::StoryNodeType type, QString name, QString description, QPoint &pos, bool relative, bool load)
{
    NodeCtrl* child = createNode(type, name, description, load);

    //set the position
    if(relative)
        child->setPos(parent->getPos().x() + pos.x(), parent->getPos().y() + pos.y());
    else
        child->setPos(pos.x(), pos.y());

    //if the out plug doesn't exist, make one
    zodiac::PlugHandle ParentNodeOutPlug = parent->getNodeHandle().getPlug("storyOut");
    if(!ParentNodeOutPlug.isValid())
        ParentNodeOutPlug = parent->getNodeHandle().createOutgoingPlug("storyOut");

    zodiac::PlugHandle childNodeInPlug = child->getNodeHandle().getPlug("storyIn");

    ParentNodeOutPlug.connectPlug(childNodeInPlug);

    if(!load)
        spaceOutStory();

    return child;
}

void MainCtrl::createStoryGraph(QString storyName)
{
    NodeCtrl* nameNode = createNode(zodiac::STORY_NAME, storyName); //create story name

    NodeCtrl* settingNode = createNode(zodiac::STORY_SETTING, "Setting", "Setting Group"); //create setting
    NodeCtrl* themeNode = createNode(zodiac::STORY_THEME, "Theme", "Theme Group"); //create theme
    NodeCtrl* plotNode = createNode(zodiac::STORY_PLOT, "Plot", "Plot Group"); //create plot
    NodeCtrl* resolutionNode = createNode(zodiac::STORY_RESOLUTION, "Resolution", "Resolution Group"); //create resolution

    //distance between each object should be 100, looks clean

    nameNode->getNodeHandle().setPos(0, -100);
    settingNode->getNodeHandle().setPos(-150, 100);
    themeNode->getNodeHandle().setPos(-50, 100);
    plotNode->getNodeHandle().setPos(50, 100);
    resolutionNode->getNodeHandle().setPos(150, 100);

    zodiac::PlugHandle nameNodeOutPlug = nameNode->getNodeHandle().getPlug("storyOut");

    zodiac::PlugHandle settingNodeInPlug = settingNode->getNodeHandle().getPlug("storyIn");
    nameNodeOutPlug.connectPlug(settingNodeInPlug);
    zodiac::PlugHandle themeNodeInPlug = themeNode->getNodeHandle().getPlug("storyIn");
    nameNodeOutPlug.connectPlug(themeNodeInPlug);
    zodiac::PlugHandle plotNodeInPlug = plotNode->getNodeHandle().getPlug("storyIn");
    nameNodeOutPlug.connectPlug(plotNodeInPlug);
    zodiac::PlugHandle resolutionNodeInPlug = resolutionNode->getNodeHandle().getPlug("storyIn");
    nameNodeOutPlug.connectPlug(resolutionNodeInPlug);

    m_createStoryAction->setEnabled(false);
}

void MainCtrl::saveStoryGraph()
{
    //get list of all story nodes
    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();
    zodiac::NodeHandle *mainStoryNode = nullptr;
    zodiac::NodeHandle *settingNode = nullptr;
    zodiac::NodeHandle *themeNode = nullptr;
    zodiac::NodeHandle *plotNode = nullptr;
    zodiac::NodeHandle *resolutionNode = nullptr;

    //iterate through the list to find the nodes
    for(QList<zodiac::NodeHandle>::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        if((*it).getType() == zodiac::NODE_STORY)
        {
            if((*it).getStoryNodeType() == zodiac::STORY_NAME)
                mainStoryNode = &(*it); //get a pointer to the handle of the name node
            else
                if((*it).getStoryNodeType() == zodiac::STORY_SETTING)
                    settingNode = &(*it); //get a pointer to the handle of the settings node
                else
                    if((*it).getStoryNodeType() == zodiac::STORY_THEME)
                        themeNode = &(*it); //get a pointer to the handle of the theme node
                    else
                        if((*it).getStoryNodeType() == zodiac::STORY_PLOT)
                            plotNode = &(*it); //get a pointer to the handle of the plot node
                        else
                            if((*it).getStoryNodeType() == zodiac::STORY_RESOLUTION)
                                resolutionNode = &(*it); //get a pointer to the handle of the resolution node
        }
    }

    m_saveAndLoadManager.DeleteAllStoryItems(); //clear the manager just in case

    //store story name
    m_saveAndLoadManager.setStoryName(mainStoryNode->getName());

    //save settings nodes
    QList<zodiac::PlugHandle> connectedPlugs = settingNode->getPlug("storyOut").getConnectedPlugs();
    for(QList<zodiac::PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
    {
        saveSettingItem((*connectedPlugIt).getNode());
    }

    //save theme nodes
    connectedPlugs = themeNode->getPlug("storyOut").getConnectedPlugs();
    for(QList<zodiac::PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
    {
        saveThemeItem((*connectedPlugIt).getNode());
    }

    //get plot node
    //get each episode node - store, store subgoal, iterate through attempts, store these and sub episodes, same with outcomes
    connectedPlugs = plotNode->getPlug("storyOut").getConnectedPlugs();
    for(QList<zodiac::PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
    {
        savePlotItem((*connectedPlugIt).getNode());
    }

    //get resolution node
    saveResolution(*resolutionNode);

    m_saveAndLoadManager.SaveStoryToFile(qobject_cast<QWidget*>(parent()));
}

void MainCtrl::saveSettingItem(zodiac::NodeHandle &settingGroup)
{
    QList<zodiac::PlugHandle> connectedPlugs = settingGroup.getPlug("storyOut").getConnectedPlugs();
    for(QList<zodiac::PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
    {
        zodiac::NodeHandle settingNode = (*connectedPlugIt).getNode();
        SettingItem *settingItem;

        if(settingNode.getType() == zodiac::STORY_SETTING_CHARACTER)
            settingItem = m_saveAndLoadManager.addCharacter(settingNode.getName(), settingNode.getDescription());
        else
            if(settingNode.getType() == zodiac::STORY_SETTING_LOCATION)
                settingItem = m_saveAndLoadManager.addLocation(settingNode.getName(), settingNode.getDescription());
            else
                if(settingNode.getType() == zodiac::STORY_SETTING_TIME)
                    settingItem = m_saveAndLoadManager.addTime(settingNode.getName(), settingNode.getDescription());
                else
                    return; //error

            QList<zodiac::PlugHandle> detailPlugs = settingNode.getPlug("storyOut").getConnectedPlugs();
            for(QList<zodiac::PlugHandle>::iterator detailPlugIt = detailPlugs.begin(); detailPlugIt != detailPlugs.end(); ++detailPlugIt)
            {
                zodiac::NodeHandle detailNode = (*detailPlugIt).getNode();
                m_saveAndLoadManager.addDetail(settingItem, detailNode.getName(), detailNode.getDescription(), "", "");
            }
    }
}

void MainCtrl::saveThemeItem(zodiac::NodeHandle &parent, EventGoal *parentItem)
{
    QList<zodiac::PlugHandle> connectedPlugs = parent.getPlug("storyOut").getConnectedPlugs();
    for(QList<zodiac::PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
    {
        zodiac::NodeHandle eventGoalNode = (*connectedPlugIt).getNode();
        EventGoal *eventGoalItem;

        if(eventGoalNode.getType() == zodiac::STORY_THEME_EVENT)
            eventGoalItem = m_saveAndLoadManager.addEvent(eventGoalNode.getName(), eventGoalNode.getDescription(), parentItem);
        else
            if(eventGoalNode.getType() == zodiac::STORY_THEME_GOAL)
                eventGoalItem = m_saveAndLoadManager.addGoal(eventGoalNode.getName(), eventGoalNode.getDescription(), parentItem);
            else
                return; //error

            if(parent.getPlug("storyOut").connectionCount() > 0)
                saveThemeItem(eventGoalNode, eventGoalItem);
    }
}

void MainCtrl::savePlotItem(zodiac::NodeHandle &parent, Episode *parentItem)
{
    QList<zodiac::PlugHandle> connectedPlugs = parent.getPlug("storyOut").getConnectedPlugs();
    for(QList<zodiac::PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
    {
        zodiac::NodeHandle episodeNode = (*connectedPlugIt).getNode();
        Episode *episodeItem;

        if(episodeNode.getType() == zodiac::STORY_PLOT_EPISODE)
            episodeItem = m_saveAndLoadManager.addEpisode(episodeNode.getName(), episodeNode.getDescription(), parentItem);
        else
            return; //error

        QList<zodiac::PlugHandle> childPlugs = parent.getPlug("storyOut").getConnectedPlugs();
        for(QList<zodiac::PlugHandle>::iterator childPlugIt = childPlugs.begin(); childPlugIt != childPlugs.end(); ++childPlugIt)
        {
            zodiac::NodeHandle childNode = (*connectedPlugIt).getNode();

            if(childNode.getStoryNodeType() == zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP)
            {
                QList<zodiac::PlugHandle> attemptPlugs = childNode.getPlug("storyOut").getConnectedPlugs();
                for(QList<zodiac::PlugHandle>::iterator attemptPlugIt = attemptPlugs.begin(); attemptPlugIt != attemptPlugs.end(); ++attemptPlugIt)
                {
                    zodiac::NodeHandle attemptNode = (*attemptPlugIt).getNode();

                    if(attemptNode.getStoryNodeType() == zodiac::STORY_PLOT_EPISODE_ATTEMPT)
                        m_saveAndLoadManager.addAttempt(attemptNode.getName(), attemptNode.getDescription(), episodeItem);
                    else //sup-episode
                        savePlotItem(attemptNode, episodeItem);
                }
            }
            else
                if(childNode.getStoryNodeType() == zodiac::STORY_PLOT_EPISODE_OUTCOME_GROUP)
                {
                    QList<zodiac::PlugHandle> outcomePlugs = childNode.getPlug("storyOut").getConnectedPlugs();
                    for(QList<zodiac::PlugHandle>::iterator outcomePlugIt = outcomePlugs.begin(); outcomePlugIt != outcomePlugs.end(); ++outcomePlugIt)
                    {
                        zodiac::NodeHandle outcomeNode = (*outcomePlugIt).getNode();

                       if(outcomeNode.getStoryNodeType() == zodiac::STORY_PLOT_EPISODE_OUTCOME)
                            m_saveAndLoadManager.addOutcome(outcomeNode.getName(), outcomeNode.getDescription(), episodeItem);
                        else //sup-episode
                            savePlotItem(outcomeNode, episodeItem);
                    }
                }
                else
                    if((childNode).getStoryNodeType() == zodiac::STORY_PLOT_EPISODE_SUBGOAL)
                    {
                        m_saveAndLoadManager.addSubGoal(childNode.getName(), childNode.getDescription(), episodeItem);
                    }
        }
    }
}

void MainCtrl::saveResolution(zodiac::NodeHandle &parent)
{
    zodiac::NodeHandle eventGroupNode;
    zodiac::NodeHandle stateGroupNode;

    QList<zodiac::PlugHandle> connectedPlugs = parent.getPlug("storyOut").getConnectedPlugs();
    for(QList<zodiac::PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
    {
        if((*connectedPlugIt).getNode().getType() == zodiac::STORY_RESOLUTION_EVENT_GROUP)
            eventGroupNode = (*connectedPlugIt).getNode();
        else
            if((*connectedPlugIt).getNode().getType() == zodiac::STORY_RESOLUTION_STATE_GROUP)
                stateGroupNode = (*connectedPlugIt).getNode();
            else
                return; //error
    }

    QList<zodiac::PlugHandle> eventPlugs = eventGroupNode.getPlug("storyOut").getConnectedPlugs();
    for(QList<zodiac::PlugHandle>::iterator eventPlugIt = eventPlugs.begin(); eventPlugIt != eventPlugs.end(); ++eventPlugIt)
    {
        zodiac::NodeHandle eventNode = (*eventPlugIt).getNode();

        m_saveAndLoadManager.addResolutionEvent(eventNode.getName(), eventNode.getDescription());
    }

    QList<zodiac::PlugHandle> statePlugs = eventGroupNode.getPlug("storyOut").getConnectedPlugs();
    for(QList<zodiac::PlugHandle>::iterator statePlugIt = statePlugs.begin(); statePlugIt != statePlugs.end(); ++statePlugIt)
    {
        zodiac::NodeHandle stateNode = (*statePlugIt).getNode();

        m_saveAndLoadManager.addResolutionState(stateNode.getName(), stateNode.getDescription());
    }

}

void MainCtrl::loadStoryGraph()
{
    if(m_saveAndLoadManager.LoadStoryFromFile(qobject_cast<QWidget*>(parent())))
    {
        //create the story graph and grab the nodes
        createStoryGraph(m_saveAndLoadManager.GetStoryName());
        QList<zodiac::NodeHandle> nodes = m_scene.getNodes();

        //find the main nodes
        zodiac::NodeHandle *startingNode;
        zodiac::NodeHandle *settingNode;
        zodiac::NodeHandle *themeNode;
        zodiac::NodeHandle *plotNode;
        zodiac::NodeHandle *resolutionNode;
        //iterate through the list to find the nodes
        for(QList<zodiac::NodeHandle>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            if((*it).getType() == zodiac::NODE_STORY)
            {
                if((*it).getStoryNodeType() == zodiac::STORY_NAME)
                    startingNode = &(*it); //get a pointer to the handle of the name node
                else
                    if((*it).getStoryNodeType() == zodiac::STORY_SETTING)
                        settingNode = &(*it); //get a pointer to the handle of the settings node
                    else
                        if((*it).getStoryNodeType() == zodiac::STORY_THEME)
                            themeNode = &(*it); //get a pointer to the handle of the theme node
                        else
                            if((*it).getStoryNodeType() == zodiac::STORY_PLOT)
                                plotNode = &(*it); //get a pointer to the handle of the plot node
                            else
                                if((*it).getStoryNodeType() == zodiac::STORY_RESOLUTION)
                                    resolutionNode = &(*it); //get a pointer to the handle of the resolution node
            }
        }

        zodiac::PlugHandle SettingNodeOutPlug = settingNode->getPlug("storyOut");   //get the outgoing plugs
        zodiac::PlugHandle ThemeNodeOutPlug = themeNode->getPlug("storyOut");
        zodiac::PlugHandle PlotNodeOutPlug = plotNode->getPlug("storyOut");
        zodiac::PlugHandle ResolutionNodeOutPlug = resolutionNode->getPlug("storyOut");

        //load the setting items
        QList<SettingItem> chars = m_saveAndLoadManager.GetCharacters();
        NodeCtrl* characterNode = nullptr;
        if(chars.size() > 0)
        {
            //create parent group node, move it, link to main settings node and set up plug for linking children
            characterNode = createNode(zodiac::STORY_SETTING_CHARACTER_GROUP, "Characters", "Characters Group");

            zodiac::PlugHandle characterNodeInPlug = characterNode->getNodeHandle().getPlug("storyIn");
            SettingNodeOutPlug.connectPlug(characterNodeInPlug);

            loadSettingItem(characterNode, chars, zodiac::STORY_SETTING_CHARACTER);
        }

        QList<SettingItem> locs = m_saveAndLoadManager.GetLocations();
        NodeCtrl* locationNode = nullptr;
        if(locs.size() > 0)
        {
            //create parent group node, move it, link to main settings node and set up plug for linking children
            locationNode = createNode(zodiac::STORY_SETTING_LOCATION_GROUP, "Locations", "Locations Group");

            zodiac::PlugHandle locationNodeInPlug = locationNode->getNodeHandle().getPlug("storyIn");
            SettingNodeOutPlug.connectPlug(locationNodeInPlug);

            loadSettingItem(locationNode, locs, zodiac::STORY_SETTING_LOCATION);
        }

        QList<SettingItem> times = m_saveAndLoadManager.GetTimes();
        NodeCtrl* timeNode = nullptr;
        if(times.size() > 0)
        {
            //create parent group node, move it, link to main settings node and set up plug for linking children
            timeNode = createNode(zodiac::STORY_SETTING_TIME_GROUP, "Times", "Times Group");

            zodiac::PlugHandle timeNodeInPlug = timeNode->getNodeHandle().getPlug("storyIn");
            SettingNodeOutPlug.connectPlug(timeNodeInPlug);

            loadSettingItem(timeNode, times, zodiac::STORY_SETTING_TIME);
        }

        //load the theme items
        QList<EventGoal> events = m_saveAndLoadManager.GetEvents();
        NodeCtrl* eventNode = nullptr;
        if(events.size() > 0)
        {
            //create group node then load the events and all sub-events
            eventNode = createNode(zodiac::STORY_THEME_EVENT_GROUP, "Events", "Events Group");
            zodiac::PlugHandle parentNodeInPlug = eventNode->getNodeHandle().getPlug("storyIn");
            ThemeNodeOutPlug.connectPlug(parentNodeInPlug);

            loadThemeItem(eventNode, events, zodiac::STORY_THEME_EVENT);
        }

        QList<EventGoal> goals = m_saveAndLoadManager.GetGoals();
        NodeCtrl* goalNode = nullptr;
        if(goals.size() > 0)
        {
            //same as events but goals
            goalNode = createNode(zodiac::STORY_THEME_GOAL_GROUP, "Goals", "Goals Group");
            zodiac::PlugHandle parentNodeInPlug = goalNode->getNodeHandle().getPlug("storyIn");
            ThemeNodeOutPlug.connectPlug(parentNodeInPlug);

            loadThemeItem(goalNode, goals, zodiac::STORY_THEME_GOAL);
        }

        //load the plot items (episodes)
        QList<Episode> episodes = m_saveAndLoadManager.GetEpisodes();
        loadEpisodes(plotNode, episodes);

        //load the resolution
        QList<EventGoal> resEvents = m_saveAndLoadManager.GetResolution().events;
        QList<SimpleNode> resStates = m_saveAndLoadManager.GetResolution().states;
        loadResolution(resolutionNode, resEvents, resStates);

        //space out the graph properly
        spaceOutStory();

        m_propertyEditor->UpdateLinkerValues(m_scene.getNodes());
    }
}

void MainCtrl::loadSettingItem(NodeCtrl *parentNode, QList<SettingItem> items, zodiac::StoryNodeType childType)
{
    //add each item to the tree
    for(QList<SettingItem>::iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt)
    {
        NodeCtrl *childNode = createStoryNode(parentNode, childType, (*itemIt).id, (*itemIt).description, QPoint(parentNode->getPos().x(), 150), true, true);

        if((*itemIt).details.size() > 0)
        {
            //add all the details for the items
            for(QList<SimpleNode>::iterator detailIt = (*itemIt).details.begin(); detailIt != (*itemIt).details.end(); ++detailIt)
            {
                createStoryNode(childNode, zodiac::STORY_ITEM_DETAILS, (*detailIt).id, (*detailIt).description, QPoint(childNode->getPos().x(), 150), true, true);
            }
        }
    }
}

void MainCtrl::loadThemeItem(NodeCtrl* parentNode, QList<EventGoal> items, zodiac::StoryNodeType childType)
{
    //add each item to the tree
    for(QList<EventGoal>::iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt)
    {
        NodeCtrl *itemNode = createStoryNode(parentNode, childType, (*itemIt).id, (*itemIt).description, QPoint(parentNode->getPos().x(), 150), true, true);

        //if sub-item then load those too
        if((*itemIt).subItems.size() > 0)
        {
            for(QList<EventGoal>::iterator subItemIt = (*itemIt).subItems.begin(); subItemIt != (*itemIt).subItems.end(); ++subItemIt)
            {
                loadThemeItem(itemNode, (*itemIt).subItems, childType);
            }
        }
    }
}

void MainCtrl::loadEpisodes(zodiac::NodeHandle *parentNode, QList<Episode> episodes)
{
    NodeCtrl *parentNodeCtrl = new NodeCtrl(this, *parentNode);

    //add each item to the tree
    for(QList<Episode>::iterator epIt = episodes.begin(); epIt != episodes.end(); ++epIt)
    {
        NodeCtrl *episodeNode = createStoryNode(parentNodeCtrl, zodiac::STORY_PLOT_EPISODE, (*epIt).id, (*epIt).description, QPoint(parentNodeCtrl->getPos().x(), 150), true, true);

        NodeCtrl *attemptGroupNode = createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP, "Attempt", "Attempt Group", QPoint(0, 150), true, true);
        //handle attempts
        if((*epIt).attempts.size() > 0 || (*epIt).attemptSubEpisodes.size() > 0)
        {
            for(QList<SimpleNode>::iterator attIt = (*epIt).attempts.begin(); attIt != (*epIt).attempts.end(); ++attIt)
            {
                createStoryNode(attemptGroupNode, zodiac::STORY_PLOT_EPISODE_ATTEMPT, (*attIt).id, (*attIt).description, QPoint(attemptGroupNode->getPos().x(), 150), true, true);
            }

            if((*epIt).attemptSubEpisodes.size() > 0)
            {
                loadEpisodes(&attemptGroupNode->getNodeHandle(), (*epIt).attemptSubEpisodes);
            }

            //get all connected plugs
            QList<zodiac::PlugHandle> connectedPlugs = attemptGroupNode->getNodeHandle().getPlug("storyOut").getConnectedPlugs();
            if(connectedPlugs.size() > 1)
            {
                for(QList<zodiac::PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
                {
                   zodiac::NodeHandle connectedNode = (*connectedPlugIt).getNode();
                }
            }
        }

        NodeCtrl *outcomeGroupNode = createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_OUTCOME_GROUP, "Outcome", "Outcome Group", QPoint(episodeNode->getPos().x(), 150), true, true);
        if((*epIt).outcomes.size() > 0 || (*epIt).outcomeSubEpisodes.size() > 0)
        {
            //handle outcomes
            for(QList<SimpleNode>::iterator outIt = (*epIt).outcomes.begin(); outIt != (*epIt).outcomes.end(); ++outIt)
            {
                createStoryNode(outcomeGroupNode, zodiac::STORY_PLOT_EPISODE_OUTCOME, (*outIt).id, (*outIt).description, QPoint(outcomeGroupNode->getPos().x(), 150), true, true);
            }

            if((*epIt).outcomeSubEpisodes.size() > 0)
            {
                //handle outcome sub-episodes
                loadEpisodes(&outcomeGroupNode->getNodeHandle(), (*epIt).outcomeSubEpisodes);
            }

            //get all connected plugs
            QList<zodiac::PlugHandle> connectedPlugs = outcomeGroupNode->getNodeHandle().getPlug("storyOut").getConnectedPlugs();
            if(connectedPlugs.size() > 1)
            {
                for(QList<zodiac::PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
                {
                   zodiac::NodeHandle connectedNode = (*connectedPlugIt).getNode();
                }
            }
        }

        //handle sub-goal
        createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_SUBGOAL, (*epIt).subGoal.id, (*epIt).subGoal.description, QPoint(episodeNode->getPos().x(), 150), true, true);
    }
}

void MainCtrl::loadResolution(zodiac::NodeHandle *resolutionNode, QList<EventGoal> events, QList<SimpleNode> states)
{
    if(events.size() > 0)
    {
        NodeCtrl *eventNode = createNode(zodiac::STORY_RESOLUTION_EVENT_GROUP, "Event", "Group of Events");
        zodiac::PlugHandle eventNodeInPlug = eventNode->getNodeHandle().getPlug("storyIn");
        resolutionNode->getPlug("storyOut").connectPlug(eventNodeInPlug);

        for(QList<EventGoal>::iterator evIt = events.begin(); evIt != events.end(); ++evIt)
        {
            createStoryNode(eventNode, zodiac::STORY_RESOLUTION_EVENT, (*evIt).id, (*evIt).description, QPoint(eventNode->getPos().x(), 150), true, true);
        }
    }

   if(states.size() > 0)
    {
        NodeCtrl *stateNode = createNode(zodiac::STORY_RESOLUTION_STATE_GROUP, "State", "Group of States");
        zodiac::PlugHandle stateNodeInPlug = stateNode->getNodeHandle().getPlug("storyIn");
        resolutionNode->getPlug("storyOut").connectPlug(stateNodeInPlug);

        for(QList<SimpleNode>::iterator stIt = states.begin(); stIt != states.end(); ++stIt)
        {
            createStoryNode(stateNode, zodiac::STORY_RESOLUTION_STATE, (*stIt).id, (*stIt).description, QPoint(stateNode->getPos().x(), 150), true, true);
        }

    }
}

void MainCtrl::saveNarrativeGraph()
{
    m_saveAndLoadManager.DeleteAllNarrativeItems(); //clear the narrative data from save and load

    //get list of all nodes
    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();

    for(QList<zodiac::NodeHandle>::iterator nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
    {
        if((*nodeIt).getPlug("reqOut").getConnectedPlugs().empty())
        {
            saveNarrativeNode((*nodeIt));
        }
    }

    m_saveAndLoadManager.SaveNarrativeToFile(qobject_cast<QWidget*>(parent()));

}

void MainCtrl::saveNarrativeNode(zodiac::NodeHandle &node)
{
    NarNode *newNarrativeNode = nullptr;

    if(node.getType() == zodiac::NODE_NARRATIVE && !(node.getName() == "SEQ" || node.getName() == "INV"))
        newNarrativeNode = m_saveAndLoadManager.addNarrativeNode(node.getName(), node.getDescription());

    if(newNarrativeNode)    //will return nullptr if node already exists or is a requirement decorator node
    {
        saveCommands(newNarrativeNode, node);
        saveRequirements(newNarrativeNode, node);
        saveStoryTags(newNarrativeNode, node);
    }

    QList<zodiac::PlugHandle> connectedPlugs = node.getPlug("reqIn").getConnectedPlugs();

    foreach (zodiac::PlugHandle plug, connectedPlugs)
    {
        zodiac::NodeHandle newNode = plug.getNode();
        saveNarrativeNode(newNode);
    }
}

void MainCtrl::saveCommands(NarNode *narNode, zodiac::NodeHandle &sceneNode)
{
    QHash<QUuid, zodiac::NodeCommand> onUnlockList = sceneNode.getOnUnlockList();
    for(QHash<QUuid, zodiac::NodeCommand>::iterator cmdIt = onUnlockList.begin(); cmdIt != onUnlockList.end(); ++cmdIt)
    {
        NarCommand* cmd = m_saveAndLoadManager.addOnUnlock(narNode, (*cmdIt).id, (*cmdIt).description);
        QHash<QString, QString> paramList = (*cmdIt).parameters;

        for(QHash<QString, QString>::iterator paramIt = paramList.begin(); paramIt != paramList.end(); ++paramIt)
        {
            m_saveAndLoadManager.addParameterToCommand(cmd, paramIt.key(), paramIt.value());
        }
    }

    QHash<QUuid, zodiac::NodeCommand> onFailList = sceneNode.getOnFailList();
    for(QHash<QUuid, zodiac::NodeCommand>::iterator cmdIt = onFailList.begin(); cmdIt != onFailList.end(); ++cmdIt)
    {
        NarCommand* cmd = m_saveAndLoadManager.addOnFail(narNode, (*cmdIt).id, (*cmdIt).description);
        QHash<QString, QString> paramList = (*cmdIt).parameters;

        for(QHash<QString, QString>::iterator paramIt = paramList.begin(); paramIt != paramList.end(); ++paramIt)
        {
            m_saveAndLoadManager.addParameterToCommand(cmd, paramIt.key(), paramIt.value());
        }
    }

    QHash<QUuid, zodiac::NodeCommand> onUnlockedList = sceneNode.getOnUnlockedList();
    for(QHash<QUuid, zodiac::NodeCommand>::iterator cmdIt = onUnlockedList.begin(); cmdIt != onUnlockedList.end(); ++cmdIt)
    {
        NarCommand* cmd = m_saveAndLoadManager.addOnUnlocked(narNode, (*cmdIt).id, (*cmdIt).description);
        QHash<QString, QString> paramList = (*cmdIt).parameters;

        for(QHash<QString, QString>::iterator paramIt = paramList.begin(); paramIt != paramList.end(); ++paramIt)
        {
            m_saveAndLoadManager.addParameterToCommand(cmd, paramIt.key(), paramIt.value());
        }
    }
}

void MainCtrl::saveRequirements(NarNode *narNode, zodiac::NodeHandle &sceneNode)
{
    if(sceneNode.getPlug("reqOut").isValid())
    {
        QList<zodiac::PlugHandle> requirementPlugs = sceneNode.getPlug("reqOut").getConnectedPlugs();

        for(QList<zodiac::PlugHandle>::iterator rPlugIt = requirementPlugs.begin(); rPlugIt != requirementPlugs.end(); ++rPlugIt)
        {
            zodiac::NodeHandle reqNode = (*rPlugIt).getNode();

            if(reqNode.getName() == "SEQ")
            {
                NarRequirements *newReq = m_saveAndLoadManager.addRequirementToNarrativeNode(narNode, "SEQ");
                saveRequirementsChildren(newReq, reqNode);  //call recursive function to handle rest of requirements
            }
            else
                if(reqNode.getName() == "INV")
                {
                    QList<zodiac::PlugHandle> invPlugs = reqNode.getPlug("reqOut").getConnectedPlugs();

                    if(invPlugs.size() < 1 || invPlugs.size() > 1)
                    {
                        qDebug() << "Error: should only be one node attached";
                        continue;
                    }
                    else
                    {
                        zodiac::NodeHandle invReqNode = (*invPlugs.begin()).getNode();

                        if(reqNode.getName() == "SEQ")
                        {
                            NarRequirements *newReq = m_saveAndLoadManager.addRequirementToNarrativeNode(narNode, "INV");
                            saveRequirementsChildren(newReq, reqNode);  //call recursive function to handle rest of requirements
                        }
                        else
                            m_saveAndLoadManager.addRequirementToNarrativeNode(narNode, "INV", invReqNode.getName());
                    }
                }
                else    //is leaf
                {
                    m_saveAndLoadManager.addRequirementToNarrativeNode(narNode, "LEAF", reqNode.getName());
                }
        }
    }
}

void MainCtrl::saveRequirementsChildren(NarRequirements *narReq, zodiac::NodeHandle &sceneNode)
{
    if(sceneNode.getPlug("reqOut").isValid())
    {
        QList<zodiac::PlugHandle> requirementPlugs = sceneNode.getPlug("reqOut").getConnectedPlugs();

        for(QList<zodiac::PlugHandle>::iterator rPlugIt = requirementPlugs.begin(); rPlugIt != requirementPlugs.end(); ++rPlugIt)
        {
            zodiac::NodeHandle reqNode = (*rPlugIt).getNode();

            if(reqNode.getName() == "SEQ")
            {
                NarRequirements *newReq = m_saveAndLoadManager.addChildRequirement(narReq, "SEQ");
                saveRequirementsChildren(newReq, reqNode);  //call recursive function to handle rest of requirements
            }
            else
                if(reqNode.getName() == "INV")
                {
                    QList<zodiac::PlugHandle> invPlugs = reqNode.getPlug("reqOut").getConnectedPlugs();

                    if(invPlugs.size() < 1 || invPlugs.size() > 1)
                    {
                        qDebug() << "Error: should only be one node attached";
                        continue;
                    }
                    else
                    {
                        zodiac::NodeHandle invReqNode = (*invPlugs.begin()).getNode();

                        if(reqNode.getName() == "SEQ")
                        {
                            NarRequirements *newReq = m_saveAndLoadManager.addChildRequirement(narReq, "INV");
                            saveRequirementsChildren(newReq, reqNode);  //call recursive function to handle rest of requirements
                        }
                        else
                            m_saveAndLoadManager.addChildRequirement(narReq, "INV", invReqNode.getName());
                    }
                }
                else    //is leaf
                {
                    m_saveAndLoadManager.addChildRequirement(narReq, "LEAF", reqNode.getName());
                }
        }
    }
}

void MainCtrl::saveStoryTags(NarNode *narNode, zodiac::NodeHandle &sceneNode)
{
    if(sceneNode.getPlug("storyOut").isValid())
    {
        QList<zodiac::PlugHandle> storyPlugs = sceneNode.getPlug("storyOut").getConnectedPlugs();

        for(QList<zodiac::PlugHandle>::iterator sPlugIt = storyPlugs.begin(); sPlugIt != storyPlugs.end(); ++sPlugIt)
        {
            zodiac::NodeHandle storyNode = (*sPlugIt).getNode();

            m_saveAndLoadManager.addStoryTagToNarrativeNode(narNode, storyNode.getName());
        }
    }
}

void MainCtrl::loadNarrativeGraph()
{
    m_saveAndLoadManager.DeleteAllNarrativeItems(); //reset the holder

    if(m_saveAndLoadManager.LoadNarrativeFromFile(qobject_cast<QWidget*>(parent())))
    {
        QList<zodiac::NodeHandle> currentNodes =  m_scene.getNodes();
        QList<zodiac::NodeHandle> currentNarSceneNodes;

        for(QList<zodiac::NodeHandle>::iterator cNIt = currentNodes.begin(); cNIt != currentNodes.end(); ++cNIt)
        {
            //also save all current narrative nodes to a separate list in case two parts of the same narrative are loaded separately, for requirements
            if((*cNIt).getType() == zodiac::NODE_NARRATIVE)
            currentNarSceneNodes.push_back((*cNIt));

        }

        QList<NarNode> narrativeNodes = m_saveAndLoadManager.GetNarrativeNodes();
        QList<NodeCtrl*> newNarSceneNodes;

        for(QList<NarNode>::iterator narIt = narrativeNodes.begin(); narIt != narrativeNodes.end(); ++narIt)
        {
            NodeCtrl* newNarNode = createNode(zodiac::STORY_NONE, (*narIt).id, (*narIt).comments);

            loadNarrativeCommands((*narIt), newNarNode);

            newNarSceneNodes.push_back(newNarNode);
        }

        //loop again for the requirements (necessary in case nodes aren't loaded in chronological order)
        for(QList<NarNode>::iterator narIt = narrativeNodes.begin(); narIt != narrativeNodes.end(); ++narIt)
        {
            if((*narIt).requirements.type != REQ_NONE)
            {
                //qDebug() << (*narIt).id << " requirements";

                NodeCtrl* newNarNode = nullptr;

                for(QList<NodeCtrl*>::iterator nodeIt = newNarSceneNodes.begin(); nodeIt != newNarSceneNodes.end(); ++nodeIt)
                {
                    if((*narIt).id == (*nodeIt)->getName())
                    {
                        newNarNode = (*nodeIt);
                    }
                }

                if(!newNarNode)
                {
                    qDebug() << "Warning: node not found in list";
                    continue;
                }

                zodiac::PlugHandle reqOutPlug;

                if(newNarNode->getNodeHandle().getPlug("reqOut").isValid())
                    reqOutPlug = newNarNode->getNodeHandle().getPlug("reqOut");
                else
                    reqOutPlug = newNarNode->addOutgoingPlug("reqOut");

                loadRequirements((*narIt).requirements, reqOutPlug, newNarSceneNodes, currentNarSceneNodes);

                QSet<zodiac::PlugEdge*> edgeList = newNarNode->getNodeHandle().getPlug("reqOut").getEdges();
                for(QSet<zodiac::PlugEdge*>::iterator plugIt = edgeList.begin(); plugIt != edgeList.end(); ++plugIt)
                {
                    (*plugIt)->setBaseColor(QColor(66, 134, 244));
                }
            }
        }

        spaceOutFullNarrative();

        m_propertyEditor->UpdateLinkerValues(m_scene.getNodes());
    }
}

void MainCtrl::loadNarrativeCommands(NarNode &loadedNode, NodeCtrl* sceneNode)
{
    for(QList<NarCommand>::iterator cmdIt = loadedNode.onUnlockCommands.begin(); cmdIt != loadedNode.onUnlockCommands.end(); ++cmdIt)
    {
        QUuid cmdKey = QUuid::createUuid();
        sceneNode->addOnUnlockCommand(cmdKey, (*cmdIt).command, (*cmdIt).description);

        for(QList<SimpleNode>::iterator paramIt = (*cmdIt).params.begin(); paramIt != (*cmdIt).params.end(); ++paramIt)
        {
            sceneNode->addParameterToOnUnlockCommand(cmdKey, (*paramIt).id, (*paramIt).description);
        }
    }

    for(QList<NarCommand>::iterator cmdIt = loadedNode.onFailCommands.begin(); cmdIt != loadedNode.onFailCommands.end(); ++cmdIt)
    {
        QUuid cmdKey = QUuid::createUuid();
        sceneNode->addOnFailCommand(cmdKey, (*cmdIt).command, (*cmdIt).description);

        for(QList<SimpleNode>::iterator paramIt = (*cmdIt).params.begin(); paramIt != (*cmdIt).params.end(); ++paramIt)
        {
            sceneNode->addParameterToOnFailCommand(cmdKey, (*paramIt).id, (*paramIt).description);
        }
    }

    for(QList<NarCommand>::iterator cmdIt = loadedNode.onUnlockedCommands.begin(); cmdIt != loadedNode.onUnlockedCommands.end(); ++cmdIt)
    {
        QUuid cmdKey = QUuid::createUuid();
        sceneNode->addOnUnlockedCommand(cmdKey, (*cmdIt).command, (*cmdIt).description);

        for(QList<SimpleNode>::iterator paramIt = (*cmdIt).params.begin(); paramIt != (*cmdIt).params.end(); ++paramIt)
        {
            sceneNode->addParameterToOnUnlockedCommand(cmdKey, (*paramIt).id, (*paramIt).description);
        }
    }

}

void MainCtrl::loadRequirements(NarRequirements &requirements, zodiac::PlugHandle &parentReqOutPlug, QList<NodeCtrl*> &sceneNodes, QList<zodiac::NodeHandle> &currentNarSceneNodes)
{
    //parentReqOutPlug.getNode().setPos(parentReqOutPlug.getNode().getPos().x(), parentReqOutPlug.getNode().getPos().y() + relativeY);

    NodeCtrl* newRequirementNode;

    if(requirements.type == REQ_LEAF)
    {
        //qDebug() << "Type: LEAF";
        //newRequirementNode = createNode(zodiac::STORY_NONE, "LEAF", "LEAF");

        if(requirements.id != "")
        {
            //qDebug() << "Id: " << requirements.id;

            bool found = false;
            //link it to the node mentioned in the id
            for(QList<NodeCtrl*>::iterator nodeIt = sceneNodes.begin(); nodeIt != sceneNodes.end(); ++nodeIt)
                if((*nodeIt)->getName() == requirements.id)
                {
                    newRequirementNode = (*nodeIt);

                    zodiac::PlugHandle nodeReqInPlug;

                    if((*nodeIt)->getNodeHandle().getPlug("reqIn").isValid())
                        nodeReqInPlug = (*nodeIt)->getNodeHandle().getPlug("reqIn");
                    else
                        nodeReqInPlug = (*nodeIt)->addIncomingPlug("reqIn");
                    parentReqOutPlug.connectPlug(nodeReqInPlug);  //link plugs
                    found  = true;
                    break;
                }

            if(!found)
            {
                qDebug() << "Warning. Node:" << requirements.id << "not found in loaded list, checking scene nodes.";

                for(QList<zodiac::NodeHandle>::iterator nodeIt = currentNarSceneNodes.begin(); nodeIt != currentNarSceneNodes.end(); ++nodeIt)
                    if((*nodeIt).getName() == requirements.id)
                    {
                        newRequirementNode = new NodeCtrl(this, (*nodeIt));;

                        zodiac::PlugHandle nodeReqInPlug;

                        if((*nodeIt).getPlug("reqIn").isValid())
                            nodeReqInPlug = (*nodeIt).getPlug("reqIn");
                        else
                            nodeReqInPlug = (*nodeIt).createIncomingPlug("reqIn");
                        parentReqOutPlug.connectPlug(nodeReqInPlug);  //link plugs
                        found  = true;
                        break;
                    }

                if(!found)
                    qDebug() << "Warning. Node:" << requirements.id << "not found!";
            }
        }
    }
    else
    {
        if(requirements.type == REQ_SEQ)
        {
           //qDebug() << "Type: SEQ";
           newRequirementNode = createNode(zodiac::STORY_NONE, "SEQ", "Requirements Sequence");
           newRequirementNode->setIdleColor(QColor(255, 204, 0));
           newRequirementNode->setSelectedColor(QColor(255, 153, 0));
        }
        else
            if(requirements.type == REQ_INV)
            {
                //qDebug() << "Type: INV";
                newRequirementNode = createNode(zodiac::STORY_NONE, "INV", "Inverse Requirements");
                newRequirementNode->setIdleColor(QColor(255, 204, 0));
                newRequirementNode->setSelectedColor(QColor(255, 153, 0));
            }

        //connect narrative node to new requirements node
        zodiac::PlugHandle reqInPlug;
        if(newRequirementNode->getNodeHandle().getPlug("reqIn").isValid())
            reqInPlug = newRequirementNode->getNodeHandle().getPlug("reqIn");
        else
            reqInPlug = newRequirementNode->addIncomingPlug("reqIn");

        parentReqOutPlug.connectPlug(reqInPlug);

        zodiac::PlugHandle reqOutPlug;

        if(requirements.id != "")
        {
            //qDebug() << "Id: " << requirements.id;

            if(newRequirementNode->getNodeHandle().getPlug("reqOut").isValid())
                reqOutPlug = newRequirementNode->getNodeHandle().getPlug("reqOut");
            else
                reqOutPlug = newRequirementNode->addOutgoingPlug("reqOut");  //create the out plug

            bool found = false;
            //link it to the node mentioned in the id
            for(QList<NodeCtrl*>::iterator nodeIt = sceneNodes.begin(); nodeIt != sceneNodes.end(); ++nodeIt)
                if((*nodeIt)->getName() == requirements.id)
                {
                    zodiac::PlugHandle nodeReqInPlug;

                    if((*nodeIt)->getNodeHandle().getPlug("reqIn").isValid())
                        nodeReqInPlug = (*nodeIt)->getNodeHandle().getPlug("reqIn");
                    else
                        nodeReqInPlug = (*nodeIt)->addIncomingPlug("reqIn");
                    reqOutPlug.connectPlug(nodeReqInPlug);  //link plugs
                    found  = true;
                    break;
                }

            if(!found)
            {
                qDebug() << "Warning. Node:" << requirements.id << "not found in loaded list, checking scene nodes.";

                for(QList<zodiac::NodeHandle>::iterator nodeIt = currentNarSceneNodes.begin(); nodeIt != currentNarSceneNodes.end(); ++nodeIt)
                    if((*nodeIt).getName() == requirements.id)
                    {
                        zodiac::PlugHandle nodeReqInPlug;

                        if((*nodeIt).getPlug("reqIn").isValid())
                            nodeReqInPlug = (*nodeIt).getPlug("reqIn");
                        else
                            nodeReqInPlug = (*nodeIt).createIncomingPlug("reqIn");
                        parentReqOutPlug.connectPlug(nodeReqInPlug);  //link plugs
                        found  = true;
                        break;
                    }

                if(!found)
                    qDebug() << "Warning. Node:" << requirements.id << "not found!";
            }
        }

        float childrenSize = requirements.children.size();
        if(childrenSize > 0)
        {
            for(QList<NarRequirements>::iterator reqIt = requirements.children.begin(); reqIt != requirements.children.end(); ++reqIt)
            {
                if(!reqOutPlug.isValid())
                {
                    if(newRequirementNode->getNodeHandle().getPlug("reqOut").isValid())
                        reqOutPlug = newRequirementNode->getNodeHandle().getPlug("reqOut");
                    else
                        reqOutPlug = newRequirementNode->addOutgoingPlug("reqOut"); //make the out plug if it doesn't exist
                }

                loadRequirements((*reqIt), reqOutPlug, sceneNodes, currentNarSceneNodes);
            }
        }

        QSet<zodiac::PlugEdge*> edgeList = newRequirementNode->getNodeHandle().getPlug("reqOut").getEdges();
        for(QSet<zodiac::PlugEdge*>::iterator plugIt = edgeList.begin(); plugIt != edgeList.end(); ++plugIt)
        {
            (*plugIt)->setBaseColor(QColor(66, 134, 244));
        }
    }
}

void MainCtrl::spaceOutFullNarrative()
{
    float xPos = INFINITY;
    float yPos = -INFINITY;

    QList<zodiac::NodeHandle> nodeList = m_scene.getNodes();

    //loop through story nodes to get lowest, leftist point or use 0,0 if none exist
    if(m_createStoryAction->isEnabled())  //enabled if no story
    {
        xPos = 0.0f;
        yPos = 0.0f;
    }
    else
        for(QList<zodiac::NodeHandle>::iterator stoIt = nodeList.begin(); stoIt != nodeList.end(); ++stoIt)
        {
            if((*stoIt).getType() == zodiac::NODE_STORY)
            {
                QPointF nodePos = (*stoIt).getPos();

                if(nodePos.x() < xPos)
                    xPos = nodePos.x();

                if(nodePos.y() > yPos)
                    yPos = nodePos.y();
            }
        }

    //float maxY = -INFINITY;

    //space out new nodes
    for(QList<zodiac::NodeHandle>::iterator narIt = nodeList.begin(); narIt != nodeList.end(); ++narIt)
    {
        if(((*narIt).getType() == zodiac::NODE_NARRATIVE) && (*narIt).getPlug("reqOut").connectionCount() == 0)
        {
            (*narIt).setPos(xPos, yPos);

            spaceOutNarrativeChildren(new NodeCtrl(this, (*narIt)), yPos);

            yPos += 150;
        }
    }
}

void MainCtrl::spaceOutNarrativeChildren(NodeCtrl* sceneNode, float &maxY)
{
    //qDebug() << sceneNode->getName();
    //qDebug() << sceneNode->getPos();

    zodiac::PlugHandle inPlug = sceneNode->getNodeHandle().getPlug("reqIn");
    zodiac::PlugHandle outPlug = sceneNode->getNodeHandle().getPlug("reqOut");

    if(inPlug.isValid() && inPlug.isIncoming())
    {
        QList<zodiac::PlugHandle> connectedPlugs = inPlug.getConnectedPlugs();

        if(connectedPlugs.size() > 0)
        {
            float averageY = 0;
            float y = maxY;//sceneNode->getPos().y();

            for(QList<zodiac::PlugHandle>::iterator plugIt = connectedPlugs.begin(); plugIt != connectedPlugs.end(); ++ plugIt)
            {
                NodeCtrl* childNode = new NodeCtrl(this, (*plugIt).getNode());

                childNode->setPos(sceneNode->getPos().x() + 150, y);
                //childNode->setPos(sceneNode->getPos().x() + 150, sceneNode->getPos().y());
                spaceOutNarrativeChildren(childNode, y);

                averageY += childNode->getPos().y();
                y += 150;

                if(y < maxY)
                    maxY = y;
            }

            //averageY /= connectedPlugs.size();

            //qDebug() << sceneNode->getName() << "old Y" << sceneNode->getPos().y() << "new Y" << averageY/connectedPlugs.size();

            sceneNode->setPos(sceneNode->getPos().x(), averageY/connectedPlugs.size());
        }
    }

    //if(sceneNode->getPos().y() < minY)
        //minY = sceneNode->getPos().y();

     /*if(outPlug.isValid() && outPlug.isOutgoing())
     {
         QList<zodiac::PlugHandle> connectedPlugs = outPlug.getConnectedPlugs();

         if(connectedPlugs.size() > 1)
         {
             qDebug() << sceneNode->getName() << outPlug.getConnectedPlugs().size();

             float averageY = 0;
             float maxX = -INFINITY;

             for(QList<zodiac::PlugHandle>::iterator plugIt = connectedPlugs.begin(); plugIt != connectedPlugs.end(); ++ plugIt)
             {
                 QPointF nodePos = (*plugIt).getNode().getPos();

                 averageY += nodePos.y();

                 if(nodePos.x() > maxX)
                     maxX = nodePos.x();
             }

             averageY /= connectedPlugs.size();

             //sceneNode->setPos(maxX + 150, averageY);
             sceneNode->setPos(sceneNode->getPos().x(), averageY);
         }
     }*/

    //return YPos;
}

void MainCtrl::spaceOutStory()
{
    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();

    //find the main nodes
    zodiac::NodeHandle startingNode;
    zodiac::NodeHandle settingNode;
    zodiac::NodeHandle themeNode;
    zodiac::NodeHandle plotNode;
    zodiac::NodeHandle resolutionNode;
    //iterate through the list to find the nodes
    for(QList<zodiac::NodeHandle>::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        if((*it).getType() == zodiac::NODE_STORY)
        {
            if((*it).getStoryNodeType() == zodiac::STORY_NAME)
                startingNode = (*it); //get a pointer to the handle of the name node
            else
                if((*it).getStoryNodeType() == zodiac::STORY_SETTING)
                    settingNode = (*it); //get a pointer to the handle of the settings node
                else
                    if((*it).getStoryNodeType() == zodiac::STORY_THEME)
                        themeNode = (*it); //get a pointer to the handle of the theme node
                    else
                        if((*it).getStoryNodeType() == zodiac::STORY_PLOT)
                            plotNode = (*it); //get a pointer to the handle of the plot node
                        else
                            if((*it).getStoryNodeType() == zodiac::STORY_RESOLUTION)
                                resolutionNode = (*it); //get a pointer to the handle of the resolution node
        }
    }

    float maxX = 0.0f;
    float currentY = settingNode.getPos().y();  //all four nodes will be the same height
    maxX = spaceOutChildNodes(settingNode, maxX, currentY);
    maxX = spaceOutChildNodes(themeNode, maxX, currentY);
    maxX = spaceOutChildNodes(plotNode, maxX, currentY);
    maxX = spaceOutChildNodes(resolutionNode, maxX, currentY);

    //centre starting node and then recentre the full graph
    startingNode.setPos((settingNode.getPos().x() + themeNode.getPos().x() + plotNode.getPos().x() + resolutionNode.getPos().x())/4, startingNode.getPos().y());
    startingNode.setPos(0, 0, true);

}

float MainCtrl::spaceOutChildNodes(zodiac::NodeHandle &node, float &xPos, float &yPos)
{
    float maxX = -INFINITY;

    if(node.getPlug("storyOut").isValid())
    {
        QList<zodiac::PlugHandle> connectedPlugs = node.getPlug("storyOut").getConnectedPlugs();

        if(!connectedPlugs.empty())
        {
            node.setPos(xPos, yPos);

            QList<zodiac::NodeHandle> childNodes;
            for(QList<zodiac::PlugHandle>::iterator plugIt = connectedPlugs.begin(); plugIt != connectedPlugs.end(); ++plugIt)
            {
                childNodes.push_back((*plugIt).getNode());
            }

            float nodeXPos = node.getPos().x();
            float nodeYPos = node.getPos().y() + 150;
            float childPos = 0.0f;
            for(QList<zodiac::NodeHandle>::iterator nodeIt = childNodes.begin(); nodeIt != childNodes.end(); ++nodeIt)
            {
                float childMaxX = spaceOutChildNodes((*nodeIt), nodeXPos, nodeYPos);

                    if(childMaxX > maxX)
                        maxX = childMaxX;

                childPos += (*nodeIt).getPos().x();
            }
            node.setPos(childPos/childNodes.size(), node.getPos().y());

            xPos = maxX;
        }
        else
        {
            node.setPos(xPos, yPos);

            xPos += 150;

            if(xPos > maxX)
                    maxX = xPos;
        }

        return maxX;
    }
    else
    {
        qDebug() << "error";
        return 0.0f;
    }
}

void MainCtrl::showLinkerWindow(zodiac::NodeHandle &node)
{
    if(m_linkWindow)
    {
        delete m_linkWindow;
        m_linkWindow = nullptr;
    }

    m_linkWindow = new LinkerWindow(node, m_scene.getNodes(), qobject_cast<QWidget*>(parent()));
    m_linkWindow->show();

    connect(m_linkWindow, SIGNAL(linkNarrativeNodes(zodiac::NodeHandle&, QList<zodiac::NodeHandle>&, QList<zodiac::NodeHandle>&)),
            this, SLOT(linkNarrativeNodes(zodiac::NodeHandle&, QList<zodiac::NodeHandle>&, QList<zodiac::NodeHandle>&)));

    connect(m_linkWindow, SIGNAL(linkStoryNodes(zodiac::NodeHandle&, QList<zodiac::NodeHandle>&)),
            this, SLOT(linkStoryNodes(zodiac::NodeHandle&, QList<zodiac::NodeHandle>&)));
}

void MainCtrl::linkNarrativeNodes(zodiac::NodeHandle &node, QList<zodiac::NodeHandle> &nodeList, QList<zodiac::NodeHandle> &inverseNodeList)
{
    //if node list is more than one, make a seq node then link them
    zodiac::NodeHandle *nodePtr = &node;

    if(!nodeList.empty())
    {
        if(nodeList.size() > 1 || nodePtr->getPlug("reqIn").connectionCount() != 0) //use seq if more than one connection
        {
            //check for existing sequence node
            QList<zodiac::PlugHandle> connectedPlugs = nodePtr->getPlug("reqIn").getConnectedPlugs();

            foreach(zodiac::PlugHandle plug, connectedPlugs)
            {
                if(plug.getNode().getName() == "SEQ")
                {
                    nodePtr = &plug.getNode();
                    break;
                }
            }

            if(nodePtr->getName() != "SEQ")
            {
                //check if node already exists to add to the sequence
                QList<zodiac::NodeHandle> connectedNodes;   //should only be one but add list in case
                if(nodePtr->getPlug("reqIn").connectionCount() > 0)
                {
                    QList<zodiac::PlugHandle> connectedPlugs = nodePtr->getPlug("reqIn").getConnectedPlugs();

                    foreach(zodiac::PlugHandle plug, connectedPlugs)
                    {
                        connectedNodes.push_back(plug.getNode());
                    }
                }

                nodePtr = &createNode(zodiac::STORY_NONE, "SEQ", "")->getNodeHandle();
                nodePtr->setIdleColor(QColor(255, 204, 0));
                nodePtr->setSelectedColor(QColor(255, 153, 0));

                node.getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"));

                foreach(zodiac::NodeHandle connectedNode, connectedNodes)   //disconnect from previous node and connect to new sequence node
                {
                    connectedNode.getPlug("reqIn").disconnectPlug(node.getPlug("reqOut"));
                    connectedNode.getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"));
                }

                QSet<zodiac::PlugEdge*> edgeList = node.getPlug("reqIn").getEdges();
                for(QSet<zodiac::PlugEdge*>::iterator plugIt = edgeList.begin(); plugIt != edgeList.end(); ++plugIt)
                {
                    (*plugIt)->setBaseColor(QColor(66, 134, 244));
                }

            }
        }

        for(QList<zodiac::NodeHandle>::iterator nodeIt = nodeList.begin(); nodeIt != nodeList.end(); ++ nodeIt)
        {
            nodePtr->getPlug("reqIn").connectPlug((*nodeIt).getPlug("reqOut"));

            QSet<zodiac::PlugEdge*> edgeList = nodePtr->getPlug("reqIn").getEdges();
            for(QSet<zodiac::PlugEdge*>::iterator plugIt = edgeList.begin(); plugIt != edgeList.end(); ++plugIt)
            {
                (*plugIt)->setBaseColor(QColor(66, 134, 244));
            }
        }
    }

    if(!inverseNodeList.empty())
    {
        zodiac::NodeHandle *oldNodePtr;

        //check for sequence or inverter node if they already exist
        if(nodePtr->getName() != "SEQ" && nodePtr->getPlug("reqIn").connectionCount() > 0)
        {
            QList<zodiac::NodeHandle> connectedNodes;   //get a list of connected nodes, should only be one but add list in case

            if(nodePtr->getPlug("reqIn").connectionCount() > 0)
            {
                QList<zodiac::PlugHandle> connectedPlugs = nodePtr->getPlug("reqIn").getConnectedPlugs();

                foreach(zodiac::PlugHandle plug, connectedPlugs) //find sequence or inverse node in list, sent to nodeptr if found
                {
                    if(plug.getNode().getName() == "SEQ" || plug.getNode().getName() == "INV")
                    {
                        nodePtr = &plug.getNode();
                        break;
                    }
                    else
                        connectedNodes.push_back(plug.getNode());
                }

                if(nodePtr->getName() == "SEQ") //check for inv in seq
                {
                    connectedPlugs = nodePtr->getPlug("reqIn").getConnectedPlugs();

                     foreach(zodiac::PlugHandle plug, connectedPlugs) //find sequence or inverse node in list, sent to nodeptr if found
                     {
                         if(plug.getNode().getName() == "INV")
                         {
                             nodePtr = &plug.getNode();
                             break;
                         }
                     }
                }
                }

            //if not found, will need to create sequence node and set to nodeptr
            if(nodePtr->getName() != "SEQ" && nodePtr->getName() != "INV")
            {
                oldNodePtr = nodePtr;

                nodePtr = &createNode(zodiac::STORY_NONE, "SEQ", "")->getNodeHandle();
                nodePtr->setIdleColor(QColor(255, 204, 0));
                nodePtr->setSelectedColor(QColor(255, 153, 0));

                node.getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"));

                foreach(zodiac::NodeHandle connectedNode, connectedNodes)
                {//disconnect from previous node and connect to new sequence node
                    connectedNode.getPlug("reqIn").disconnectPlug(oldNodePtr->getPlug("reqOut"));
                    connectedNode.getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"));
                }

                QSet<zodiac::PlugEdge*> edgeList = oldNodePtr->getPlug("reqIn").getEdges();
                for(QSet<zodiac::PlugEdge*>::iterator plugIt = edgeList.begin(); plugIt != edgeList.end(); ++plugIt)
                {
                    (*plugIt)->setBaseColor(QColor(66, 134, 244));
                }

            }
        }

        oldNodePtr = nodePtr;

        if(nodePtr->getName() != "INV")
        {
            nodePtr = &createNode(zodiac::STORY_NONE, "INV", "")->getNodeHandle();
            nodePtr->setIdleColor(QColor(255, 204, 0));
            nodePtr->setSelectedColor(QColor(255, 153, 0));

            oldNodePtr->getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"));

            QSet<zodiac::PlugEdge*> edgeList = oldNodePtr->getPlug("reqIn").getEdges();
            for(QSet<zodiac::PlugEdge*>::iterator plugIt = edgeList.begin(); plugIt != edgeList.end(); ++plugIt)
            {
                (*plugIt)->setBaseColor(QColor(66, 134, 244));
            }
        }

        if(inverseNodeList.size() > 1 || nodePtr->getPlug("reqIn").connectionCount() > 0)
        {
            //check if node already exists to add to the sequence
            QList<zodiac::NodeHandle> connectedNodes;   //should only be one but add list in case
            if(nodePtr->getPlug("reqIn").connectionCount() > 0)
            {
                QList<zodiac::PlugHandle> connectedPlugs = nodePtr->getPlug("reqIn").getConnectedPlugs();

                foreach(zodiac::PlugHandle plug, connectedPlugs)
                {
                    connectedNodes.push_back(plug.getNode());
                }
            }

            oldNodePtr = nodePtr;

            nodePtr = &createNode(zodiac::STORY_NONE, "SEQ", "")->getNodeHandle();
            nodePtr->setIdleColor(QColor(255, 204, 0));
            nodePtr->setSelectedColor(QColor(255, 153, 0));

            oldNodePtr->getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"));

            foreach(zodiac::NodeHandle connectedNode, connectedNodes)
            {//disconnect from previous node and connect to new sequence node
                connectedNode.getPlug("reqIn").disconnectPlug(oldNodePtr->getPlug("reqOut"));
                connectedNode.getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"));
            }
        }

        for(QList<zodiac::NodeHandle>::iterator nodeIt = inverseNodeList.begin(); nodeIt != inverseNodeList.end(); ++ nodeIt)
        {
            nodePtr->getPlug("reqIn").connectPlug((*nodeIt).getPlug("reqOut"));

            QSet<zodiac::PlugEdge*> edgeList = nodePtr->getPlug("reqIn").getEdges();
            for(QSet<zodiac::PlugEdge*>::iterator plugIt = edgeList.begin(); plugIt != edgeList.end(); ++plugIt)
            {
                (*plugIt)->setBaseColor(QColor(66, 134, 244));
            }
        }
    }

    spaceOutFullNarrative();

    m_propertyEditor->UpdateLinkerValues(m_scene.getNodes());
}

void MainCtrl::linkStoryNodes(zodiac::NodeHandle &node, QList<zodiac::NodeHandle> &nodeList)
{
    if(nodeList.size() > 0)
    {
        for(QList<zodiac::NodeHandle>::iterator nodeIt = nodeList.begin(); nodeIt != nodeList.end(); ++nodeIt)
        {
            //make plugs and connect
            if(!node.getPlug("storyOut").isValid())
                node.createOutgoingPlug("storyOut");
            if(!(*nodeIt).getPlug("narrativeIn").isValid())
                (*nodeIt).createIncomingPlug("narrativeIn");

            node.getPlug("storyOut").connectPlug((*nodeIt).getPlug("narrativeIn")); //connect nodes

            (*nodeIt).setLabelBackgroundColor(QColor(2, 202, 0));
        }


        QSet<zodiac::PlugEdge*> edgeList = node.getPlug("storyOut").getEdges();
        for(QSet<zodiac::PlugEdge*>::iterator plugIt = edgeList.begin(); plugIt != edgeList.end(); ++plugIt)
        {
            (*plugIt)->setBaseColor(QColor(0, 184, 13));
        }

        m_propertyEditor->UpdateLinkerValues(m_scene.getNodes());
    }
}

void MainCtrl::lockAllNodes()
{
    QList<zodiac::NodeHandle> currentNodes =  m_scene.getNodes();

    QList<zodiac::NodeHandle> invNodes;

    for(QList<zodiac::NodeHandle>::iterator cNIt = currentNodes.begin(); cNIt != currentNodes.end(); ++cNIt)
    {
        if((*cNIt).isNodeDecorator())   //not the INV and SEQ nodes
        {
            if((*cNIt).getName() == "INV")
                invNodes.push_back(((*cNIt)));

            continue;
        }

        (*cNIt).setIdleColor(QColor("#ff1800"));
        (*cNIt).setSelectedColor(QColor("#ff331e"));
    }

    for(QList<zodiac::NodeHandle>::iterator iNIt = invNodes.begin(); iNIt != invNodes.end(); ++iNIt)
    {
        if(((*iNIt)).getPlug("reqOut").isValid())
        {
            QList<zodiac::PlugHandle> InvInPlugs = (*iNIt).getPlug("reqOut").getConnectedPlugs();
            //this should only return one node
            if(InvInPlugs.size() > 1) qDebug() << "Error: more than one node without a sequencer";
            for(QList<zodiac::PlugHandle>::iterator plugIt = InvInPlugs.begin(); plugIt != InvInPlugs.end(); ++plugIt)
            {
                if((*plugIt).getNode().getName() == "SEQ")
                {
                    //check reqOut plug to see if all nodes are unlocked
                    if((*plugIt).getNode().getPlug("reqOut").isValid())
                    {
                        QList<zodiac::NodeHandle> seqInNodes;
                        QList<zodiac::PlugHandle> seqInPlugs = (*plugIt).getNode().getPlug("reqOut").getConnectedPlugs();
                        for(QList<zodiac::PlugHandle>::iterator plugIt = seqInPlugs.begin(); plugIt != seqInPlugs.end(); ++plugIt)
                        {
                            seqInNodes.push_back((*plugIt).getNode());
                        }

                        //if true then lock node, otherwise it's still unlockable
                        if(areAllNodesUnlocked(seqInNodes))
                        {
                            (*iNIt).setIdleColor(QColor("#ff1800"));
                            (*iNIt).setSelectedColor(QColor("#ff331e"));
                        }
                        else
                        {
                            (*iNIt).setIdleColor(QColor("#3333cc"));
                            (*iNIt).setIdleColor(QColor("#4949cc"));
                        }
                    }
                }
                else
                {
                    if((*plugIt).getNode().getLockedStatus())
                    {
                        (*iNIt).setIdleColor(QColor("#3333cc"));    //node locked, unlockable
                        (*iNIt).setIdleColor(QColor("#4949cc"));
                    }
                    else
                    {
                        (*iNIt).setIdleColor(QColor("#ff1800"));        //node unlocked, lock
                        (*iNIt).setSelectedColor(QColor("#ff331e"));
                    }
                }
            }
        }
    }

}

void MainCtrl::resetAllNodes()
{
    QList<zodiac::NodeHandle> currentNodes =  m_scene.getNodes();

    for(QList<zodiac::NodeHandle>::iterator cNIt = currentNodes.begin(); cNIt != currentNodes.end(); ++cNIt)
    {
        (*cNIt).setIdleColor(QColor("#4b77a7"));
        (*cNIt).setSelectedColor(QColor("#62abfa"));
        (*cNIt).setLockedStatus(true);
    }
}

void MainCtrl::unlockNode(QString nodeName)
{
    QList<zodiac::NodeHandle> currentNodes =  m_scene.getNodes();
    bool found = false;

    for(QList<zodiac::NodeHandle>::iterator cNIt = currentNodes.begin(); cNIt != currentNodes.end(); ++cNIt)
    {
        if((*cNIt).getType() == zodiac::NODE_NARRATIVE && (*cNIt).getName() == nodeName)
        {
            found = true;

            //unlocked change colour of node to green to show unlocked
            (*cNIt).setLockedStatus(false);
            (*cNIt).setIdleColor(QColor("#00cc00"));
            (*cNIt).setSelectedColor(QColor("#5bff5b"));

            //change colour of story nodes to green as now unlocked
            if(((*cNIt)).getPlug("storyOut").isValid())
            {
                QList<zodiac::NodeHandle> storyOutNodes;
                QList<zodiac::PlugHandle> storyOutPlugs = (*cNIt).getPlug("storyOut").getConnectedPlugs();
                for(QList<zodiac::PlugHandle>::iterator plugIt = storyOutPlugs.begin(); plugIt != storyOutPlugs.end(); ++plugIt)
                {
                    storyOutNodes.push_back((*plugIt).getNode());
                }

                for(QList<zodiac::NodeHandle>::iterator sNIt = storyOutNodes.begin(); sNIt != storyOutNodes.end(); ++sNIt)
                {
                    (*sNIt).setIdleColor(QColor("#00cc00"));
                    (*cNIt).setSelectedColor(QColor("#5bff5b"));
                }
            }

            //check if other nodes are unlockable, turn them blue
            if((*cNIt).getPlug("reqIn").isValid())
            {
                QList<zodiac::NodeHandle> reqNodes;
                QList<zodiac::PlugHandle> reqPlugs = (*cNIt).getPlug("reqIn").getConnectedPlugs();

                //get all nodes which require node to be unlocked first
                for(QList<zodiac::PlugHandle>::iterator plugIt = reqPlugs.begin(); plugIt != reqPlugs.end(); ++plugIt)
                {
                    reqNodes.push_back((*plugIt).getNode());
                }

                showUnlockableNodes(reqNodes);
            }

            break;
        }
    }

    if(!found)
    {
        /*QMessageBox messageBox;
        messageBox.critical(0,"Error","Node unlocked which does not exist in the narrative graph.\nPlease ensure that the correct and complete graph is loaded");
        messageBox.setFixedSize(500,200);*/
    }
}

void MainCtrl::showUnlockableNodes(QList<zodiac::NodeHandle> &nodes)
{
    for(QList<zodiac::NodeHandle>::iterator rNIt = nodes.begin(); rNIt != nodes.end(); ++rNIt)
    {
        if((*rNIt).getName() == "SEQ")
        {
            //check reqOut plug to see if all nodes are unlocked
            if(((*rNIt)).getPlug("reqOut").isValid())
            {
                QList<zodiac::NodeHandle> seqInNodes;
                QList<zodiac::PlugHandle> seqInPlugs = (*rNIt).getPlug("reqOut").getConnectedPlugs();
                for(QList<zodiac::PlugHandle>::iterator plugIt = seqInPlugs.begin(); plugIt != seqInPlugs.end(); ++plugIt)
                {
                    seqInNodes.push_back((*plugIt).getNode());
                }

                //if true then use reqIn to change to blue
                if(areAllNodesUnlocked(seqInNodes))
                {
                    QList<zodiac::NodeHandle> seqOutNodes;
                    QList<zodiac::PlugHandle> seqOutPlugs = (*rNIt).getPlug("reqIn").getConnectedPlugs();
                    for(QList<zodiac::PlugHandle>::iterator plugIt = seqOutPlugs.begin(); plugIt != seqOutPlugs.end(); ++plugIt)
                    {
                        seqOutNodes.push_back((*plugIt).getNode());
                    }

                    showUnlockableNodes(seqOutNodes);
                }
            }
        }
        else
            if((*rNIt).getName() == "INV")
            {
                //node should be now locked
                //check for possible seq node
                if(((*rNIt)).getPlug("reqOut").isValid())
                {
                    QList<zodiac::PlugHandle> InvInPlugs = (*rNIt).getPlug("reqOut").getConnectedPlugs();
                    //this should only return one node
                    if(InvInPlugs.size() > 1) qDebug() << "Error: more than one node without a sequencer";
                    for(QList<zodiac::PlugHandle>::iterator plugIt = InvInPlugs.begin(); plugIt != InvInPlugs.end(); ++plugIt)
                    {
                        if((*plugIt).getNode().getName() == "SEQ")
                        {
                            //check reqOut plug to see if all nodes are unlocked
                            if((*plugIt).getNode().getPlug("reqOut").isValid())
                            {
                                QList<zodiac::NodeHandle> seqInNodes;
                                QList<zodiac::PlugHandle> seqInPlugs = (*plugIt).getNode().getPlug("reqOut").getConnectedPlugs();
                                for(QList<zodiac::PlugHandle>::iterator plugIt = seqInPlugs.begin(); plugIt != seqInPlugs.end(); ++plugIt)
                                {
                                    seqInNodes.push_back((*plugIt).getNode());
                                }

                                //if true then lock node, otherwise it's still unlockable
                                if(areAllNodesUnlocked(seqInNodes))
                                {
                                    (*rNIt).setIdleColor(QColor("#ff1800"));
                                    (*rNIt).setSelectedColor(QColor("#ff331e"));
                                }
                                else
                                {
                                    (*rNIt).setIdleColor(QColor("#3333cc"));    //unlockable
                                    (*rNIt).setIdleColor(QColor("#4949cc"));
                                }
                            }
                        }
                        else
                        {
                            (*rNIt).setIdleColor(QColor("#ff1800"));    //lock it
                            (*rNIt).setSelectedColor(QColor("#ff331e"));
                        }
                    }
                }

            }
            else
            {
                (*rNIt).setIdleColor(QColor("#3333cc"));
                (*rNIt).setIdleColor(QColor("#4949cc"));
            }
    }
}

bool MainCtrl::areAllNodesUnlocked(QList<zodiac::NodeHandle> &nodes)
{
    for(QList<zodiac::NodeHandle>::iterator rNIt = nodes.begin(); rNIt != nodes.end(); ++rNIt)
    {
        if((*rNIt).getName() == "SEQ")
        {
            //check reqIn plug to see if all nodes are unlocked
            if(((*rNIt)).getPlug("reqIn").isValid())
            {
                QList<zodiac::NodeHandle> seqInNodes;
                QList<zodiac::PlugHandle> seqInPlugs = (*rNIt).getPlug("reqIn").getConnectedPlugs();
                for(QList<zodiac::PlugHandle>::iterator plugIt = seqInPlugs.begin(); plugIt != seqInPlugs.end(); ++plugIt)
                {
                    seqInNodes.push_back((*plugIt).getNode());
                }

                if(!areAllNodesUnlocked(seqInNodes))
                    return false;
            }
        }
        else
            if((*rNIt).getName() == "INV")
            {
                if(((*rNIt)).getPlug("reqOut").isValid())
                {
                    QList<zodiac::NodeHandle> invNodes;
                    QList<zodiac::PlugHandle> invPlugs = (*rNIt).getPlug("reqOut").getConnectedPlugs();

                    //get all nodes which require node to be unlocked first
                    for(QList<zodiac::PlugHandle>::iterator invPlugIt = invPlugs.begin(); invPlugIt != invPlugs.end(); ++invPlugIt)
                    {
                        invNodes.push_back((*invPlugIt).getNode());
                    }

                    for(QList<zodiac::NodeHandle>::iterator invNodeIt = invNodes.begin(); invNodeIt != invNodes.end(); ++invNodeIt)
                    {
                        if((*invNodeIt).getName() == "SEQ")
                        {
                            QList<zodiac::NodeHandle> seqNodes;
                            QList<zodiac::PlugHandle> seqPlugs = (*invNodeIt).getPlug("reqOut").getConnectedPlugs();

                            //get all nodes which require node to be unlocked first
                            for(QList<zodiac::PlugHandle>::iterator seqPlugIt = seqPlugs.begin(); seqPlugIt != seqPlugs.end(); ++seqPlugIt)
                            {
                                seqNodes.push_back((*seqPlugIt).getNode());
                            }

                            if(areAllNodesUnlocked(seqNodes))   //inverse so returning true means not unlocked
                                return false;
                        }
                        else
                            if(!(*rNIt).getLockedStatus())  //if unlocked then fail
                                return false;
                    }
                }
            }
            else
                {
                     if((*rNIt).getLockedStatus())
                         return false;
                }
    }

    return true;
}

void MainCtrl::checkNarrativeAndStoryGraphsLoaded()
{
    bool loadStory = true;
    bool loadNarrative = true;

    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();

    foreach(zodiac::NodeHandle node, nodes)
    {
        if(node.getType() == zodiac::NODE_STORY)
            loadStory = false;
        else
            if(node.getType() == zodiac::NODE_NARRATIVE)
                loadNarrative = false;
    }

    if(loadStory)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("No Story Graph Loaded");
        msgBox.setText("Do you want to load a story graph before starting analytics?");
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if(msgBox.exec() == QMessageBox::Yes)
        {
          loadStoryGraph();
        }
    }

    if(loadNarrative)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("No Narrative Graph Loaded");
        msgBox.setText("Do you want to load a narrative graph before starting analytics?");
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if(msgBox.exec() == QMessageBox::Yes)
        {
          loadNarrativeGraph();
        }
    }
}

void MainCtrl::updateAnalyticsProperties()
{
    m_propertyEditor->UpdateLinkerValues(m_scene.getNodes());
}
