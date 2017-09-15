#include "mainctrl.h"

#include <QDebug>
#include <cstdlib>

#include "nodectrl.h"
#include "propertyeditor.h"
#include "zodiacgraph/nodehandle.h"

QString MainCtrl::s_defaultName = "Node ";

MainCtrl::MainCtrl(QObject *parent, zodiac::Scene* scene, PropertyEditor* propertyEditor, QUndoStack *undoStack)
    : QObject(parent)
    , m_scene(zodiac::SceneHandle(scene))
    , m_propertyEditor(propertyEditor)
    , m_nodes(QHash<zodiac::NodeHandle, NodeCtrl*>())
    , m_nodeIndex(1)            // name suffixes start at 1
    , m_pUndoStack(undoStack)
{
    m_saveAndLoadManager.LoadNarrativeParamsAndCommands(qobject_cast<QWidget*>(parent));

    m_propertyEditor->setMainCtrl(this);
    m_propertyEditor->setCommandList(m_saveAndLoadManager.GetCommandListPointer());
    m_propertyEditor->setUndoStack(undoStack);

    connect(&m_scene, SIGNAL(selectionChanged(QList<zodiac::NodeHandle>)),
            this, SLOT(selectionChanged(QList<zodiac::NodeHandle>)));
}

NodeCtrl* MainCtrl::createNode(zodiac::StoryNodeType storyType, const QString& name)
{
    // the newly created Node is the only selected one to avoid confusion
    m_scene.deselectAll();

    // use the given name or construct a default one
    QString nodeName = name;
    if(nodeName.isEmpty()){
        nodeName = s_defaultName + QString::number(m_nodeIndex++);
    }

    // create the node
    NodeCtrl* nodeCtrl = new NodeCtrl(this, m_scene.createNode(nodeName, storyType));
    m_nodes.insert(nodeCtrl->getNodeHandle(), nodeCtrl);

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
        // nodes with connections cannot be deleted
        return false;
    }

    // disconnect and delete the node
    node->disconnect();
    zodiac::NodeHandle handle = node->getNodeHandle();
    m_nodes.remove(handle);
    bool result = handle.remove();
    Q_ASSERT(result);
    return result;
}

void MainCtrl::printZodiacScene()
{
    QList<zodiac::NodeHandle> allNodes = m_nodes.keys();
    for(zodiac::NodeHandle node : allNodes){
        int number = node.getName().right(2).trimmed().toInt();
        QString nodeCtrl = "nodeCtrl" + QString::number(number);
        QPointF pos = node.getPos();

        qDebug() << "NodeCtrl* nodeCtrl" + QString::number(number) + " = mainCtrl->createNode(\"" + node.getName() + "\");";
        qDebug() << nodeCtrl + "->getNodeHandle().setPos(" + QString::number(pos.x()) + ", " + QString::number(pos.y()) + ");";

        for(zodiac::PlugHandle plug : node.getPlugs()){
            if(plug.isIncoming()){
                qDebug() << nodeCtrl + "->addIncomingPlug(\"" + plug.getName() + "\");";
            } else {
                qDebug() << nodeCtrl + "->addOutgoingPlug(\"" + plug.getName() + "\");";
            }
        }

        qDebug() << ""; // newline
    }

    for(zodiac::NodeHandle node : allNodes){
        int number = node.getName().right(2).trimmed().toInt();
        QString nodeCtrl = "nodeCtrl" + QString::number(number);
        for(zodiac::PlugHandle plug : node.getPlugs()){
            if(plug.isIncoming()) continue;
            for(zodiac::PlugHandle otherPlug : plug.getConnectedPlugs()){
                int otherNumber = otherPlug.getNode().getName().right(2).trimmed().toInt();
                QString otherNodeCtrl = "nodeCtrl" + QString::number(otherNumber);
                qDebug() << nodeCtrl + "->getNodeHandle().getPlug(\"" + plug.getName() + "\").connectPlug(" + otherNodeCtrl + "->getNodeHandle().getPlug(\"" + otherPlug.getName() + "\"));";
            }
        }
    }
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
}

NodeCtrl* MainCtrl::createStoryNode(NodeCtrl *parent, zodiac::StoryNodeType type, QString name, QPoint &relativePos)
{
    NodeCtrl* child = createNode(type, name);

    //do something with the position
    child->setPos(parent->getPos().x() + relativePos.x(), parent->getPos().y() + relativePos.y());

    //if the out plug doesn't exist, make one
    zodiac::PlugHandle ParentNodeOutPlug = parent->getNodeHandle().getPlug("out");
    if(!ParentNodeOutPlug.isValid())
        ParentNodeOutPlug = parent->getNodeHandle().createOutgoingPlug("out");

    zodiac::PlugHandle childNodeInPlug = child->getNodeHandle().createIncomingPlug("in");

    ParentNodeOutPlug.connectPlug(childNodeInPlug);

    return child;
}

