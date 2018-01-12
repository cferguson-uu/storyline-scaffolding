#include "nodehandle.h"

#include "node.h"
#include "plug.h"
#include "scene.h"
#include "scenehandle.h"

namespace zodiac {

NodeHandle::NodeHandle(Node* node)
    : QObject(nullptr)
    , m_node(node)
    , m_isValid(node!=nullptr)
{
    connectSignals();
}

NodeHandle& NodeHandle::operator = (const NodeHandle& other)
{
    if(m_node){
        m_node->disconnect(this);
    }
    m_node = other.data();
    m_isValid = m_node != nullptr;
    connectSignals();
    return *this;
}

bool NodeHandle::isRemovable() const
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return false;
    }
#endif
    return m_node->isRemovable();
}

bool NodeHandle::remove()
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return false;
    }
#endif
    if(m_node->getScene()->removeNode(m_node)){
        m_isValid = false;
        return true;
    } else {
        return false;
    }
}

const QUuid& NodeHandle::getId() const
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        QUuid();
    }
#endif
    return m_node->getUniqueId();
}

const NodeType NodeHandle::getType() const
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        QUuid();
    }
#endif
    return m_node->getType();
}

QString NodeHandle::getName() const
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return "";
    }
#endif
    return m_node->getDisplayName();
}

void NodeHandle::rename(const QString& name)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return;
    }
#endif
    m_node->setDisplayName(name);
}

QString NodeHandle::getDescription() const
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return "";
    }
#endif
    return m_node->getDisplayDescription();
}

void NodeHandle::changeDescription(const QString& description)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return;
    }
#endif
    m_node->setDisplayDescription(description);
}

void NodeHandle::setLabelBackgroundColor(const QColor& color)
{
    #ifdef QT_DEBUG
        Q_ASSERT(m_isValid);
    #else
        if(!m_isValid){
            return;
        }
    #endif
    m_node->setLabelBackgroundColor(color);
}

QHash<QUuid, NodeCommand> NodeHandle::getOnUnlockList()
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return QHash<QUuid, NodeCommand>();
    }
#endif

    return nNode->getOnUnlockList();
}

QHash<QUuid, NodeCommand> NodeHandle::getOnFailList()
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return QHash<QUuid, NodeCommand>();
    }
#endif
    return nNode->getOnFailList();
}

QHash<QUuid, NodeCommand> NodeHandle::getOnUnlockedList()
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return QHash<QUuid, NodeCommand>();
    }
#endif
    return nNode->getOnUnlockedList();
}

void NodeHandle::addOnUnlockCommand(const QUuid& key, const QString& value, const QString& description)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->addOnUnlockCommand(key, value, description);
}

void NodeHandle::addOnFailCommand(const QUuid& key, const QString& value, const QString& description)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->addOnFailCommand(key, value, description);
}

void NodeHandle::addOnUnlockedCommand(const QUuid& key, const QString& value, const QString& description)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
     nNode->addOnUnlockedCommand(key, value, description);
}

void NodeHandle::removeOnUnlockCommand(const QUuid& key)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->removeOnUnlockCommand(key);
}
void NodeHandle::removeOnFailCommand(const QUuid& key)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->removeOnFailCommand(key);
}

void NodeHandle::removeOnUnlockedCommand(const QUuid& key)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->removeOnUnlockedCommand(key);
}

QString NodeHandle::getParameterFromOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return "";
    }
#endif
    return nNode->getParameterFromOnUnlockCommand(cmdKey, paramKey);
}

QString NodeHandle::getParameterFromOnFailCommand(const QUuid& cmdKey, const QString& paramKey)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return "";
    }
#endif
    return nNode->getParameterFromOnFailCommand(cmdKey, paramKey);
}

QString NodeHandle::getParameterFromOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return "";
    }
#endif
    return nNode->getParameterFromOnUnlockedCommand(cmdKey, paramKey);
}

void NodeHandle::addParameterToOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->addParameterToOnUnlockCommand(cmdKey, paramKey, value);
}

void NodeHandle::addParameterToOnFailCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->addParameterToOnFailCommand(cmdKey, paramKey, value);
}

void NodeHandle::addParameterToOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->addParameterToOnUnlockedCommand(cmdKey, paramKey, value);
}

void NodeHandle::removeParameterFromOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->removeParameterFromOnUnlockCommand(cmdKey, paramKey);
}

void NodeHandle::removeParameterFromOnFailCommand(const QUuid& cmdKey, const QString& paramKey)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->removeParameterFromOnFailCommand(cmdKey, paramKey);
}

