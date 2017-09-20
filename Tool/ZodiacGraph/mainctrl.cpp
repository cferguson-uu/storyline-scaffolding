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

NodeCtrl* MainCtrl::createNode(zodiac::StoryNodeType storyType, const QString& name, const QString& description)
{
    // the newly created Node is the only selected one to avoid confusion
    m_scene.deselectAll();

    // use the given name or construct a default one
    QString nodeName = name;
    if(nodeName.isEmpty()){
        nodeName = s_defaultName + QString::number(m_nodeIndex++);
    }

    // create the node
    NodeCtrl* nodeCtrl = new NodeCtrl(this, m_scene.createNode(nodeName, description, storyType));
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

NodeCtrl* MainCtrl::createStoryNode(NodeCtrl *parent, zodiac::StoryNodeType type, QString name, QString description, QPoint &pos, bool relative)
{
    NodeCtrl* child = createNode(type, name, description);

    //set the position
    if(relative)
        child->setPos(parent->getPos().x() + pos.x(), parent->getPos().y() + pos.y());
    else
        child->setPos(pos.x(), pos.y());

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
                    startingNode = &(*it); //get a pointer to the handle of the settings node
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
        zodiac::PlugHandle SettingNodeOutPlug = settingNode->createOutgoingPlug("out");   //create the outgoing plugs
        zodiac::PlugHandle ThemeNodeOutPlug = themeNode->createOutgoingPlug("out");
        zodiac::PlugHandle PlotNodeOutPlug = plotNode->createOutgoingPlug("out");
        zodiac::PlugHandle ResolutionNodeOutPlug = resolutionNode->createOutgoingPlug("out");

        float maxXVal = settingNode->getPos().x();
        int centreIt = 0;
        float centrePos = settingNode->getPos().x();

        //load the setting items
        std::list<SettingItem> chars = m_saveAndLoadManager.GetCharacters();
        NodeCtrl* characterNode = nullptr;
        if(chars.size() > 0)
        {
            //create parent group node, move it, link to main settings node and set up plug for linking children
            characterNode = createNode(zodiac::STORY_SETTING_CHARACTER_GROUP, "Characters");
            characterNode->getNodeHandle().setPos(maxXVal, settingNode->getPos().y()+100);

            zodiac::PlugHandle characterNodeInPlug = characterNode->getNodeHandle().createIncomingPlug("in");
            SettingNodeOutPlug.connectPlug(characterNodeInPlug);

            maxXVal = loadSettingItem(characterNode, chars, zodiac::STORY_SETTING_CHARACTER, false);

            centrePos += characterNode->getPos().x();
            ++centreIt;
        }

        std::list<SettingItem> locs = m_saveAndLoadManager.GetLocations();
        NodeCtrl* locationNode = nullptr;
        if(locs.size() > 0)
        {
            //create parent group node, move it, link to main settings node and set up plug for linking children
            locationNode = createNode(zodiac::STORY_SETTING_LOCATION_GROUP, "Locations");
            locationNode->getNodeHandle().setPos(maxXVal, settingNode->getPos().y()+100);

            zodiac::PlugHandle locationNodeInPlug = locationNode->getNodeHandle().createIncomingPlug("in");
            SettingNodeOutPlug.connectPlug(locationNodeInPlug);

            maxXVal = loadSettingItem(locationNode, locs, zodiac::STORY_SETTING_LOCATION, true);

            centrePos += locationNode->getPos().x();
            ++centreIt;
        }

        std::list<SettingItem> times = m_saveAndLoadManager.GetTimes();
        NodeCtrl* timeNode = nullptr;
        if(times.size() > 0)
        {
            //create parent group node, move it, link to main settings node and set up plug for linking children
            timeNode = createNode(zodiac::STORY_SETTING_TIME_GROUP, "Times");
            timeNode->getNodeHandle().setPos(maxXVal, settingNode->getPos().y()+100);

            zodiac::PlugHandle timeNodeInPlug = timeNode->getNodeHandle().createIncomingPlug("in");
            SettingNodeOutPlug.connectPlug(timeNodeInPlug);

            maxXVal = loadSettingItem(timeNode, times, zodiac::STORY_SETTING_TIME, true);

            centrePos += timeNode->getPos().x();
            ++centreIt;
        }

        if(centreIt != 0)  //if these nodes have been created, centre the setting node
        {
            centrePos /= centreIt;
            settingNode->setPos(centrePos, settingNode->getPos().y());

            centrePos = centreIt = 0;
        }

        //load the theme items
        std::list<EventGoal> events = m_saveAndLoadManager.GetEvents();
        NodeCtrl* eventNode = nullptr;
        if(events.size() > 0)
        {
            //create group node then load the events and all sub-events
            eventNode = createNode(zodiac::STORY_THEME_EVENT_GROUP, "Events");
            eventNode->getNodeHandle().setPos(/*themeNode->getPos().x()*/maxXVal, themeNode->getPos().y()+100);
            zodiac::PlugHandle parentNodeInPlug = eventNode->getNodeHandle().createIncomingPlug("in");
            ThemeNodeOutPlug.connectPlug(parentNodeInPlug);

            maxXVal = loadThemeItem(eventNode, events, zodiac::STORY_THEME_EVENT).y() + 100;

            centrePos += eventNode->getPos().x();
            ++centreIt;
        }

        std::list<EventGoal> goals = m_saveAndLoadManager.GetGoals();
        NodeCtrl* goalNode = nullptr;
        if(goals.size() > 0)
        {
            //same as events but goals
            goalNode = createNode(zodiac::STORY_THEME_GOAL_GROUP, "Goals");
            goalNode->getNodeHandle().setPos(/*themeNode->getPos().x()*/maxXVal, themeNode->getPos().y()+100);
            zodiac::PlugHandle parentNodeInPlug = goalNode->getNodeHandle().createIncomingPlug("in");
            ThemeNodeOutPlug.connectPlug(parentNodeInPlug);

            maxXVal = loadThemeItem(goalNode, goals, zodiac::STORY_THEME_GOAL).y() + 100;

            centrePos += goalNode->getPos().x();
            ++centreIt;
        }

        if(centreIt != 0)  //if these nodes have been created, centre the setting node
        {
            centrePos /= centreIt;
            themeNode->setPos(centrePos, themeNode->getPos().y());

            centrePos = centreIt = 0;
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

float MainCtrl::loadSettingItem(NodeCtrl *parentNode, std::list<SettingItem> items, zodiac::StoryNodeType childType, bool move)
{
    float minX = INFINITY;
    float maxX = -INFINITY;

    float nodePos = parentNode->getPos().x();

    if(items.size() > 1)
    {
        float averageDetails = 0;

        for(std::list<SettingItem>::iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt)
        {
            averageDetails += (*itemIt).details.size();
        }

        averageDetails /= items.size();

        if(averageDetails > 0)
            nodePos = (averageDetails * -0.5) * 100;
        else
            nodePos = (0.5 + (items.size() * -0.5)) * 100;
    }

    //add each item to the tree
    for(std::list<SettingItem>::iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt)
    {
        if(nodePos + parentNode->getPos().x() < minX)
            minX = nodePos + parentNode->getPos().x();
        if(nodePos + parentNode->getPos().x() > maxX)
            maxX = nodePos + parentNode->getPos().x();

        NodeCtrl *childNode = createStoryNode(parentNode, childType, (*itemIt).id, (*itemIt).description, QPoint(nodePos, 100));

        float detailSpacer = (*itemIt).details.size() * -0.5 + 0.5; //calculate spacing

        //add all the details for the items
        for(std::list<SimpleNodeWithState>::iterator detailIt = (*itemIt).details.begin(); detailIt != (*itemIt).details.end(); ++detailIt)
        {
            if((detailSpacer * 100) + childNode->getPos().x() < minX)
                minX = (detailSpacer * 100) + childNode->getPos().x();
            if((detailSpacer * 100) + childNode->getPos().x() > maxX)
                maxX = (detailSpacer * 100) + childNode->getPos().x();

            createStoryNode(childNode, zodiac::STORY_ITEM_DETAILS, (*detailIt).id, (*detailIt).description, QPoint(detailSpacer * 100, 100));
            detailSpacer += 1;
        }

        std::list<SettingItem>::iterator nx = std::next(itemIt, 1);
        if(nx != items.end())
        {
            if((*nx).details.size() == 0)
                nodePos += 150;
            else
                nodePos += ((*itemIt).details.size() + (*nx).details.size())/2 * 100 + 50;
        }
    }

    if(move)
    {
        float width = maxX - minX;
        parentNode->setPos(parentNode->getPos().x() + width, parentNode->getPos().y(), true);
        maxX += width;
    }

    qDebug() << parentNode->getName() << parentNode->getPos().x() << " minX " << minX << " maxX " << maxX;

    return maxX + 100;
}

QPointF MainCtrl::loadThemeItem(NodeCtrl* parentNode, std::list<EventGoal> items, zodiac::StoryNodeType childType)
{
    float minX = INFINITY;
    float maxX = -INFINITY;

    float nodePos = parentNode->getPos().x();

    if(items.size() > 1)
    {
        float averageDetails = 0;

        for(std::list<EventGoal>::iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt)
        {
            averageDetails = getThemeItemWidth((*itemIt));
        }

        averageDetails /= items.size();

        if(averageDetails > 0)
            nodePos = (averageDetails * -0.5) * 100;
        else
            nodePos = (0.5 + (items.size() * -0.5)) * 100;
    }

    //add each item to the tree
    for(std::list<EventGoal>::iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt)
    {
        NodeCtrl *itemNode = createStoryNode(parentNode, childType, (*itemIt).id, (*itemIt).description, QPoint(0, 100));

        if(nodePos < minX)
            minX = nodePos;
        if(nodePos > maxX)
            maxX = nodePos;

        //if sub-item then load those too
        if((*itemIt).subItems.size() > 0)
        {
            std::list<EventGoal> subItems = (*itemIt).subItems;
            for(std::list<EventGoal>::iterator subItemIt = subItems.begin(); subItemIt != subItems.end(); ++subItemIt)
            {
                QPointF minMax = loadThemeItem(itemNode, subItems, childType);

                if(minMax.x() < minX)
                    minX = minMax.x();
                if(minMax.y() > maxX)
                    maxX = minMax.y();
            }
        }

        std::list<EventGoal>::iterator nx = std::next(itemIt, 1);
        if(nx != items.end())
        {
            if((*nx).subItems.size() == 0)
                nodePos += 150;
            else
                nodePos += (getThemeItemWidth((*itemIt)) + getThemeItemWidth((*nx)))/2 * 100 + 50;
        }
    }

    /*//if(move)
    //{
        float width = maxX - minX;
        parentNode->setPos(parentNode->getPos().x() + width, parentNode->getPos().y(), true);
        maxX += width;
    //}*/

    return QPointF(minX, maxX);
}

int MainCtrl::getThemeItemWidth(EventGoal &item)
{
    int i = 0;

    for(std::list<EventGoal>::iterator itemIt = item.subItems.begin(); itemIt != item.subItems.end(); ++itemIt)
    {
        ++i;
        i += getThemeItemWidth((*itemIt));
    }

    return i;
}

void MainCtrl::loadEpisodes(zodiac::NodeHandle *parentNode, std::list<Episode> episodes)
{
    //add each item to the tree
    for(std::list<Episode>::iterator epIt = episodes.begin(); epIt != episodes.end(); ++epIt)
    {
        NodeCtrl *episodeNode = createNode(zodiac::STORY_PLOT_EPISODE, (*epIt).id, (*epIt).description);
        episodeNode->getNodeHandle().setPos(parentNode->getPos().x(), parentNode->getPos().y()+100);
        zodiac::PlugHandle episodeNodeInPlug = episodeNode->getNodeHandle().createIncomingPlug("in");
        parentNode->getPlug("out").connectPlug(episodeNodeInPlug);


        NodeCtrl *attemptGroupNode = createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP, "Attempt", "Attempt", QPoint(0, 100));
        NodeCtrl *outcomeGroupNode = createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_OUTCOME_GROUP, "Outcome", "Outcome", QPoint(0, 100));

        //handle attempts
        for(std::list<SimpleNodeWithState>::iterator attIt = (*epIt).attempts.begin(); attIt != (*epIt).attempts.end(); ++attIt)
        {
            createStoryNode(attemptGroupNode, zodiac::STORY_PLOT_EPISODE_ATTEMPT, (*attIt).id, (*attIt).description, QPoint(0, 100));
        }

        //handle attempt sub-episodes
        loadEpisodes(&attemptGroupNode->getNodeHandle(), (*epIt).attemptSubEpisodes);
        /*for(std::list<Episode>::iterator attEpIt = (*epIt).attemptSubEpisodes.begin(); attEpIt != (*epIt).attemptSubEpisodes.end(); ++attEpIt)
        {

        }*/

        //handle outcomes
        for(std::list<SimpleNodeWithState>::iterator outIt = (*epIt).outcomes.begin(); outIt != (*epIt).outcomes.end(); ++outIt)
        {
            createStoryNode(outcomeGroupNode, zodiac::STORY_PLOT_EPISODE_OUTCOME, (*outIt).id, (*outIt).description, QPoint(0, 100));
        }

        //handle outcome sub-episodes
        loadEpisodes(&outcomeGroupNode->getNodeHandle(), (*epIt).outcomeSubEpisodes);
        /*for(std::list<Episode>::iterator outEpIt = (*epIt).outcomeSubEpisodes.begin(); outEpIt != (*epIt).outcomeSubEpisodes.end(); ++outEpIt)
        {

        }*/

        //handle sub-goal
        createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_SUBGOAL, (*epIt).stateID, (*epIt).stateDescription, QPoint(0, 100));
    }
}

void MainCtrl::loadResolution(zodiac::NodeHandle *resolutionNode, std::list<EventGoal> events, std::list<SimpleNode> states)
{
    if(events.size() > 0)
    {
        NodeCtrl *eventNode = createNode(zodiac::STORY_RESOLUTION_EVENT_GROUP, "Event");
        eventNode->getNodeHandle().setPos(resolutionNode->getPos().x(), resolutionNode->getPos().y()+100);
        zodiac::PlugHandle eventNodeInPlug = eventNode->getNodeHandle().createIncomingPlug("in");
        resolutionNode->getPlug("out").connectPlug(eventNodeInPlug);

        for(std::list<EventGoal>::iterator evIt = events.begin(); evIt != events.end(); ++evIt)
        {
            createStoryNode(eventNode, zodiac::STORY_RESOLUTION_EVENT, (*evIt).id, (*evIt).description, QPoint(0, 100));
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
            createStoryNode(stateNode, zodiac::STORY_RESOLUTION_STATE, (*stIt).id, (*stIt).description, QPoint(0, 100));
        }
    }
}