void MainCtrl::createStoryGraph(QString storyName)
{
    NodeCtrl* nameNode = createNode(zodiac::STORY_NAME, storyName); //create story name

    NodeCtrl* settingNode = createNode(zodiac::STORY_SETTING, "Setting"); //create setting
    NodeCtrl* themeNode = createNode(zodiac::STORY_THEME, "Theme"); //create theme
    NodeCtrl* plotNode = createNode(zodiac::STORY_PLOT, "Plot"); //create plot
    NodeCtrl* resolutionNode = createNode(zodiac::STORY_RESOLUTION, "Resolution"); //create resolution

    //distance between each object should be 100, looks clean

    nameNode->getNodeHandle().setPos(0, -100);
    settingNode->getNodeHandle().setPos(-150, 100);
    themeNode->getNodeHandle().setPos(-50, 100);
    plotNode->getNodeHandle().setPos(50, 100);
    resolutionNode->getNodeHandle().setPos(150, 100);

    zodiac::PlugHandle nameNodeOutPlug = nameNode->getNodeHandle().createOutgoingPlug("nameOut");

    zodiac::PlugHandle settingNodeInPlug = settingNode->getNodeHandle().createIncomingPlug("settingIn");
    nameNodeOutPlug.connectPlug(settingNodeInPlug);
    zodiac::PlugHandle themeNodeInPlug = themeNode->getNodeHandle().createIncomingPlug("themeIn");
    nameNodeOutPlug.connectPlug(themeNodeInPlug);
    zodiac::PlugHandle plotNodeInPlug = plotNode->getNodeHandle().createIncomingPlug("plotIn");
    nameNodeOutPlug.connectPlug(plotNodeInPlug);
    zodiac::PlugHandle resolutionNodeInPlug = resolutionNode->getNodeHandle().createIncomingPlug("resolutionIn");
    nameNodeOutPlug.connectPlug(resolutionNodeInPlug);
}

void MainCtrl::saveStoryGraph()
{
    //get list of all story nodes
    QList<zodiac::NodeHandle> nodes = m_scene.getNodes();
    zodiac::NodeHandle *mainStoryNode;

    //iterate through the list to find the main story node
    for(QList<zodiac::NodeHandle>::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        if((*it).getType() == zodiac::NODE_STORY && (*it).getStoryNodeType() == zodiac::STORY_NAME)
        {
            mainStoryNode = &(*it); //get a pointer to the handle of the main node as a starting point
            break;
        }
    }

    //store story name

    //get settings node
    //get characters node - store, then iterate through details nodes and store these
    //get locations node - store, then iterate through details nodes and store these
    //get times node - store, then iterate through details nodes and store these

    //get theme node
    //get events node - find each seperate event and sub-events, store these
    //get goals node - find each seperate goal and sub-goals, store these

    //get plot node
    //get each episode node - store, store subgoal, iterate through attempts, store these and sub episodes, same with outcomes

    //get resolution node
    //get events node - iterate though events and store
    //get states node = iterate through states and store
}

