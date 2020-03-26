#include "mainctrl.h"

#include <QDebug>
#include <cstdlib>

#include "nodectrl.h"
#include "propertyeditor.h"
#include "zodiacgraph/nodehandle.h"

QColor storyLinkColor("#cc5d4e");
QColor narrativeLinkColor("#4286f4");
QColor storyNarrativeLinkColor("#00b80d");


QString MainCtrl::s_defaultName = "Node ";

MainCtrl::MainCtrl(QObject *parent, zodiac::Scene* scene, PropertyEditor* propertyEditor, AnalyticsHandler* analyticsHandler, QAction* newStoryNodeAction, QUndoStack *undoStack)
    : QObject(parent)
    , m_scene(zodiac::SceneHandle(scene))
    , m_propertyEditor(propertyEditor)
    , m_nodes(QHash<zodiac::NodeHandle, NodeCtrl*>())
    , m_nodeIndex(1)            // name suffixes start at 1
    , m_analytics(analyticsHandler)
    , m_narrativeSorter(new NarrativeFileSorter(qobject_cast<QWidget*>(parent)))
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

    connect(m_analytics, &AnalyticsHandler::checkForGraphs,
        [=]{ checkGraphLoaded(zodiac::NODE_STORY); checkGraphLoaded(zodiac::NODE_NARRATIVE);});

    connect(m_analytics, &AnalyticsHandler::closeNodeProperties,
        &m_scene, &zodiac::SceneHandle::deselectAll);

    connect(m_analytics, &AnalyticsHandler::lockAllNodes,
        this, &MainCtrl::lockAllNodes);

    connect(m_analytics, &AnalyticsHandler::resetNodes,
        this, &MainCtrl::resetAllNodes);

    connect(m_narrativeSorter, SIGNAL(loadOrderedNarrative(QVector<QString>)),
            this, SLOT(spaceOutNarrative(QVector<QString>)));
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

    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();
    if(node->getType() == zodiac::NODE_STORY) //if no more story nodes, allow a new graph to be created
    {
        bool noStoryNodes = true;

        foreach (zodiac::NodeHandle n, nodes)
        {
            if(n == node->getNodeHandle())
                continue;

            if(n.getType() == zodiac::NODE_STORY)
            {
                noStoryNodes = false;
                break;
            }
        }

        if(noStoryNodes)
             m_createStoryAction->setEnabled(true);
    }
    else    //otherwise remove any unused filenames
    {
        QString fileName = node->getFileName();

        foreach (zodiac::NodeHandle n, nodes)
        {
            if(n == node->getNodeHandle())
                continue;

            if(n.getType() == zodiac::NODE_NARRATIVE && n.getFileName() == fileName)
            {
                m_saveAndLoadManager.removeFileName(fileName);
                m_narrativeSorter->removeFromOrderedList(fileName);
                break;
            }
        }
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

    ParentNodeOutPlug.connectPlug(childNodeInPlug, storyLinkColor);

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
    nameNodeOutPlug.connectPlug(settingNodeInPlug, storyLinkColor);
    zodiac::PlugHandle themeNodeInPlug = themeNode->getNodeHandle().getPlug("storyIn");
    nameNodeOutPlug.connectPlug(themeNodeInPlug, storyLinkColor);
    zodiac::PlugHandle plotNodeInPlug = plotNode->getNodeHandle().getPlug("storyIn");
    nameNodeOutPlug.connectPlug(plotNodeInPlug, storyLinkColor);
    zodiac::PlugHandle resolutionNodeInPlug = resolutionNode->getNodeHandle().getPlug("storyIn");
    nameNodeOutPlug.connectPlug(resolutionNodeInPlug, storyLinkColor);

    m_createStoryAction->setEnabled(false);
}

void MainCtrl::saveStoryGraph()
{
    //get list of all story nodes
    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();
    NodeCtrl *mainStoryNode = nullptr;
    NodeCtrl *settingNode = nullptr;
    NodeCtrl *themeNode = nullptr;
    NodeCtrl *plotNode = nullptr;
    NodeCtrl *resolutionNode = nullptr;

    //NodeCtrl *parentNodeCtrl = new NodeCtrl(this, *parentNode);

    //iterate through the list to find the nodes
    foreach (zodiac::NodeHandle node, nodes)
    {
        if(node.getType() == zodiac::NODE_STORY)
        {
            if(node.getStoryNodeType() == zodiac::STORY_NAME)
                mainStoryNode = new NodeCtrl(this, node); //get a pointer to the handle of the name node

            else
                if(node.getStoryNodeType() == zodiac::STORY_SETTING)
                    settingNode = new NodeCtrl(this, node); //get a pointer to the handle of the settings node

                else
                    if(node.getStoryNodeType() == zodiac::STORY_THEME)
                        themeNode = new NodeCtrl(this, node); //get a pointer to the handle of the theme node

                    else
                        if(node.getStoryNodeType() == zodiac::STORY_PLOT)
                            plotNode = new NodeCtrl(this, node); //get a pointer to the handle of the plot node
                        else
                            if(node.getStoryNodeType() == zodiac::STORY_RESOLUTION)
                                resolutionNode = new NodeCtrl(this, node); //get a pointer to the handle of the resolution node
        }
    }

    m_saveAndLoadManager.DeleteAllStoryItems(); //clear the manager just in case

    //store story name
    m_saveAndLoadManager.setStoryName(mainStoryNode->getName());

    //save settings nodes
    QList<zodiac::PlugHandle> connectedPlugs = settingNode->getNodeHandle().getPlug("storyOut").getConnectedPlugs();

    foreach (zodiac::PlugHandle cPlug, connectedPlugs)
        saveSettingItem(cPlug.getNode());

    //save theme nodes
    connectedPlugs = themeNode->getNodeHandle().getPlug("storyOut").getConnectedPlugs();

    foreach (zodiac::PlugHandle cPlug, connectedPlugs)
        saveThemeItem(cPlug.getNode());

    //get plot node
    //get each episode node - store, store subgoal, iterate through attempts, store these and sub episodes, same with outcomes
    savePlotItem(plotNode->getNodeHandle());

    //get resolution node
    saveResolution(resolutionNode->getNodeHandle());

    m_saveAndLoadManager.SaveStoryToFile(qobject_cast<QWidget*>(parent()));
}

