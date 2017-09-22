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

NodeCtrl* MainCtrl::createStoryNode(NodeCtrl *parent, zodiac::StoryNodeType type, QString name, QString description, QPoint &pos, bool relative, bool load)
{
    NodeCtrl* child = createNode(type, name, description, load);

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

            maxXVal = loadSettingItem(characterNode, chars, zodiac::STORY_SETTING_CHARACTER);

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

            maxXVal = loadSettingItem(locationNode, locs, zodiac::STORY_SETTING_LOCATION);

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

            maxXVal = loadSettingItem(timeNode, times, zodiac::STORY_SETTING_TIME);

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

        if(centreIt != 0)  //if these nodes have been created, centre the theme node
        {
            centrePos /= centreIt;
            themeNode->setPos(centrePos, themeNode->getPos().y());

            centrePos = centreIt = 0;
        }

        //load the plot items (episodes)
        std::list<Episode> episodes = m_saveAndLoadManager.GetEpisodes();
        plotNode->setPos(maxXVal, plotNode->getPos().y());
        maxXVal = loadEpisodes(plotNode, episodes).y() + 100;
        resolutionNode->setPos(maxXVal, resolutionNode->getPos().x());

        //load the resolution
        std::list<EventGoal> resEvents = m_saveAndLoadManager.GetResolution().events;
        std::list<SimpleNode> resStates = m_saveAndLoadManager.GetResolution().states;
        resolutionNode->setPos(maxXVal, resolutionNode->getPos().y());
        loadResolution(resolutionNode, resEvents, resStates);

        //centre the main node
        startingNode->setPos((settingNode->getPos().x() + themeNode->getPos().x() + plotNode->getPos().x() + resolutionNode->getPos().x())/4, startingNode->getPos().y());
    }
}

float MainCtrl::loadSettingItem(NodeCtrl *parentNode, std::list<SettingItem> items, zodiac::StoryNodeType childType)
{
    float minX = INFINITY;
    float maxX = -INFINITY;

    float nodePos = parentNode->getPos().x();

    float parentPos = 0;
    int parentPosIt = 0;
    //add each item to the tree
    for(std::list<SettingItem>::iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt)
    {
        NodeCtrl *childNode = createStoryNode(parentNode, childType, (*itemIt).id, (*itemIt).description, QPoint(nodePos - parentNode->getPos().x(), 100));

        if(childNode->getPos().x() < minX)
            minX = childNode->getPos().x();
        if(childNode->getPos().x() > maxX)
            maxX = childNode->getPos().x();

        if((*itemIt).details.size() > 0)
        {
            float detailSpacer = 0;
            float childCentrePos = 0;
            int childCentreIt = 0;
            //add all the details for the items
            for(std::list<SimpleNodeWithState>::iterator detailIt = (*itemIt).details.begin(); detailIt != (*itemIt).details.end(); ++detailIt)
            {
                NodeCtrl* detailNode = createStoryNode(childNode, zodiac::STORY_ITEM_DETAILS, (*detailIt).id, (*detailIt).description, QPoint(detailSpacer, 100));

                if(detailNode->getPos().x() < minX)
                    minX = detailNode->getPos().x();
                if(detailNode->getPos().x() > maxX)
                    maxX = detailNode->getPos().x();

                detailSpacer += 100;

                childCentrePos += detailNode->getPos().x();
                ++childCentreIt;
            }

            childNode->setPos(childCentrePos/childCentreIt, childNode->getPos().y());
        }

        parentPos += childNode->getPos().x();
        ++parentPosIt;

        nodePos = maxX += 100;
    }
        parentNode->setPos(parentPos/parentPosIt, parentNode->getPos().y(), false);

    return maxX;
}