void MainCtrl::loadStoryGraph()
{
    qDebug() << "load";
    if(m_saveAndLoadManager.LoadStoryFromFile(qobject_cast<QWidget*>(parent())))
    {
        //create the story graph and grab the nodes
        createStoryGraph(m_saveAndLoadManager.GetStoryName());
        QList<zodiac::NodeHandle> nodes = m_scene.getNodes();

        //find the main nodes
        zodiac::NodeHandle *settingNode;
        zodiac::NodeHandle *themeNode;
        zodiac::NodeHandle *plotNode;
        zodiac::NodeHandle *resolutionNode;
        //iterate through the list to find the nodes
        for(QList<zodiac::NodeHandle>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            if((*it).getType() == zodiac::NODE_STORY)
            {
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
        zodiac::PlugHandle SettingNodeOutPlug = settingNode->createOutgoingPlug("out");   //create the outgoing plugs
        zodiac::PlugHandle ThemeNodeOutPlug = themeNode->createOutgoingPlug("out");
        zodiac::PlugHandle PlotNodeOutPlug = plotNode->createOutgoingPlug("out");
        zodiac::PlugHandle ResolutionNodeOutPlug = resolutionNode->createOutgoingPlug("out");

        //load the setting items
        std::list<SettingItem> chars = m_saveAndLoadManager.GetCharacters();
        loadSettingItem(settingNode, chars, zodiac::STORY_SETTING_CHARACTER_GROUP, zodiac::STORY_SETTING_CHARACTER, "Characters");

        std::list<SettingItem> locs = m_saveAndLoadManager.GetLocations();
        loadSettingItem(settingNode, locs, zodiac::STORY_SETTING_LOCATION_GROUP, zodiac::STORY_SETTING_LOCATION, "Locations");

        std::list<SettingItem> times = m_saveAndLoadManager.GetTimes();
        loadSettingItem(settingNode, times, zodiac::STORY_SETTING_TIME_GROUP, zodiac::STORY_SETTING_TIME, "Times");

        //load the theme items
        std::list<EventGoal> events = m_saveAndLoadManager.GetEvents();
        if(events.size() > 0)
        {
            //create group node then load the events and all sub-events
            NodeCtrl* parentNode = createNode(zodiac::STORY_THEME_EVENT_GROUP, "Events");
            parentNode->getNodeHandle().setPos(themeNode->getPos().x(), themeNode->getPos().y()+100);
            zodiac::PlugHandle parentNodeInPlug = parentNode->getNodeHandle().createIncomingPlug("in");
            ThemeNodeOutPlug.connectPlug(parentNodeInPlug);

            loadThemeItem(parentNode, events, zodiac::STORY_THEME_EVENT);
        }

        std::list<EventGoal> goals = m_saveAndLoadManager.GetGoals();
        if(goals.size() > 0)
        {
            //same as events but goals
            NodeCtrl* parentNode = createNode(zodiac::STORY_THEME_GOAL_GROUP, "Goals");
            parentNode->getNodeHandle().setPos(themeNode->getPos().x(), themeNode->getPos().y()+100);
            zodiac::PlugHandle parentNodeInPlug = parentNode->getNodeHandle().createIncomingPlug("in");
            ThemeNodeOutPlug.connectPlug(parentNodeInPlug);

            loadThemeItem(parentNode, goals, zodiac::STORY_THEME_GOAL);
        }

        //load the plot items (episodes)
        std::list<Episode> episodes = m_saveAndLoadManager.GetEpisodes();
        loadEpisodes(plotNode, episodes);

        //load the resolution
        std::list<EventGoal> resEvents = m_saveAndLoadManager.GetResolution().events;
        std::list<SimpleNode> resStates = m_saveAndLoadManager.GetResolution().states;
        loadResolution(resolutionNode, resEvents, resStates);
    }
}

void MainCtrl::loadSettingItem(zodiac::NodeHandle *settingsNode, std::list<SettingItem> items, zodiac::StoryNodeType parentType, zodiac::StoryNodeType childType, QString parentName)
{
    if(items.size() == 0)   //if no items, don't create the group
        return;

    //create parent group node, move it, link to main settings node and set up plug for linking children
    NodeCtrl* parentNode = createNode(parentType, parentName);
    parentNode->getNodeHandle().setPos(settingsNode->getPos().x(), settingsNode->getPos().y()+100);

    zodiac::PlugHandle parentNodeInPlug = parentNode->getNodeHandle().createIncomingPlug("in");
    settingsNode->getPlug("out").connectPlug(parentNodeInPlug);

    //add each item to the tree
    for(std::list<SettingItem>::iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt)
    {
        NodeCtrl *childNode = createStoryNode(parentNode, childType, (*itemIt).id, QPoint(0, 100));

        //add all the details for the items
        std::list<SimpleNodeWithState> details = (*itemIt).details;
        for(std::list<SimpleNodeWithState>::iterator detailIt = details.begin(); detailIt != details.end(); ++detailIt)
        {
            createStoryNode(childNode, zodiac::STORY_ITEM_DETAILS, (*detailIt).id, QPoint(0, 100));
        }
    }
}

void MainCtrl::loadThemeItem(NodeCtrl* parentNode, std::list<EventGoal> items, zodiac::StoryNodeType childType)
{
    //add each item to the tree
    for(std::list<EventGoal>::iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt)
    {
        NodeCtrl *itemNode = createStoryNode(parentNode, childType, (*itemIt).id, QPoint(0, 100));

        //if sub-item then load those too
        if((*itemIt).subItems.size() > 0)
        {
            std::list<EventGoal> subItems = (*itemIt).subItems;
            for(std::list<EventGoal>::iterator subItemIt = subItems.begin(); subItemIt != subItems.end(); ++subItemIt)
            {
                loadThemeItem(itemNode, subItems, childType);

            }
        }
    }
}

void MainCtrl::loadEpisodes(zodiac::NodeHandle* parentNode, std::list<Episode> episodes)
{
    //add each item to the tree
    for(std::list<Episode>::iterator epIt = episodes.begin(); epIt != episodes.end(); ++epIt)
    {
        NodeCtrl *episodeNode = createNode(zodiac::STORY_PLOT_EPISODE, (*epIt).id);
        episodeNode->getNodeHandle().setPos(parentNode->getPos().x(), parentNode->getPos().y()+100);
        zodiac::PlugHandle episodeNodeInPlug = episodeNode->getNodeHandle().createIncomingPlug("in");
        parentNode->getPlug("out").connectPlug(episodeNodeInPlug);


        NodeCtrl *attemptGroupNode = createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP, "Attempt", QPoint(0, 100));
        NodeCtrl *outcomeGroupNode = createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_OUTCOME_GROUP, "Outcome", QPoint(0, 100));

        //handle attempts
        for(std::list<SimpleNodeWithState>::iterator attIt = (*epIt).attempts.begin(); attIt != (*epIt).attempts.end(); ++attIt)
        {
            createStoryNode(attemptGroupNode, zodiac::STORY_PLOT_EPISODE_ATTEMPT, (*attIt).id, QPoint(0, 100));
        }

        //handle attempt sub-episodes
        loadEpisodes(&attemptGroupNode->getNodeHandle(), (*epIt).attemptSubEpisodes);
        /*for(std::list<Episode>::iterator attEpIt = (*epIt).attemptSubEpisodes.begin(); attEpIt != (*epIt).attemptSubEpisodes.end(); ++attEpIt)
        {

        }*/

        //handle outcomes
        for(std::list<SimpleNodeWithState>::iterator outIt = (*epIt).outcomes.begin(); outIt != (*epIt).outcomes.end(); ++outIt)
        {
            createStoryNode(outcomeGroupNode, zodiac::STORY_PLOT_EPISODE_OUTCOME, (*outIt).id, QPoint(0, 100));
        }

        //handle outcome sub-episodes
        loadEpisodes(&outcomeGroupNode->getNodeHandle(), (*epIt).outcomeSubEpisodes);
        /*for(std::list<Episode>::iterator outEpIt = (*epIt).outcomeSubEpisodes.begin(); outEpIt != (*epIt).outcomeSubEpisodes.end(); ++outEpIt)
        {

        }*/

        //handle sub-goal
        createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_SUBGOAL, (*epIt).stateID, QPoint(0, 100));
    }
}