void MainCtrl::saveSettingItem(zodiac::NodeHandle &settingGroup)
{
    QList<zodiac::PlugHandle> connectedPlugs = settingGroup.getPlug("storyOut").getConnectedPlugs();
    foreach (zodiac::PlugHandle cPlug, connectedPlugs)
    {
        zodiac::NodeHandle settingNode = cPlug.getNode();
        SettingItem *settingItem;

        if(settingNode.getStoryNodeType() == zodiac::STORY_SETTING_CHARACTER)
            settingItem = m_saveAndLoadManager.addCharacter(settingNode.getName(), settingNode.getDescription());
        else
            if(settingNode.getStoryNodeType() == zodiac::STORY_SETTING_LOCATION)
                settingItem = m_saveAndLoadManager.addLocation(settingNode.getName(), settingNode.getDescription());
            else
                if(settingNode.getStoryNodeType() == zodiac::STORY_SETTING_TIME)
                    settingItem = m_saveAndLoadManager.addTime(settingNode.getName(), settingNode.getDescription());
                else
                    return; //error

            QList<zodiac::PlugHandle> detailPlugs = settingNode.getPlug("storyOut").getConnectedPlugs();
            foreach (zodiac::PlugHandle dPlug, detailPlugs)
            {
                zodiac::NodeHandle detailNode = dPlug.getNode();
                m_saveAndLoadManager.addDetail(settingItem, detailNode.getName(), detailNode.getDescription(), "", "");
            }
    }
}

void MainCtrl::saveThemeItem(zodiac::NodeHandle &parent, EventGoal *parentItem)
{
    QList<zodiac::PlugHandle> connectedPlugs = parent.getPlug("storyOut").getConnectedPlugs();
    foreach (zodiac::PlugHandle cPlug, connectedPlugs)
    {
        zodiac::NodeHandle eventGoalNode = cPlug.getNode();
        EventGoal *eventGoalItem;

       eventGoalItem = m_saveAndLoadManager.addEventGoal(eventGoalNode.getName(), eventGoalNode.getDescription(), eventGoalNode.getStoryNodeType(), parentItem);

       if(parent.getPlug("storyOut").connectionCount() > 0)
           saveThemeItem(eventGoalNode, eventGoalItem);
    }
}