void NodeHandle::removeParameterFromOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->removeParameterFromOnUnlockedCommand(cmdKey, paramKey);
}

void NodeHandle::removeAllParametersFromOnUnlockCommand(const QUuid& cmdKey)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->removeAllParametersFromOnUnlockCommand(cmdKey);
}

void NodeHandle::removeAllParametersFromOnFailCommand(const QUuid& cmdKey)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->removeAllParametersFromOnFailCommand(cmdKey);
}

void NodeHandle::removeAllParametersFromOnUnlockedCommand(const QUuid& cmdKey)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->removeAllParametersFromOnUnlockedCommand(cmdKey);
}

void NodeHandle::editParameterInOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->editParameterInOnUnlockCommand(cmdKey, paramKey, value);
}

void NodeHandle::editParameterInOnFailCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }
#endif
    nNode->editParameterInOnFailCommand(cmdKey, paramKey, value);
}

void NodeHandle::editParameterInOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }

#endif
    nNode->editParameterInOnUnlockedCommand(cmdKey, paramKey, value);
}

void NodeHandle::setLockedStatus(bool status)
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return;
    }

#endif

    nNode->setLockedStatus(status);
}

bool NodeHandle::getLockedStatus()
{
    NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(nNode);
#else
    if(!m_isValid || !nNode){
        return false;
    }

#endif
    return nNode->getLockedStatus();
}

StoryNodeType NodeHandle::getStoryNodeType()
{
    StoryNode *sNode = static_cast<StoryNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(sNode);
#else
    if(!m_isValid || !sNode){
        return STORY_ERROR;
    }
#endif
    return sNode->getStoryNodeType();
}

QString NodeHandle::getStoryNodePrefix()
{
    StoryNode *sNode = static_cast<StoryNode*>(m_node);
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
    Q_ASSERT(sNode);
#else
    if(!m_isValid || !sNode){
        return STORY_ERROR;
    }
#endif
    return sNode->getStoryNodePrefix();
}

const QColor& NodeHandle::getIdleColor()
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return QColor();
    }
#endif
    return m_node->getIdleColor();
}

void NodeHandle::setIdleColor(const QColor& color)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return;
    }
#endif
    return m_node->setIdleColor(color);
}

const QColor& NodeHandle::getSelectedColor()
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return QColor();
    }
#endif
    return m_node->getSelectedColor();
}

void NodeHandle::setSelectedColor(const QColor& color)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return;
    }
#endif
    return m_node->setSelectedColor(color);
}

const QColor& NodeHandle::getOutlineColor()
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return QColor();
    }
#endif
    return m_node->getOutlineColor();
}

void NodeHandle::setOutlineColor(const QColor& color)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return;
    }
#endif
    return m_node->setOutlineColor(color);
}

void NodeHandle::setOutlineWidth(qreal width)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return;
    }
#endif
    return m_node->setOutlineWidth(width);
}

void NodeHandle::softSetExpansion(NodeExpansion newState)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return;
    }
#endif
    m_node->softSetExpansion(newState);
}

PlugHandle NodeHandle::createIncomingPlug(const QString& name)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        PlugHandle();
    }
#endif
    return PlugHandle(m_node->createPlug(name, PlugDirection::IN));
}

PlugHandle NodeHandle::createOutgoingPlug(const QString& name)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        PlugHandle();
    }
#endif
    return PlugHandle(m_node->createPlug(name, PlugDirection::OUT));
}

QList<PlugHandle> NodeHandle::getPlugs() const
{
    QList<PlugHandle> result;
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return result;
    }
#endif
    QList<Plug*> plugs = m_node->getPlugs();
    result.reserve(plugs.size());
    for(Plug* plug : plugs){
        result.append(PlugHandle(plug));
    }
    return result;
}

PlugHandle NodeHandle::getPlug(const QString& name) const
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return PlugHandle();
    }
#endif
    return PlugHandle(m_node->getPlug(name));
}

void NodeHandle::setSelected(bool isSelected)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return;
    }
#endif
    m_node->setSelected(isSelected);
}

SceneHandle NodeHandle::getScene() const
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return SceneHandle();
    }
#endif
    return SceneHandle(m_node->getScene());
}

QPointF NodeHandle::getPos() const
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return QPointF();
    }
#endif
    return m_node->pos();
}

void NodeHandle::setPos(qreal x, qreal y, bool updateChildren, bool updateParents)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return;
    }