QPointF MainCtrl::loadThemeItem(NodeCtrl* parentNode, std::list<EventGoal> items, zodiac::StoryNodeType childType)
{
    float minX = INFINITY;
    float maxX = -INFINITY;

    float nodePos = 0;
    float parentPos = 0;
    int parentPosIt = 0;

    //add each item to the tree
    for(std::list<EventGoal>::iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt)
    {
        NodeCtrl *itemNode = createStoryNode(parentNode, childType, (*itemIt).id, (*itemIt).description, QPoint(nodePos, 100));

        parentPos += nodePos + parentNode->getPos().x();
        ++parentPosIt;

        if(nodePos + parentNode->getPos().x()  < minX)
            minX = nodePos + parentNode->getPos().x();
        if(nodePos + parentNode->getPos().x() > maxX)
            maxX = nodePos + parentNode->getPos().x();

        //if sub-item then load those too
        if((*itemIt).subItems.size() > 0)
        {
            float localMaxX = 0;
            for(std::list<EventGoal>::iterator subItemIt = (*itemIt).subItems.begin(); subItemIt != (*itemIt).subItems.end(); ++subItemIt)
            {
                QPointF minMax = loadThemeItem(itemNode, (*itemIt).subItems, childType);

                localMaxX = minMax.y() - itemNode->getPos().x();

                if(minMax.x() < minX)
                    minX = minMax.x();
                if(minMax.y() > maxX)
                    maxX = minMax.y();
            }
            nodePos += localMaxX;
        }

        nodePos += 100;
    }

    parentNode->setPos(parentPos/parentPosIt, parentNode->getPos().y(), false);

    return QPointF(minX, maxX);
}