void MainCtrl::savePlotItem(zodiac::NodeHandle &parent, Episode *parentItem, zodiac::StoryNodeType type)
{
    QList<zodiac::PlugHandle> connectedPlugs = parent.getPlug("storyOut").getConnectedPlugs();
    foreach (zodiac::PlugHandle cPlug, connectedPlugs)
    {
        zodiac::NodeHandle episodeNode = cPlug.getNode();
        Episode *episodeItem;

        if(episodeNode.getStoryNodeType() == zodiac::STORY_PLOT_EPISODE || episodeNode.getStoryNodeType() == zodiac::STORY_PLOT_SUBEPISODE)
            episodeItem = m_saveAndLoadManager.addEpisode(episodeNode.getName(), episodeNode.getDescription(), parentItem, type);
        else
            continue; //error or we're looking for a subepisode

        QList<zodiac::PlugHandle> childPlugs = episodeNode.getPlug("storyOut").getConnectedPlugs();
        foreach (zodiac::PlugHandle childPlug, childPlugs)
        {
            zodiac::NodeHandle childNode = childPlug.getNode();

            if(childNode.getStoryNodeType() == zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP)
            {
                QList<zodiac::PlugHandle> attemptPlugs = childNode.getPlug("storyOut").getConnectedPlugs();
                for(QList<zodiac::PlugHandle>::iterator attemptPlugIt = attemptPlugs.begin(); attemptPlugIt != attemptPlugs.end(); ++attemptPlugIt)
                {
                    zodiac::NodeHandle attemptNode = (*attemptPlugIt).getNode();

                    if(attemptNode.getStoryNodeType() == zodiac::STORY_PLOT_EPISODE_ATTEMPT)
                        m_saveAndLoadManager.addAttempt(attemptNode.getName(), attemptNode.getDescription(), episodeItem);
                    else //sup-episode
                        savePlotItem(childNode, episodeItem, childNode.getStoryNodeType());
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
                            savePlotItem(childNode, episodeItem, childNode.getStoryNodeType());
                    }
                }
                else
                    if(childNode.getStoryNodeType() == zodiac::STORY_PLOT_EPISODE_SUBGOAL)
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
    foreach (zodiac::PlugHandle cPlug, connectedPlugs)
    {
        if(cPlug.getNode().getStoryNodeType() == zodiac::STORY_RESOLUTION_EVENT_GROUP)
            eventGroupNode = cPlug.getNode();
        else
            if(cPlug.getNode().getStoryNodeType() == zodiac::STORY_RESOLUTION_STATE_GROUP)
                stateGroupNode = cPlug.getNode();
            else
                return; //error
    }

    saveThemeItem(eventGroupNode);  //use theme function as resolution event identical to theme event

    QList<zodiac::PlugHandle> statePlugs = stateGroupNode.getPlug("storyOut").getConnectedPlugs();
    foreach (zodiac::PlugHandle sPlug, statePlugs)
    {
        zodiac::NodeHandle stateNode = sPlug.getNode();
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
        NodeCtrl *startingNode;
        NodeCtrl *settingNode;
        NodeCtrl *themeNode;
        NodeCtrl *plotNode;
        NodeCtrl *resolutionNode;
        //iterate through the list to find the nodes
        foreach (zodiac::NodeHandle node, nodes)
        {
            if(node.getType() == zodiac::NODE_STORY)
            {
                if(node.getStoryNodeType() == zodiac::STORY_NAME)
                    startingNode = new NodeCtrl(this, node); //get a pointer to the handle of the name node
                else
                    if(node.getStoryNodeType() == zodiac::STORY_SETTING)
                        settingNode = new NodeCtrl(this, node); //get a pointer to the handle of the settings node
                    else
                        if(node.getStoryNodeType() == zodiac::STORY_THEME)
                            themeNode = new NodeCtrl(this, node); //get a pointer to the handle of the theme node
                        else
                            if(node.getStoryNodeType() == zodiac::STORY_PLOT)
                                plotNode = new NodeCtrl(this, node); //get a pointer to the handle of the plot node
                            else
                                if(node.getStoryNodeType() == zodiac::STORY_RESOLUTION)
                                    resolutionNode = new NodeCtrl(this, node); //get a pointer to the handle of the resolution node
            }
        }

        zodiac::PlugHandle SettingNodeOutPlug = settingNode->getNodeHandle().getPlug("storyOut");   //get the outgoing plugs
        zodiac::PlugHandle ThemeNodeOutPlug = themeNode->getNodeHandle().getPlug("storyOut");

        //load the setting items
        QList<SettingItem> chars = m_saveAndLoadManager.GetCharacters();
        NodeCtrl* characterNode = nullptr;
        if(chars.size() > 0)
        {
            //create parent group node, move it, link to main settings node and set up plug for linking children
            characterNode = createNode(zodiac::STORY_SETTING_CHARACTER_GROUP, "Characters", "Characters Group");

            zodiac::PlugHandle characterNodeInPlug = characterNode->getNodeHandle().getPlug("storyIn");
            SettingNodeOutPlug.connectPlug(characterNodeInPlug, storyLinkColor);

            loadSettingItem(characterNode, chars, zodiac::STORY_SETTING_CHARACTER);
        }

        QList<SettingItem> locs = m_saveAndLoadManager.GetLocations();
        NodeCtrl* locationNode = nullptr;
        if(locs.size() > 0)
        {
            //create parent group node, move it, link to main settings node and set up plug for linking children
            locationNode = createNode(zodiac::STORY_SETTING_LOCATION_GROUP, "Locations", "Locations Group");

            zodiac::PlugHandle locationNodeInPlug = locationNode->getNodeHandle().getPlug("storyIn");
            SettingNodeOutPlug.connectPlug(locationNodeInPlug, storyLinkColor);

            loadSettingItem(locationNode, locs, zodiac::STORY_SETTING_LOCATION);
        }

        QList<SettingItem> times = m_saveAndLoadManager.GetTimes();
        NodeCtrl* timeNode = nullptr;
        if(times.size() > 0)
        {
            //create parent group node, move it, link to main settings node and set up plug for linking children
            timeNode = createNode(zodiac::STORY_SETTING_TIME_GROUP, "Times", "Times Group");

            zodiac::PlugHandle timeNodeInPlug = timeNode->getNodeHandle().getPlug("storyIn");
            SettingNodeOutPlug.connectPlug(timeNodeInPlug, storyLinkColor);

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
            ThemeNodeOutPlug.connectPlug(parentNodeInPlug, storyLinkColor);

            loadThemeItem(eventNode, events, zodiac::STORY_THEME_EVENT);
        }

        QList<EventGoal> goals = m_saveAndLoadManager.GetGoals();
        NodeCtrl* goalNode = nullptr;
        if(goals.size() > 0)
        {
            //same as events but goals
            goalNode = createNode(zodiac::STORY_THEME_GOAL_GROUP, "Goals", "Goals Group");
            zodiac::PlugHandle parentNodeInPlug = goalNode->getNodeHandle().getPlug("storyIn");
            ThemeNodeOutPlug.connectPlug(parentNodeInPlug, storyLinkColor);

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
    foreach (SettingItem sItem, items)
    {
        NodeCtrl *childNode = createStoryNode(parentNode, childType, sItem.id, sItem.description, QPoint(parentNode->getPos().x(), 150), true, true);

        if(sItem.details.size() > 0)
        {
            //add all the details for the items
            for(QList<SimpleNode>::iterator detailIt = sItem.details.begin(); detailIt != sItem.details.end(); ++detailIt)
            {
                createStoryNode(childNode, zodiac::STORY_ITEM_DETAILS, (*detailIt).id, (*detailIt).description, QPoint(childNode->getPos().x(), 150), true, true);
            }
        }
    }
}

void MainCtrl::loadThemeItem(NodeCtrl* parentNode, QList<EventGoal> items, zodiac::StoryNodeType childType)
{
    //add each item to the tree
    foreach (EventGoal tItem, items)
    {
        NodeCtrl *itemNode = createStoryNode(parentNode, childType, tItem.id, tItem.description, QPoint(parentNode->getPos().x(), 150), true, true);

        //if sub-item then load those too
        if(tItem.subItems.size() > 0)
                loadThemeItem(itemNode, tItem.subItems, childType);
    }
}

void MainCtrl::loadEpisodes(NodeCtrl *parentNode, QList<Episode> episodes)
{
    //add each item to the tree
    foreach (Episode eItem, episodes)
    {
        NodeCtrl *episodeNode;

        if(parentNode->getStoryNodeType() == zodiac::STORY_PLOT)
            episodeNode = createStoryNode(parentNode, zodiac::STORY_PLOT_EPISODE, eItem.id, eItem.description, QPoint(parentNode->getPos().x(), 150), true, true);
        else

            episodeNode = createStoryNode(parentNode, zodiac::STORY_PLOT_SUBEPISODE, eItem.id, eItem.description, QPoint(parentNode->getPos().x(), 150), true, true);

        NodeCtrl *attemptGroupNode = createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP, "Attempt", "Attempt Group", QPoint(0, 150), true, true);
        //handle attempts
        if(eItem.attempts.size() > 0 || eItem.attemptSubEpisodes.size() > 0)
        {
            foreach (SimpleNode att, eItem.attempts)
                createStoryNode(attemptGroupNode, zodiac::STORY_PLOT_EPISODE_ATTEMPT, att.id, att.description, QPoint(attemptGroupNode->getPos().x(), 150), true, true);

            if(eItem.attemptSubEpisodes.size() > 0)
                loadEpisodes(attemptGroupNode, eItem.attemptSubEpisodes);

            //get all connected plugs
            QList<zodiac::PlugHandle> connectedPlugs = attemptGroupNode->getNodeHandle().getPlug("storyOut").getConnectedPlugs();
            if(connectedPlugs.size() > 1)
                foreach (zodiac::PlugHandle cPlug, connectedPlugs)
                {
                    zodiac::NodeHandle connectedNode = cPlug.getNode(); //////THIS SHOULD BE DOING SOMETHING!!!!
                }
        }

        NodeCtrl *outcomeGroupNode = createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_OUTCOME_GROUP, "Outcome", "Outcome Group", QPoint(episodeNode->getPos().x(), 150), true, true);
        if(eItem.outcomes.size() > 0 || eItem.outcomeSubEpisodes.size() > 0)
        {
            //handle outcomes
            foreach (SimpleNode out, eItem.outcomes)
                createStoryNode(outcomeGroupNode, zodiac::STORY_PLOT_EPISODE_OUTCOME, out.id, out.description, QPoint(outcomeGroupNode->getPos().x(), 150), true, true);

            if(eItem.outcomeSubEpisodes.size() > 0)
                loadEpisodes(outcomeGroupNode, eItem.outcomeSubEpisodes); //handle outcome sub-episodes

            //get all connected plugs
            QList<zodiac::PlugHandle> connectedPlugs = outcomeGroupNode->getNodeHandle().getPlug("storyOut").getConnectedPlugs();
            if(connectedPlugs.size() > 1)
                foreach (zodiac::PlugHandle cPlug, connectedPlugs)
                {
                   zodiac::NodeHandle connectedNode = cPlug.getNode(); //////THIS SHOULD BE DOING SOMETHING!!!!
                }
        }

        //handle sub-goal
        createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_SUBGOAL, eItem.subGoal.id, eItem.subGoal.description, QPoint(episodeNode->getPos().x(), 150), true, true);
    }
}

void MainCtrl::loadResolution(NodeCtrl *resolutionNode, QList<EventGoal> events, QList<SimpleNode> states)
{
    if(events.size() > 0)
    {
        NodeCtrl *eventNode = createNode(zodiac::STORY_RESOLUTION_EVENT_GROUP, "Event", "Group of Events");
        zodiac::PlugHandle eventNodeInPlug = eventNode->getNodeHandle().getPlug("storyIn");
        resolutionNode->getNodeHandle().getPlug("storyOut").connectPlug(eventNodeInPlug, storyLinkColor);

        loadThemeItem(eventNode, events, zodiac::STORY_RESOLUTION_EVENT);
    }

   if(states.size() > 0)
    {
        NodeCtrl *stateNode = createNode(zodiac::STORY_RESOLUTION_STATE_GROUP, "State", "Group of States");
        zodiac::PlugHandle stateNodeInPlug = stateNode->getNodeHandle().getPlug("storyIn");
        resolutionNode->getNodeHandle().getPlug("storyOut").connectPlug(stateNodeInPlug, storyLinkColor);

        foreach (SimpleNode state, states)
        {
            createStoryNode(stateNode, zodiac::STORY_RESOLUTION_STATE, state.id, state.description, QPoint(stateNode->getPos().x(), 150), true, true);
        }

    }
}

void MainCtrl::saveNarrativeGraph()
{
    m_saveAndLoadManager.DeleteAllNarrativeItems(); //clear the narrative data from save and load

    //get list of all nodes
    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();

    foreach (zodiac::NodeHandle node, nodes)
        if(node.getType() == zodiac::NODE_NARRATIVE && node.getPlug("reqOut").getConnectedPlugs().empty())
            saveNarrativeNode(node);

    m_saveAndLoadManager.SaveNarrativeToFile(qobject_cast<QWidget*>(parent()));

}

void MainCtrl::saveNarrativeNode(zodiac::NodeHandle &node)
{
    NarNode *newNarrativeNode = nullptr;

    if(node.getType() == zodiac::NODE_NARRATIVE && !(node.getName() == "SEQ" || node.getName() == "INV"))
        newNarrativeNode = m_saveAndLoadManager.addNarrativeNode(node.getName(), node.getDescription(), node.getFileName());

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

        foreach (zodiac::PlugHandle rPlug, requirementPlugs)
        {
            zodiac::NodeHandle reqNode = rPlug.getNode();

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

        foreach (zodiac::PlugHandle rPlug, requirementPlugs)
        {
            zodiac::NodeHandle reqNode = rPlug.getNode();

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

        foreach (zodiac::PlugHandle sPlug, storyPlugs)
        {
            zodiac::NodeHandle storyNode = sPlug.getNode();

            m_saveAndLoadManager.addStoryTagToNarrativeNode(narNode, storyNode.getStoryNodePrefix() + storyNode.getName());
        }
    }
}

void MainCtrl::loadNarrativeGraph()
{
    checkGraphLoaded(zodiac::NODE_STORY);   //story graph should be loaded first

    m_saveAndLoadManager.DeleteAllNarrativeItems(); //reset the holder

    if(m_saveAndLoadManager.LoadNarrativeFromFile(qobject_cast<QWidget*>(parent())))
    {
        QList<zodiac::NodeHandle> currentNodes =  m_scene.getNodes();
        QList<zodiac::NodeHandle> currentNarSceneNodes;

        foreach (zodiac::NodeHandle cNode, currentNodes)
        {
            //also save all current narrative nodes to a separate list in case two parts of the same narrative are loaded separately, for requirements
            if(cNode.getType() == zodiac::NODE_NARRATIVE)
                currentNarSceneNodes.push_back(cNode);
        }

        QList<NarNode> narrativeNodes = m_saveAndLoadManager.GetNarrativeNodes();
        QList<NodeCtrl*> newNarSceneNodes;

        foreach (NarNode nNode, narrativeNodes)
        {
            NodeCtrl* newNarNode = createNode(zodiac::STORY_NONE, nNode.id, nNode.comments);
            newNarNode->setFileName(nNode.fileName);

            loadNarrativeCommands(nNode, newNarNode);

            loadStoryTags(newNarNode, nNode.storyTags);

            newNarSceneNodes.push_back(newNarNode);
        }

        //loop again for the requirements (necessary in case nodes aren't loaded in chronological order)
        foreach (NarNode nNode, narrativeNodes)
        {
            if(nNode.requirements.type != REQ_NONE)
            {
                //qDebug() << (*narIt).id << " requirements";

                NodeCtrl* newNarNode = nullptr;

                foreach (NodeCtrl* newNNode, newNarSceneNodes)
                {
                    if(nNode.id == newNNode->getName())
                    {
                        newNarNode = newNNode;
                    }
                }

                if(!newNarNode)
                {
                    qDebug() << "Warning: node -"+ nNode.id +"- not found in recently loaded list";
                    continue;
                }

                zodiac::PlugHandle reqOutPlug;

                if(newNarNode->getNodeHandle().getPlug("reqOut").isValid())
                    reqOutPlug = newNarNode->getNodeHandle().getPlug("reqOut");
                else
                    reqOutPlug = newNarNode->addOutgoingPlug("reqOut");

                loadRequirements(nNode.requirements, reqOutPlug, newNarSceneNodes, currentNarSceneNodes);
            }
        }

        spaceOutFullNarrative();

        m_propertyEditor->UpdateLinkerValues(m_scene.getNodes());
    }
}

void MainCtrl::loadNarrativeCommands(NarNode &loadedNode, NodeCtrl* sceneNode)
{
    foreach (NarCommand oUCmd, loadedNode.onUnlockCommands)
    {
        QUuid cmdKey = QUuid::createUuid();
        sceneNode->addOnUnlockCommand(cmdKey, oUCmd.command, oUCmd.description);

        foreach (SimpleNode cmdParam, oUCmd.params)
            sceneNode->addParameterToOnUnlockCommand(cmdKey, cmdParam.id, cmdParam.description);
    }

    foreach (NarCommand oFCmd, loadedNode.onFailCommands)
    {
        QUuid cmdKey = QUuid::createUuid();
        sceneNode->addOnFailCommand(cmdKey, oFCmd.command, oFCmd.description);

        foreach (SimpleNode cmdParam, oFCmd.params)
            sceneNode->addParameterToOnFailCommand(cmdKey, cmdParam.id, cmdParam.description);
    }

    foreach (NarCommand oUdCmd, loadedNode.onUnlockedCommands)
    {
        QUuid cmdKey = QUuid::createUuid();
        sceneNode->addOnUnlockedCommand(cmdKey, oUdCmd.command, oUdCmd.description);

        foreach (SimpleNode cmdParam, oUdCmd.params)
            sceneNode->addParameterToOnUnlockedCommand(cmdKey, cmdParam.id, cmdParam.description);
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
            foreach (NodeCtrl* sNode, sceneNodes)
                if(sNode->getName() == requirements.id)
                {
                    newRequirementNode = sNode;

                    zodiac::PlugHandle nodeReqInPlug;

                    if(sNode->getNodeHandle().getPlug("reqIn").isValid())
                        nodeReqInPlug = sNode->getNodeHandle().getPlug("reqIn");
                    else
                        nodeReqInPlug = sNode->addIncomingPlug("reqIn");

                    parentReqOutPlug.connectPlug(nodeReqInPlug, narrativeLinkColor);  //link plugs
                    found  = true;
                    break;
                }

            if(!found)
            {
                qDebug() << "Warning. Node:" << requirements.id << "not found in loaded list, checking scene nodes.";

                foreach (zodiac::NodeHandle cSNode, currentNarSceneNodes)
                    if(cSNode.getName() == requirements.id)
                    {
                        newRequirementNode = new NodeCtrl(this, cSNode);

                        zodiac::PlugHandle nodeReqInPlug;

                        if(cSNode.getPlug("reqIn").isValid())
                            nodeReqInPlug = cSNode.getPlug("reqIn");
                        else
                            nodeReqInPlug = cSNode.createIncomingPlug("reqIn");

                        parentReqOutPlug.connectPlug(nodeReqInPlug, narrativeLinkColor);  //link plugs
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

        parentReqOutPlug.connectPlug(reqInPlug, narrativeLinkColor);

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
            foreach (NodeCtrl* sNode, sceneNodes)
                if(sNode->getName() == requirements.id)
                {
                    zodiac::PlugHandle nodeReqInPlug;

                    if(sNode->getNodeHandle().getPlug("reqIn").isValid())
                        nodeReqInPlug = sNode->getNodeHandle().getPlug("reqIn");
                    else
                        nodeReqInPlug = sNode->addIncomingPlug("reqIn");
                    reqOutPlug.connectPlug(nodeReqInPlug, narrativeLinkColor);  //link plugs
                    found  = true;
                    break;
                }

            if(!found)
            {
                qDebug() << "Warning. Node:" << requirements.id << "not found in loaded list, checking scene nodes.";

                foreach (zodiac::NodeHandle sNode, currentNarSceneNodes)
                    if(sNode.getName() == requirements.id)
                    {
                        zodiac::PlugHandle nodeReqInPlug;

                        if(sNode.getPlug("reqIn").isValid())
                            nodeReqInPlug = sNode.getPlug("reqIn");
                        else
                            nodeReqInPlug = sNode.createIncomingPlug("reqIn");
                        parentReqOutPlug.connectPlug(nodeReqInPlug, narrativeLinkColor);  //link plugs
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
            foreach (NarRequirements reqChild, requirements.children)
            {
                if(!reqOutPlug.isValid())
                {
                    if(newRequirementNode->getNodeHandle().getPlug("reqOut").isValid())
                        reqOutPlug = newRequirementNode->getNodeHandle().getPlug("reqOut");
                    else
                        reqOutPlug = newRequirementNode->addOutgoingPlug("reqOut"); //make the out plug if it doesn't exist
                }

                loadRequirements(reqChild, reqOutPlug, sceneNodes, currentNarSceneNodes);
            }
        }
    }
}

void MainCtrl::loadStoryTags(NodeCtrl* narrativeNode, QList<QString> storyTags)
{
    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();

    foreach (QString tag, storyTags)
    {
        foreach (zodiac::NodeHandle storyNode, nodes)
        {
            if(storyNode.getType() == zodiac::NODE_STORY)
            {
                if(storyNode.getStoryNodePrefix() + storyNode.getName() == tag)
                {
                    narrativeNode->getNodeHandle().getPlug("storyOut").connectPlug(storyNode.getPlug("narrativeIn"), storyNarrativeLinkColor);

                    storyNode.setLabelBackgroundColor(QColor(2, 202, 0));
                }
            }
        }
    }
}

void MainCtrl::spaceOutFullNarrative()
{
    QVector<QString> oldFileNames = m_narrativeSorter->getOrderedList();
    QVector<QString> newFileNames = m_saveAndLoadManager.getFileNames();

    foreach (QString fileName, newFileNames)
    {
        if(oldFileNames.contains(fileName))
            newFileNames.removeOne(fileName);
    }

    if(newFileNames.size() == 0)
        spaceOutNarrative(oldFileNames);
    else
        if(oldFileNames.size() + newFileNames.size() <= 1 || (oldFileNames.size() >= 1 && std::is_permutation(oldFileNames.begin(), oldFileNames.end(), newFileNames.begin())))
            spaceOutNarrative(newFileNames);
        else
            m_narrativeSorter->showWindow(newFileNames);
}

void MainCtrl::spaceOutNarrative(QVector<QString> fileNames)
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
        foreach (zodiac::NodeHandle storyNode, nodeList)
        {
            if(storyNode.getType() == zodiac::NODE_STORY)
            {
                QPointF nodePos = storyNode.getPos();

                if(nodePos.x() < xPos)
                    xPos = nodePos.x();

                if(nodePos.y() > yPos)
                    yPos = nodePos.y() + 150; //add 150 to ensure the nodes are underneath
            }
        }

    float oldYPos = yPos;

    //space out nodes
    foreach (QString fileName, fileNames)
        foreach (zodiac::NodeHandle narNode, nodeList)
        {
            if((narNode.getType() == zodiac::NODE_NARRATIVE) && narNode.getFileName() == fileName && narNode.getPlug("reqOut").connectionCount() == 0)
            {
                narNode.setPos(xPos, yPos);
                spaceOutNarrativeChildren(new NodeCtrl(this, narNode), yPos, xPos);

                yPos = oldYPos;
                xPos += 150;
            }
        }

    foreach (zodiac::NodeHandle narNode, nodeList)
    {
        if((narNode.getType() == zodiac::NODE_NARRATIVE) && narNode.getPlug("reqOut").connectionCount() > 1)
        {
            QList<zodiac::PlugHandle> connectedPlugs = narNode.getPlug("reqOut").getConnectedPlugs();
            float averageY = 0;

            foreach (zodiac::PlugHandle plug, connectedPlugs)
            {
                averageY += plug.getNode().getPos().y();
            }

            narNode.setPos(narNode.getPos().x(), averageY/connectedPlugs.size(), false, true);
        }
    }
}

void MainCtrl::spaceOutNarrativeChildren(NodeCtrl* sceneNode, float &maxY, float &maxX)
{
    zodiac::PlugHandle inPlug = sceneNode->getNodeHandle().getPlug("reqIn");

    if(inPlug.isValid() && inPlug.isIncoming())
    {
        QList<zodiac::PlugHandle> connectedPlugs = inPlug.getConnectedPlugs();

        if(connectedPlugs.size() > 0)
        {
            float averageY = 0;
            float y = maxY;

            foreach (zodiac::PlugHandle plug, connectedPlugs)
            {
                NodeCtrl* childNode = new NodeCtrl(this, plug.getNode());

                childNode->setPos(sceneNode->getPos().x() + 150, y);
                spaceOutNarrativeChildren(childNode, y, maxX);

                averageY += childNode->getPos().y();

                if(y > maxY)
                    maxY = y;

                if(childNode->getPos().x() > maxX)
                    maxX = childNode->getPos().x();

                y += 150;
            }

            sceneNode->setPos(sceneNode->getPos().x(), averageY/connectedPlugs.size());
        }
    }
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
    foreach(zodiac::NodeHandle node, nodes)
    {
        if(node.getStoryNodeType() == zodiac::STORY_NAME)
            startingNode = node;
        else
            if(node.getStoryNodeType() == zodiac::STORY_SETTING)
                settingNode = node;
            else
                if(node.getStoryNodeType() == zodiac::STORY_THEME)
                    themeNode = node;
                else
                    if(node.getStoryNodeType() == zodiac::STORY_PLOT)
                        plotNode = node;
                    else
                        if(node.getStoryNodeType() == zodiac::STORY_RESOLUTION)
                            resolutionNode = node;
    }

    if(!startingNode.isValid())
        qDebug() << "no starting node";
    if(!settingNode.isValid())
        qDebug() << "no setting node";
    if(!themeNode.isValid())
        qDebug() << "no theme node";
    if(!plotNode.isValid())
        qDebug() << "no plot node";
    if(!resolutionNode.isValid())
        qDebug() << "no resolution node";

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
            foreach (zodiac::PlugHandle plug, connectedPlugs)
                childNodes.push_back(plug.getNode());

            float nodeXPos = node.getPos().x();
            float nodeYPos = node.getPos().y() + 150;
            float childPos = 0.0f;
            foreach (zodiac::NodeHandle cNode, childNodes)
            {
                float childMaxX = spaceOutChildNodes(cNode, nodeXPos, nodeYPos);

                if(childMaxX > maxX)
                    maxX = childMaxX;

                childPos += cNode.getPos().x();
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
    if(nodeList.isEmpty() && inverseNodeList.isEmpty()) //ignore if both lists empty
        return;

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

                node.getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"), narrativeLinkColor);

                foreach(zodiac::NodeHandle connectedNode, connectedNodes)   //disconnect from previous node and connect to new sequence node
                {
                    connectedNode.getPlug("reqIn").disconnectPlug(node.getPlug("reqOut"));
                    connectedNode.getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"), narrativeLinkColor);
                }
            }
        }

        foreach(zodiac::NodeHandle node, nodeList)
        {
            nodePtr->getPlug("reqIn").connectPlug(node.getPlug("reqOut"), narrativeLinkColor);
            if(node.getFileName() == "")
                node.setFileName(node.getFileName());
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

                node.getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"), narrativeLinkColor);

                foreach(zodiac::NodeHandle connectedNode, connectedNodes)
                {//disconnect from previous node and connect to new sequence node
                    connectedNode.getPlug("reqIn").disconnectPlug(oldNodePtr->getPlug("reqOut"));
                    connectedNode.getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"), narrativeLinkColor);
                }
            }
        }

        oldNodePtr = nodePtr;

        if(nodePtr->getName() != "INV")
        {
            nodePtr = &createNode(zodiac::STORY_NONE, "INV", "")->getNodeHandle();
            nodePtr->setIdleColor(QColor(255, 204, 0));
            nodePtr->setSelectedColor(QColor(255, 153, 0));

            oldNodePtr->getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"), narrativeLinkColor);
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

            oldNodePtr->getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"), narrativeLinkColor);

            foreach(zodiac::NodeHandle connectedNode, connectedNodes)
            {//disconnect from previous node and connect to new sequence node
                connectedNode.getPlug("reqIn").disconnectPlug(oldNodePtr->getPlug("reqOut"));
                connectedNode.getPlug("reqIn").connectPlug(nodePtr->getPlug("reqOut"), narrativeLinkColor);
            }
        }

        for(QList<zodiac::NodeHandle>::iterator nodeIt = inverseNodeList.begin(); nodeIt != inverseNodeList.end(); ++ nodeIt)
        {
            nodePtr->getPlug("reqIn").connectPlug((*nodeIt).getPlug("reqOut"), narrativeLinkColor);
            if((*nodeIt).getFileName() == "")
                (*nodeIt).setFileName(node.getFileName());
        }
    }

    spaceOutFullNarrative();

    m_propertyEditor->UpdateLinkerValues(m_scene.getNodes());
}

void MainCtrl::linkStoryNodes(zodiac::NodeHandle &node, QList<zodiac::NodeHandle> &nodeList)
{
    if(nodeList.size() > 0)
    {
        foreach(zodiac::NodeHandle nodeToLink, nodeList)
        {
            //make plugs and connect
            if(!node.getPlug("storyOut").isValid())
                node.createOutgoingPlug("storyOut");
            if(!nodeToLink.getPlug("narrativeIn").isValid())
                nodeToLink.createIncomingPlug("narrativeIn");

            node.getPlug("storyOut").connectPlug(nodeToLink.getPlug("narrativeIn"), storyNarrativeLinkColor); //connect nodes

            nodeToLink.setLabelBackgroundColor(QColor(2, 202, 0));
        }

        m_propertyEditor->UpdateLinkerValues(m_scene.getNodes());
    }
}

void MainCtrl::lockAllNodes()
{
    QList<zodiac::NodeHandle> currentNodes =  m_scene.getNodes();

    QList<zodiac::NodeHandle> invNodes;

    foreach(zodiac::NodeHandle cNode, currentNodes)
    {
        if(cNode.isNodeDecorator())   //not the INV and SEQ nodes
        {
            if(cNode.getName() == "INV")
                invNodes.push_back((cNode));

            continue;
        }

        if(cNode.getType() == zodiac::NODE_NARRATIVE)
        {
            if(cNode.getPlug("reqOut").connectionCount() == 0)    //unlockable if no requirements
            {
                cNode.setLockedStatus(zodiac::UNLOCKABLE);
                cNode.setIdleColor(QColor("#3333cc"));
                cNode.setIdleColor(QColor("#4949cc"));
            }
            else
            {
                cNode.setLockedStatus(zodiac::LOCKED);
                cNode.setIdleColor(QColor("#ff1800"));
                cNode.setSelectedColor(QColor("#ff331e"));
            }
        }
        else
        {
            cNode.setIdleColor(QColor("#ff1800"));
            cNode.setSelectedColor(QColor("#ff331e"));
        }
    }

    foreach(zodiac::NodeHandle iNode, invNodes)
    {
        if(iNode.getPlug("reqOut").isValid())
        {
            QList<zodiac::PlugHandle> InvInPlugs = iNode.getPlug("reqOut").getConnectedPlugs();
            //this should only return one node
            if(InvInPlugs.size() > 1)
                qDebug() << "Error: more than one node without a sequencer";
            foreach(zodiac::PlugHandle iInPlug, InvInPlugs)
            {
                if(iInPlug.getNode().getName() == "SEQ")
                {
                    //check reqOut plug to see if all nodes are unlocked
                    if(iInPlug.getNode().getPlug("reqOut").isValid())
                    {
                        QList<zodiac::NodeHandle> seqInNodes;
                        QList<zodiac::PlugHandle> seqInPlugs = iInPlug.getNode().getPlug("reqOut").getConnectedPlugs();
                        foreach(zodiac::PlugHandle sInPlug, seqInPlugs)
                        {
                            seqInNodes.push_back(sInPlug.getNode());
                        }

                        //if true then lock node, otherwise it's still unlockable
                        if(areAllNodesUnlocked(seqInNodes))
                        {
                            iNode.setLockedStatus(zodiac::LOCKED);
                            iNode.setIdleColor(QColor("#ff1800"));
                            iNode.setSelectedColor(QColor("#ff331e"));
                        }
                        else
                        {
                            iNode.setLockedStatus(zodiac::UNLOCKABLE);
                            iNode.setIdleColor(QColor("#3333cc"));
                            iNode.setIdleColor(QColor("#4949cc"));
                        }
                    }
                }
                else
                {
                    if(iInPlug.getNode().getLockedStatus())
                    {
                        iNode.setLockedStatus(zodiac::UNLOCKABLE);
                        iNode.setIdleColor(QColor("#3333cc"));    //node locked, unlockable
                        iNode.setIdleColor(QColor("#4949cc"));
                    }
                    else
                    {
                        iNode.setLockedStatus(zodiac::LOCKED);
                        iNode.setIdleColor(QColor("#ff1800"));        //node unlocked, locked
                        iNode.setSelectedColor(QColor("#ff331e"));
                    }
                }
            }
        }
    }

}

void MainCtrl::resetAllNodes()
{
    QList<zodiac::NodeHandle> currentNodes =  m_scene.getNodes();

    foreach(zodiac::NodeHandle cNode, currentNodes)
    {
        if(cNode.getType() == zodiac::NODE_NARRATIVE)
        {
            cNode.setIdleColor(QColor("#4b77a7"));
            cNode.setSelectedColor(QColor("#62abfa"));
            cNode.setLockedStatus(zodiac::LOCKED);
        }
        else
            if(cNode.getType() == zodiac::NODE_STORY)
            {
                cNode.setIdleColor(QColor("#4b77a7"));
                cNode.setSelectedColor(QColor("#62abfa"));
            }
    }
}

void MainCtrl::unlockNode(QString nodeName)
{
    QList<zodiac::NodeHandle> currentNodes =  m_scene.getNodes();
    bool found = false;

    foreach(zodiac::NodeHandle cNode, currentNodes)
    {
        if(cNode.getType() == zodiac::NODE_NARRATIVE && cNode.getName() == nodeName)
        {
            found = true;

            //unlocked change colour of node to green to show unlocked
            cNode.setLockedStatus(zodiac::UNLOCKED);
            cNode.setIdleColor(QColor("#00cc00"));
            cNode.setSelectedColor(QColor("#5bff5b"));

            //change colour of story nodes to green as now unlocked
            if(cNode.getPlug("storyOut").isValid())
            {
                QList<zodiac::PlugHandle> storyOutPlugs = cNode.getPlug("storyOut").getConnectedPlugs();

                foreach(zodiac::PlugHandle outPlug, storyOutPlugs)
                {
                    outPlug.getNode().setIdleColor(QColor("#00cc00"));
                    cNode.setSelectedColor(QColor("#5bff5b"));
                }
            }

            //check if other nodes are unlockable, turn them blue
            if(cNode.getPlug("reqIn").isValid())
            {
                QList<zodiac::NodeHandle> reqNodes;
                QList<zodiac::PlugHandle> reqPlugs = cNode.getPlug("reqIn").getConnectedPlugs();

                //get all nodes which require node to be unlocked first
                foreach(zodiac::PlugHandle reqPlug, reqPlugs)
                    reqNodes.push_back(reqPlug.getNode());

                showUnlockableNodes(reqNodes);
            }

            break;
        }
    }

    /*if(!found)
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","Node unlocked which does not exist in the narrative graph.\nPlease ensure that the correct and complete graph is loaded");
        messageBox.setFixedSize(500,200);
    }*/
}

void MainCtrl::showUnlockableNodes(QList<zodiac::NodeHandle> &nodes)
{
    foreach(zodiac::NodeHandle rNode, nodes)
    {
        if(rNode.getName() == "SEQ")
        {
            //check reqOut plug to see if all nodes are unlocked
            if(rNode.getPlug("reqOut").isValid())
            {
                QList<zodiac::NodeHandle> seqInNodes;
                QList<zodiac::PlugHandle> seqInPlugs = rNode.getPlug("reqOut").getConnectedPlugs();

                foreach(zodiac::PlugHandle sIPlug, seqInPlugs)
                    seqInNodes.push_back(sIPlug.getNode());

                //if true then use reqIn to change to blue
                if(areAllNodesUnlocked(seqInNodes))
                {
                    QList<zodiac::NodeHandle> seqOutNodes;
                    QList<zodiac::PlugHandle> seqOutPlugs = rNode.getPlug("reqIn").getConnectedPlugs();

                    foreach(zodiac::PlugHandle sOPlug, seqOutPlugs)
                        seqOutNodes.push_back(sOPlug.getNode());

                    showUnlockableNodes(seqOutNodes);
                }
            }
        }
        else
            if(rNode.getName() == "INV")
            {
                //node should be now locked
                //check for possible seq node
                if((rNode).getPlug("reqOut").isValid())
                {
                    QList<zodiac::PlugHandle> InvInPlugs = rNode.getPlug("reqOut").getConnectedPlugs();
                    //this should only return one node
                    if(InvInPlugs.size() > 1)
                        qDebug() << "Error: more than one node without a sequencer";

                    foreach(zodiac::PlugHandle iIPlug, InvInPlugs)
                    {
                        if(iIPlug.getNode().getName() == "SEQ")
                        {
                            //check reqOut plug to see if all nodes are unlocked
                            if(iIPlug.getNode().getPlug("reqOut").isValid())
                            {
                                QList<zodiac::NodeHandle> seqInNodes;
                                QList<zodiac::PlugHandle> seqInPlugs = iIPlug.getNode().getPlug("reqOut").getConnectedPlugs();
                                foreach(zodiac::PlugHandle sIPlug, seqInPlugs)
                                {
                                    seqInNodes.push_back(sIPlug.getNode());
                                }

                                //if true then lock node, otherwise it's still unlockable
                                if(areAllNodesUnlocked(seqInNodes))
                                {
                                    rNode.setLockedStatus(zodiac::LOCKED);
                                    rNode.setIdleColor(QColor("#ff1800"));
                                    rNode.setSelectedColor(QColor("#ff331e"));
                                }
                                else
                                {
                                    rNode.setLockedStatus(zodiac::UNLOCKABLE);
                                    rNode.setIdleColor(QColor("#3333cc"));    //unlockable
                                    rNode.setIdleColor(QColor("#4949cc"));
                                }
                            }
                        }
                        else
                        {
                            rNode.setLockedStatus(zodiac::LOCKED);
                            rNode.setIdleColor(QColor("#ff1800"));    //lock it
                            rNode.setSelectedColor(QColor("#ff331e"));
                        }
                    }
                }

            }
            else
            {
                rNode.setLockedStatus(zodiac::UNLOCKABLE);
                rNode.setIdleColor(QColor("#3333cc"));
                rNode.setIdleColor(QColor("#4949cc"));
            }
    }
}

bool MainCtrl::areAllNodesUnlocked(QList<zodiac::NodeHandle> &nodes)
{
    foreach(zodiac::NodeHandle rNode, nodes)
    {
        if(rNode.getName() == "SEQ")
        {
            //check reqIn plug to see if all nodes are unlocked
            if(rNode.getPlug("reqIn").isValid())
            {
                QList<zodiac::NodeHandle> seqInNodes;
                QList<zodiac::PlugHandle> seqInPlugs = rNode.getPlug("reqIn").getConnectedPlugs();

                foreach(zodiac::PlugHandle sIPlug, seqInPlugs)
                    seqInNodes.push_back(sIPlug.getNode());

                if(!areAllNodesUnlocked(seqInNodes))
                    return false;
            }
        }
        else
            if(rNode.getName() == "INV")
            {
                if((rNode).getPlug("reqOut").isValid())
                {
                    QList<zodiac::NodeHandle> invNodes;
                    QList<zodiac::PlugHandle> invPlugs = rNode.getPlug("reqOut").getConnectedPlugs();

                    //get all nodes which require node to be unlocked first
                    foreach(zodiac::PlugHandle iPlug, invPlugs)
                        invNodes.push_back(iPlug.getNode());

                    foreach(zodiac::NodeHandle iNode, invNodes)
                    {
                        if(iNode.getName() == "SEQ")
                        {
                            QList<zodiac::NodeHandle> seqNodes;
                            QList<zodiac::PlugHandle> seqPlugs = iNode.getPlug("reqOut").getConnectedPlugs();

                            //get all nodes which require node to be unlocked first
                            foreach(zodiac::PlugHandle sPlug, seqPlugs)
                                seqNodes.push_back(sPlug.getNode());

                            if(areAllNodesUnlocked(seqNodes))   //inverse so returning true means not unlocked
                                return false;
                        }
                        else
                            if(!rNode.getLockedStatus())  //if unlocked then fail
                                return false;
                    }
                }
            }
            else
                {
                     if(rNode.getLockedStatus())
                         return false;
                }
    }

    return true;
}

void MainCtrl::checkGraphLoaded(zodiac::NodeType type)
{
    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();
    bool loadGraph = true;

    foreach(zodiac::NodeHandle node, nodes)
    {
        if(node.getType() == type)
        {
            loadGraph = false;
            break;  //some kind of graph exists, no need to continue
        }
    }

    if(loadGraph)
    {
        QMessageBox msgBox;

        if(type == zodiac::NODE_NARRATIVE)
        {
            msgBox.setWindowTitle("No Narrative Graph Loaded");
            msgBox.setText("Do you want to load a narrative graph before continuing?");
        }
        else
            if(type == zodiac::NODE_STORY)
            {
                msgBox.setWindowTitle("No Story Graph Loaded");
                msgBox.setText("Do you want to load a story graph before continuing");
            }

        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if(msgBox.exec() == QMessageBox::Yes)
        {
            if(type == zodiac::NODE_NARRATIVE)
                loadNarrativeGraph();
            else
                if(type == zodiac::NODE_STORY)
                    loadStoryGraph();
        }
    }
}

void MainCtrl::updateAnalyticsProperties()
{
    m_propertyEditor->UpdateLinkerValues(m_scene.getNodes());
}

void MainCtrl::changeReqVisibility(bool show)
{
    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();

    foreach (zodiac::NodeHandle node, nodes)
    {
        if(node.getType() == zodiac::NODE_NARRATIVE)
        {
            QSet<zodiac::PlugEdge *> outEdges = node.getPlug("reqOut").getEdges();

            foreach (zodiac::PlugEdge *outEdge, outEdges)
            {
                 outEdge->setVisible(show);
            }

            QSet<zodiac::PlugEdge *> inEdges = node.getPlug("reqIn").getEdges();

            foreach (zodiac::PlugEdge *inEdge, inEdges)
            {
                 inEdge->setVisible(show);
            }
        }
    }
}

void MainCtrl::changeStoryVisibility(bool show, zodiac::NodeType type)
{
    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();

    foreach (zodiac::NodeHandle node, nodes)
    {
        if(node.getType() == type)
        {
            QSet<zodiac::PlugEdge *> outEdges = node.getPlug("storyOut").getEdges();

            foreach (zodiac::PlugEdge *outEdge, outEdges)
            {
                 outEdge->setVisible(show);
            }
        }
    }
}
