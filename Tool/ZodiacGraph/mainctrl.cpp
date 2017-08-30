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

void MainCtrl::createStoryGraph()
{
    NodeCtrl* nameNode = createNode(zodiac::STORY_NAME, "Story"); //create story name

    NodeCtrl* settingNode = createNode(zodiac::STORY_SETTING, "Setting"); //create setting
    NodeCtrl* themeNode = createNode(zodiac::STORY_THEME, "Theme"); //create theme
    NodeCtrl* plotNode = createNode(zodiac::STORY_PLOT, "Plot"); //create plot
    NodeCtrl* resolutionNode = createNode(zodiac::STORY_RESOLUTION, "Resolution"); //create resolution
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
}