QPointF MainCtrl::loadEpisodes(zodiac::NodeHandle *parentNode, std::list<Episode> episodes)
{
    NodeCtrl *parentNodeCtrl = new NodeCtrl(this, *parentNode);

    float mainMinX = INFINITY;
    float mainMaxX = -INFINITY;

    float nodePos = parentNodeCtrl->getPos().x();
    float parentPos = 0;
    int parentPosIt = 0;

    //add each item to the tree
    for(std::list<Episode>::iterator epIt = episodes.begin(); epIt != episodes.end(); ++epIt)
    {
        float epMinX = INFINITY;
        float epMaxX = -INFINITY;

        NodeCtrl *episodeNode = createStoryNode(parentNodeCtrl, zodiac::STORY_PLOT_EPISODE, (*epIt).id, (*epIt).description, QPoint(nodePos - parentNodeCtrl->getPos().x(), 100), true, true);

        if(episodeNode->getPos().x() < epMinX)
            epMinX = episodeNode->getPos().x();
        if(episodeNode->getPos().x() > epMaxX)
            epMaxX = episodeNode->getPos().x();

        NodeCtrl *attemptGroupNode = createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP, "Attempt", "Attempt", QPoint(0, 100));
        //handle attempts
        if((*epIt).attempts.size() > 0 || (*epIt).attemptSubEpisodes.size() > 0)
        {
            float attemptPos = 0;
            float attemptMinX = INFINITY;
            float attemptMaxX = -INFINITY;

            for(std::list<SimpleNodeWithState>::iterator attIt = (*epIt).attempts.begin(); attIt != (*epIt).attempts.end(); ++attIt)
            {
                NodeCtrl *attemptNode = createStoryNode(attemptGroupNode, zodiac::STORY_PLOT_EPISODE_ATTEMPT, (*attIt).id, (*attIt).description, QPoint(attemptPos, 100));
                attemptPos += 100;     

                if(attemptNode->getPos().x()  < attemptMinX)
                    attemptMinX = attemptNode->getPos().x();
                if(attemptNode->getPos().x() > attemptMaxX)
                    attemptMaxX = attemptNode->getPos().x();
            }

            if((*epIt).attemptSubEpisodes.size() > 0)
            {
                QPointF oldAttemptPos = attemptGroupNode->getPos();
                attemptGroupNode->setPos(attemptMaxX, oldAttemptPos.y());

                //handle outcome sub-episodes
                QPointF attemptMinMax = loadEpisodes(&attemptGroupNode->getNodeHandle(), (*epIt).attemptSubEpisodes);

                if(attemptMinMax.x()  < attemptMinX)
                    attemptMinX = attemptMinMax.x();
                if(attemptMinMax.y() > attemptMaxX)
                    attemptMaxX = attemptMinMax.y();
            }

            //centre node
            float attemptParentPos = 0;
            int attemptParentPosIt = 0;
            //get all connected plugs
            QList<zodiac::PlugHandle> connectedPlugs = attemptGroupNode->getNodeHandle().getPlug("out").getConnectedPlugs();
            if(connectedPlugs.size() > 1)
            {
                for(QList<zodiac::PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
                {
                   zodiac::NodeHandle connectedNode = (*connectedPlugIt).getNode();
                   attemptParentPos += connectedNode.getPos().x();
                   ++attemptParentPosIt;
                }
                attemptGroupNode->setPos(attemptParentPos/attemptParentPosIt, attemptGroupNode->getPos().y(), false);
            }

            if(attemptMinX < epMinX)
                epMinX = attemptMinX;
            if(attemptMaxX > epMaxX)
                epMaxX = attemptMaxX;
        }

        epMaxX += 100; //gap between nodes

        NodeCtrl *outcomeGroupNode = createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_OUTCOME_GROUP, "Outcome", "Outcome", QPoint(epMaxX - episodeNode->getPos().x(), 100));
        if((*epIt).outcomes.size() > 0 || (*epIt).outcomeSubEpisodes.size() > 0)
        {
            //handle outcomes
            float outcomePos = 0;
            float outcomeMinX = INFINITY;
            float outcomeMaxX = -INFINITY;
            for(std::list<SimpleNodeWithState>::iterator outIt = (*epIt).outcomes.begin(); outIt != (*epIt).outcomes.end(); ++outIt)
            {
                NodeCtrl *outcomeNode = createStoryNode(outcomeGroupNode, zodiac::STORY_PLOT_EPISODE_OUTCOME, (*outIt).id, (*outIt).description, QPoint(outcomePos, 100));

                if(outcomeNode->getPos().x() < outcomeMinX)
                    outcomeMinX = outcomeNode->getPos().x();
                if(outcomeNode->getPos().x() > outcomeMaxX)
                    outcomeMaxX = outcomeNode->getPos().x();

                outcomePos += 100;
            }

            if((*epIt).outcomeSubEpisodes.size() > 0)
            {
                QPointF oldOutcomePos = outcomeGroupNode->getPos();
                outcomeGroupNode->setPos(outcomeMaxX, oldOutcomePos.y());

                //handle outcome sub-episodes
                QPointF outcomeMinMax = loadEpisodes(&outcomeGroupNode->getNodeHandle(), (*epIt).outcomeSubEpisodes);

                if(outcomeMinMax.x()  < outcomeMinX)
                    outcomeMinX = outcomeMinMax.x();
                if(outcomeMinMax.y() > outcomeMaxX)
                    outcomeMaxX = outcomeMinMax.y();
            }

            //centre node
            float outcomeParentPos = 0;
            float outcomeParentPosIt = 0;
            //get all connected plugs
            QList<zodiac::PlugHandle> connectedPlugs = outcomeGroupNode->getNodeHandle().getPlug("out").getConnectedPlugs();
            if(connectedPlugs.size() > 1)
            {
                for(QList<zodiac::PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
                {
                   zodiac::NodeHandle connectedNode = (*connectedPlugIt).getNode();
                   outcomeParentPos += connectedNode.getPos().x();
                   ++outcomeParentPosIt;
                }
                outcomeGroupNode->setPos(outcomeParentPos/outcomeParentPosIt, outcomeGroupNode->getPos().y(), false);
            }

            if(outcomeMinX < epMinX)
                epMinX = outcomeMinX;
            if(outcomeMaxX > epMaxX)
                epMaxX = outcomeMaxX;
        }

        //handle sub-goal
        epMaxX += 100; //gap between nodes

        NodeCtrl *subGoalNode = createStoryNode(episodeNode, zodiac::STORY_PLOT_EPISODE_SUBGOAL, (*epIt).stateID, (*epIt).stateDescription, QPoint(epMaxX - episodeNode->getPos().x(), 100));

        //centre episode node
        episodeNode->setPos((attemptGroupNode->getPos().x() + outcomeGroupNode->getPos().x() + subGoalNode->getPos().x())/3, episodeNode->getPos().y());

        parentPos += episodeNode->getPos().x();
        ++parentPosIt;

        if(epMinX < mainMinX)
            mainMinX = epMinX;
        if(epMaxX > mainMaxX)
            mainMaxX = epMaxX;

        nodePos = epMaxX += 100;
    }

    parentNodeCtrl->setPos(parentPos/parentPosIt, parentNodeCtrl->getPos().y());


    qDebug() << mainMaxX;

    return QPointF(mainMinX, mainMaxX);
}