void MainCtrl::loadResolution(zodiac::NodeHandle* resolutionNode, std::list<EventGoal> events, std::list<SimpleNode> states)
{
    if(events.size() > 0)
    {
        NodeCtrl *eventNode = createNode(zodiac::STORY_RESOLUTION_EVENT_GROUP, "Event");
        eventNode->getNodeHandle().setPos(resolutionNode->getPos().x(), resolutionNode->getPos().y()+100);
        zodiac::PlugHandle eventNodeInPlug = eventNode->getNodeHandle().createIncomingPlug("in");
        resolutionNode->getPlug("out").connectPlug(eventNodeInPlug);

        for(std::list<EventGoal>::iterator evIt = events.begin(); evIt != events.end(); ++evIt)
        {
            createStoryNode(eventNode, zodiac::STORY_RESOLUTION_EVENT, (*evIt).id, QPoint(0, 100));
        }
    }

    if(states.size() > 0)
    {
        NodeCtrl *stateNode = createNode(zodiac::STORY_RESOLUTION_STATE_GROUP, "State");
        stateNode->getNodeHandle().setPos(resolutionNode->getPos().x(), resolutionNode->getPos().y()+100);
        zodiac::PlugHandle stateNodeInPlug = stateNode->getNodeHandle().createIncomingPlug("in");
        resolutionNode->getPlug("out").connectPlug(stateNodeInPlug);

        for(std::list<SimpleNode>::iterator stIt = states.begin(); stIt != states.end(); ++stIt)
        {
            createStoryNode(stateNode, zodiac::STORY_RESOLUTION_STATE, (*stIt).id, QPoint(0, 100));
        }
    }
}
