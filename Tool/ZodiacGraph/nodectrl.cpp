#include "nodectrl.h"

#include "mainctrl.h"

typedef zodiac::PlugHandle PlugHandle;

NodeCtrl::NodeCtrl(MainCtrl* manager,  zodiac::NodeHandle node)
    : QObject(manager)
    , m_manager(manager)
    , m_node(node)
    , m_plugs(QHash<PlugHandle, QList<PlugHandle>>())
{
    // connect node signals
    connect(&m_node, SIGNAL(removalRequested()), this, SLOT(remove()));
    connect(&m_node, SIGNAL(inputConnected(zodiac::PlugHandle, zodiac::PlugHandle)),
            this, SLOT(inputConnected(zodiac::PlugHandle, zodiac::PlugHandle)));
    connect(&m_node, SIGNAL(outputConnected(zodiac::PlugHandle, zodiac::PlugHandle)),
            this, SLOT(outputConnected(zodiac::PlugHandle, zodiac::PlugHandle)));
    connect(&m_node, SIGNAL(inputDisconnected(zodiac::PlugHandle, zodiac::PlugHandle)),
            this, SLOT(inputDisconnected(zodiac::PlugHandle, zodiac::PlugHandle)));
    connect(&m_node, SIGNAL(outputDisconnected(zodiac::PlugHandle, zodiac::PlugHandle)),
            this, SLOT(outputDisconnected(zodiac::PlugHandle, zodiac::PlugHandle)));

    connect(&m_node, SIGNAL(createStoryChild(zodiac::StoryNodeType, QString, QPoint&)),
            this, SLOT(createStoryChild(zodiac::StoryNodeType, QString, QPoint&)));
}

void NodeCtrl::rename(const QString& name)
{
    m_node.rename(name);
}

void NodeCtrl::changeDescription(const QString& description)
{
    m_node.changeDescription(description);
}

zodiac::NodeType NodeCtrl::getType() const
{
    return m_node.getType();
}

QPointF NodeCtrl::getPos() const
{
    return m_node.getPos();
}

void NodeCtrl::setPos(qreal x, qreal y)
{
    m_node.setPos(x, y);
}

QHash<QUuid, zodiac::NodeCommand> NodeCtrl::getOnUnlockList()
{
    return m_node.getOnUnlockList();
}

QHash<QUuid, zodiac::NodeCommand> NodeCtrl::getOnFailList()
{
    return m_node.getOnFailList();
}

QHash<QUuid, zodiac::NodeCommand> NodeCtrl::getOnUnlockedList()
{
    return m_node.getOnUnlockedList();
}

void NodeCtrl::addOnUnlockCommand(const QUuid& key, const QString& value, const QString& description)
{
    m_node.addOnUnlockCommand(key, value, description);
}

void NodeCtrl::addOnFailCommand(const QUuid& key, const QString& value, const QString& description)
{
    m_node.addOnFailCommand(key, value, description);
}

void NodeCtrl::addOnUnlockedCommand(const QUuid& key, const QString& value, const QString& description)
{
     m_node.addOnUnlockedCommand(key, value, description);
}

void NodeCtrl::removeOnUnlockCommand(const QUuid& key)
{
    m_node.removeOnUnlockCommand(key);
}
void NodeCtrl::removeOnFailCommand(const QUuid& key)
{
    m_node.removeOnFailCommand(key);
}

void NodeCtrl::removeOnUnlockedCommand(const QUuid& key)
{
    m_node.removeOnUnlockedCommand(key);
}

QString NodeCtrl::getParameterFromOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey)
{
    return m_node.getParameterFromOnUnlockCommand(cmdKey, paramKey);
}

QString NodeCtrl::getParameterFromOnFailCommand(const QUuid& cmdKey, const QString& paramKey)
{
    return m_node.getParameterFromOnFailCommand(cmdKey, paramKey);
}

QString NodeCtrl::getParameterFromOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey)
{
    return m_node.getParameterFromOnUnlockedCommand(cmdKey, paramKey);
}

void NodeCtrl::addParameterToOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_node.addParameterToOnUnlockCommand(cmdKey, paramKey, value);
}

void NodeCtrl::addParameterToOnFailCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_node.addParameterToOnFailCommand(cmdKey, paramKey, value);
}

void NodeCtrl::addParameterToOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_node.addParameterToOnUnlockedCommand(cmdKey, paramKey, value);
}

void NodeCtrl::removeParameterFromOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey)
{
    m_node.removeParameterFromOnUnlockCommand(cmdKey, paramKey);
}

void NodeCtrl::removeParameterFromOnFailCommand(const QUuid& cmdKey, const QString& paramKey)
{
    m_node.removeParameterFromOnFailCommand(cmdKey, paramKey);
}

void NodeCtrl::removeParameterFromOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey)
{
    m_node.removeParameterFromOnUnlockedCommand(cmdKey, paramKey);
}