void MainCtrl::loadResolution(zodiac::NodeHandle *resolutionNode, std::list<EventGoal> events, std::list<SimpleNode> states)
{
    float maxX = resolutionNode->getPos().x();

    float parentCentrePos = 0;
    int parentCentreIt = 0;

    if(events.size() > 0)
    {
        NodeCtrl *eventNode = createNode(zodiac::STORY_RESOLUTION_EVENT_GROUP, "Event");
        eventNode->getNodeHandle().setPos(resolutionNode->getPos().x(), resolutionNode->getPos().y()+100);
        zodiac::PlugHandle eventNodeInPlug = eventNode->getNodeHandle().createIncomingPlug("in");
        resolutionNode->getPlug("out").connectPlug(eventNodeInPlug);


        float nodePos = 0;
        float centrePos = 0;
        int nodeIt = 0;
        for(std::list<EventGoal>::iterator evIt = events.begin(); evIt != events.end(); ++evIt)
        {
            NodeCtrl *childNode = createStoryNode(eventNode, zodiac::STORY_RESOLUTION_EVENT, (*evIt).id, (*evIt).description, QPoint(nodePos, 100));

            nodePos += 100;

            maxX =  childNode->getPos().x();

            centrePos += maxX;
            ++nodeIt;
        }

        eventNode->setPos(centrePos/nodeIt, eventNode->getPos().y());
        maxX += 100;

        parentCentrePos += eventNode->getPos().x();
        ++parentCentreIt;
    }

   if(states.size() > 0)
    {
        NodeCtrl *stateNode = createNode(zodiac::STORY_RESOLUTION_STATE_GROUP, "State");
        stateNode->getNodeHandle().setPos(maxX, resolutionNode->getPos().y()+100);
        zodiac::PlugHandle stateNodeInPlug = stateNode->getNodeHandle().createIncomingPlug("in");
        resolutionNode->getPlug("out").connectPlug(stateNodeInPlug);


        float nodePos = 0;
        float centrePos = 0;
        int nodeIt = 0;
        for(std::list<SimpleNode>::iterator stIt = states.begin(); stIt != states.end(); ++stIt)
        {
            NodeCtrl *childNode = createStoryNode(stateNode, zodiac::STORY_RESOLUTION_STATE, (*stIt).id, (*stIt).description, QPoint(nodePos, 100));

            nodePos += 100;

            centrePos += childNode->getPos().x();
            ++nodeIt;
        }

        stateNode->setPos(centrePos/nodeIt, stateNode->getPos().y());

        parentCentrePos += stateNode->getPos().x();
        ++parentCentreIt;
    }   

   if(parentCentreIt > 0)
       resolutionNode->setPos(parentCentrePos/parentCentreIt, resolutionNode->getPos().y());
}
