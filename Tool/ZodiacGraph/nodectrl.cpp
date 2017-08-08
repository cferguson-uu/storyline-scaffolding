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
}

void NodeCtrl::rename(const QString& name)
{
    m_node.rename(name);
}

void NodeCtrl::changeDescription(const QString& description)
{
    m_node.changeDescription(description);
}

QHash<QString, zodiac::NodeCommand> NodeCtrl::getOnUnlockList()
{
    return m_node.getOnUnlockList();
}

QHash<QString, zodiac::NodeCommand> NodeCtrl::getOnFailList()
{
    return m_node.getOnFailList();
}

QHash<QString, zodiac::NodeCommand> NodeCtrl::getOnUnlockedList()
{
    return m_node.getOnUnlockedList();
}

void NodeCtrl::addOnUnlockCommand(const QString& key)
{
    m_node.addOnUnlockCommand(key);
}

void NodeCtrl::addOnFailCommand(const QString& key)
{
    m_node.addOnFailCommand(key);
}

void NodeCtrl::addOnUnlockedCommand(const QString& key)
{
     m_node.addOnUnlockedCommand(key);
}

void NodeCtrl::removeOnUnlockCommand(const QString& key)
{
    m_node.removeOnUnlockCommand(key);
}
void NodeCtrl::removeOnFailCommand(const QString& key)
{
    m_node.removeOnFailCommand(key);
}

void NodeCtrl::removeOnUnlockedCommand(const QString& key)
{
    m_node.removeOnUnlockedCommand(key);
}

void NodeCtrl::addParameterToOnUnlockCommand(const QString& cmdKey, const QString& paramKey, const QString& value)
{
    m_node.addParameterToOnUnlockCommand(cmdKey, paramKey, value);
}

void NodeCtrl::addParameterToOnFailCommand(const QString& cmdKey, const QString& paramKey, const QString& value)
{
    m_node.addParameterToOnFailCommand(cmdKey, paramKey, value);
}

void NodeCtrl::addParameterToOnUnlockedCommand(const QString& cmdKey, const QString& paramKey, const QString& value)
{
    m_node.addParameterToOnUnlockedCommand(cmdKey, paramKey, value);
}

void NodeCtrl::removeParameterFromOnUnlockCommand(const QString& cmdKey, const QString& paramKey)
{
    m_node.removeParameterFromOnUnlockCommand(cmdKey, paramKey);
}

void NodeCtrl::removeParameterFromOnFailCommand(const QString& cmdKey, const QString& paramKey)
{
    m_node.removeParameterFromOnFailCommand(cmdKey, paramKey);
}

void NodeCtrl::removeParameterFromOnUnlockedCommand(const QString& cmdKey, const QString& paramKey)
{
    m_node.removeParameterFromOnUnlockedCommand(cmdKey, paramKey);
}

void NodeCtrl::removeAllParametersFromOnUnlockCommand(const QString& cmdKey)
{
    m_node.removeAllParametersFromOnUnlockCommand(cmdKey);
}

void NodeCtrl::removeAllParametersFromOnFailCommand(const QString& cmdKey)
{
    m_node.removeAllParametersFromOnFailCommand(cmdKey);
}

void NodeCtrl::removeAllParametersFromOnUnlockedCommand(const QString& cmdKey)
{
    m_node.removeAllParametersFromOnUnlockedCommand(cmdKey);
}

void NodeCtrl::editParameterInOnUnlockCommand(const QString& cmdKey, const QString& paramKey, const QString& value)
{
    m_node.editParameterInOnUnlockCommand(cmdKey, paramKey, value);
}

void NodeCtrl::editParameterInOnFailCommand(const QString& cmdKey, const QString& paramKey, const QString& value)
{
    m_node.editParameterInOnFailCommand(cmdKey, paramKey, value);
}

void NodeCtrl::editParameterInOnUnlockedCommand(const QString& cmdKey, const QString& paramKey, const QString& value)
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