#endif
    QPointF oldPos = m_node->pos();
    m_node->setPos(x, y);
    m_node->updateConnectedEdges();

    if(updateChildren)
    {
        QPointF difference = m_node->pos() - oldPos;    //get difference between the two positions

        //get all outgoing plugs and iterate
        QList<PlugHandle> plugs = getPlugs();
        for(QList<PlugHandle>::iterator plugIt = plugs.begin(); plugIt != plugs.end(); ++plugIt)
        {
            if((*plugIt).isOutgoing())
            {
                //get all connected plugs
                QList<PlugHandle> connectedPlugs = (*plugIt).getConnectedPlugs();
                for(QList<PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
                {
                    NodeHandle newNode = (*connectedPlugIt).getNode();
                    QPointF newNodePos = newNode.getPos() + difference;    //add the difference to the old node position to move it relative to the parent
                    newNode.setPos(newNodePos.x(), newNodePos.y(), true);
                }
            }
        }
    }

    if(updateParents)
    {
        QPointF difference = m_node->pos() - oldPos;    //get difference between the two positions

        //get all incoming plugs and iterate
        QList<PlugHandle> plugs = getPlugs();
        for(QList<PlugHandle>::iterator plugIt = plugs.begin(); plugIt != plugs.end(); ++plugIt)
        {
            if((*plugIt).isIncoming())
            {
                //get all connected plugs
                QList<PlugHandle> connectedPlugs = (*plugIt).getConnectedPlugs();
                for(QList<PlugHandle>::iterator connectedPlugIt = connectedPlugs.begin(); connectedPlugIt != connectedPlugs.end(); ++connectedPlugIt)
                {
                    NodeHandle newNode = (*connectedPlugIt).getNode();
                    QPointF newNodePos = newNode.getPos() + difference;    //add the difference to the old node position to move it relative to the parent
                    newNode.setPos(newNodePos.x(), newNodePos.y(), false, true);
                }
            }
        }
    }
}

void NodeHandle::connectSignals()
{
    if(!m_isValid){
        return;
    }
    connect(m_node, SIGNAL(destroyed()), this, SLOT(nodeWasDestroyed()));
    connect(m_node, SIGNAL(nodeActivated()), this, SIGNAL(nodeActivated()));
    connect(m_node, SIGNAL(nodeRenamed(QString)), this, SIGNAL(nodeRenamed(QString)));
    connect(m_node, SIGNAL(removalRequested()), this, SIGNAL(removalRequested()));
    connect(m_node, SIGNAL(inputConnected(Plug*,Plug*)), this, SLOT(passInputConnected(Plug*,Plug*)));
    connect(m_node, SIGNAL(inputDisconnected(Plug*,Plug*)), this, SLOT(passInputDisconnected(Plug*,Plug*)));
    connect(m_node, SIGNAL(outputConnected(Plug*,Plug*)), this, SLOT(passOutputConnected(Plug*,Plug*)));
    connect(m_node, SIGNAL(outputDisconnected(Plug*,Plug*)), this, SLOT(passOutputDisconnected(Plug*,Plug*)));

    if(m_node->getType() == NODE_STORY)
    {
        StoryNode *sNode = static_cast<StoryNode*>(m_node);
        connect(sNode, SIGNAL(createStoryChild(zodiac::StoryNodeType, QString, QString, QPoint&)), this, SIGNAL(createStoryChild(zodiac::StoryNodeType, QString, QString, QPoint&)));
    }
    else
        if(m_node->getType() == NODE_NARRATIVE)
        {
            NarrativeNode *nNode = static_cast<NarrativeNode*>(m_node);
            connect(nNode, SIGNAL(openLinker()), this, SIGNAL(openLinker()));
        }
}

void NodeHandle::passInputConnected(Plug* myInput, Plug* otherOutput)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#endif
    emit inputConnected(PlugHandle(myInput), PlugHandle(otherOutput));
}

void NodeHandle::passOutputConnected(Plug* myOutput, Plug* otherInput)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#endif
    emit outputConnected(PlugHandle(myOutput), PlugHandle(otherInput));
}

void NodeHandle::passInputDisconnected(Plug* myInput, Plug* otherOutput)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#endif
    emit inputDisconnected(PlugHandle(myInput), PlugHandle(otherOutput));
}

void NodeHandle::passOutputDisconnected(Plug* myOutput, Plug* otherInput)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#endif
    emit outputDisconnected(PlugHandle(myOutput), PlugHandle(otherInput));
}

void NodeHandle::nodeWasDestroyed()
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#endif
    m_isValid = false;
    this->disconnect();
}


bool NodeHandle::isNodeDecorator()
{
#ifdef QT_DEBUG
    Q_ASSERT(m_isValid);
#else
    if(!m_isValid){
        return false;
    }
#endif
    return m_node->isNodeDecorator();
}

} // namespace zodiac