void NodeCtrl::removeAllParametersFromOnUnlockCommand(const QUuid& cmdKey)
{
    m_node.removeAllParametersFromOnUnlockCommand(cmdKey);
}

void NodeCtrl::removeAllParametersFromOnFailCommand(const QUuid& cmdKey)
{
    m_node.removeAllParametersFromOnFailCommand(cmdKey);
}

void NodeCtrl::removeAllParametersFromOnUnlockedCommand(const QUuid& cmdKey)
{
    m_node.removeAllParametersFromOnUnlockedCommand(cmdKey);
}

void NodeCtrl::editParameterInOnUnlockCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_node.editParameterInOnUnlockCommand(cmdKey, paramKey, value);
}

void NodeCtrl::editParameterInOnFailCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_node.editParameterInOnFailCommand(cmdKey, paramKey, value);
}

void NodeCtrl::editParameterInOnUnlockedCommand(const QUuid& cmdKey, const QString& paramKey, const QString& value)
{
    m_node.editParameterInOnUnlockedCommand(cmdKey, paramKey, value);
}

QString NodeCtrl::renamePlug(const QString& oldName, const QString& newName)
{
    if(newName == oldName){
        return oldName;
    }

    PlugHandle plug = m_node.getPlug(oldName);
    if(!plug.isValid()){
        // return the empty string if there is no plug by the given name
        return "";
    }

    // disconnect all connected plugs
    if(plug.isIncoming()){
        for(PlugHandle otherPlug : m_plugs.value(plug)){
            m_manager->getCtrlForHandle(otherPlug.getNode())->outputDisconnected(otherPlug, plug);
         }
    } else {
        for(PlugHandle otherPlug : m_plugs.value(plug)){
            m_manager->getCtrlForHandle(otherPlug.getNode())->inputDisconnected(otherPlug, plug);
        }
    }

    // rename the plug
    QString actualName = plug.rename(newName);

    // reconnect other plugs
    if(plug.isIncoming()){
        for(PlugHandle otherPlug : m_plugs.value(plug)){
            m_manager->getCtrlForHandle(otherPlug.getNode())->outputConnected(otherPlug, plug);
        }
    } else {
        for(PlugHandle otherPlug : m_plugs.value(plug)){
            m_manager->getCtrlForHandle(otherPlug.getNode())->inputConnected(otherPlug, plug);
        }
    }

    return actualName;
}

bool NodeCtrl::togglePlugDirection(const QString& name)
{
    PlugHandle plug = m_node.getPlug(name);
    if(!plug.isValid() || !plug.toggleDirection()){
        return false;
    }
    Q_ASSERT(m_plugs.contains(plug));
    Q_ASSERT(m_plugs[plug].size() == 0);

    return true;
}

bool NodeCtrl::removePlug(const QString& name)
{
    // get the plug that is about to be removed
    PlugHandle plug = m_node.getPlug(name);
    if(!plug.isValid() || !plug.isRemovable()){
        return false;
    }

    // remove all references to the plug
    Q_ASSERT(m_plugs.contains(plug));
    Q_ASSERT(m_plugs[plug].size() == 0);
    m_plugs.remove(plug);

    // remove the plug
    bool result = plug.remove();
    Q_ASSERT(result);
    return result;
}

void NodeCtrl::setSelected(bool isSelected)
{
    m_node.setSelected(isSelected);
}

bool NodeCtrl::remove()
{
    return m_manager->deleteNode(this);
}

void NodeCtrl::createStoryChild(zodiac::StoryNodeType type, QString name, QString description, QPoint &relativePos)
{
    m_manager->createStoryNode(this, type, name, description, relativePos);
}

zodiac::PlugHandle NodeCtrl::addPlug(const QString& name, bool incoming)
{
    PlugHandle newPlug;
    if(incoming){
        newPlug = m_node.createIncomingPlug(name);
    } else {
        newPlug = m_node.createOutgoingPlug(name);
    }
    Q_ASSERT(newPlug.isValid());
    m_plugs.insert(newPlug, QList<PlugHandle>());
    return newPlug;
}

void NodeCtrl::inputConnected(PlugHandle myInput, PlugHandle otherOutput)
{
    m_plugs[myInput].append(otherOutput);
}

void NodeCtrl::outputConnected(PlugHandle myOutput, PlugHandle otherInput)
{
    m_plugs[myOutput].append(otherInput);
}

void NodeCtrl::inputDisconnected(PlugHandle myInput, PlugHandle otherOutput)
{
    m_plugs[myInput].removeOne(otherOutput);
    Q_ASSERT(m_plugs[myInput].count(otherOutput) == 0);
}

void NodeCtrl::outputDisconnected(PlugHandle myOutput, PlugHandle otherInput)
{
    m_plugs[myOutput].removeOne(otherInput);
    Q_ASSERT(m_plugs[myOutput].count(otherInput) == 0);
}
