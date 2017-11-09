#include "undoedits.h"
#include "nodeproperties.h"
#include "propertyeditor.h"

#include <QDebug>

TextEditCommand::TextEditCommand(bool isDescription, const QString &oldText, const QString &newText, NodeCtrl* node, void (NodeCtrl::*nameChangeFunc)(const QString &), Collapsible *collapsible,
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
    ,m_NewText(newText)
    ,m_OldText(oldText)
    ,m_isDescription(isDescription)
    ,m_pNameChangeFunc(nameChangeFunc)
    ,m_Node(node)
{
    if(collapsible)
    {
        m_Collapsible = collapsible;
        m_PropEdit = collapsible->getParent();
    }
}

void TextEditCommand::undo()
{
    //used to avoid incorrect memory access when a node is closed, this re-opens it
    m_Node->setSelected(true);
    m_Collapsible = m_PropEdit->getCollapsible(m_Node->getNodeHandle());

    //ensure the correct field is selected
    if(m_isDescription)
        m_TextItem = m_Collapsible->getNodeProperties()->getDescriptionEdit();
    else
    {
        m_TextItem = m_Collapsible->getNodeProperties()->getNameEdit();
        //update name in the property editor if necessary
        m_Collapsible->updateTitle(m_OldText);
    }

    m_TextItem->setText(m_OldText);
    (m_Node->*m_pNameChangeFunc)(m_OldText);
}

void TextEditCommand::redo()
{
    //used to avoid incorrect memory access when a node is closed, this re-opens it
    m_Node->setSelected(true);
    m_Collapsible = m_PropEdit->getCollapsible(m_Node->getNodeHandle());

    //ensure the correct field is selected
    if(m_isDescription)
        m_TextItem = m_Collapsible->getNodeProperties()->getDescriptionEdit();
    else
    {
        m_TextItem = m_Collapsible->getNodeProperties()->getNameEdit();
        //update name in the property editor if necessary
        m_Collapsible->updateTitle(m_OldText);
    }

    //store new text in the field (only has an effect on actual redo) and store in node
    m_TextItem->setText(m_NewText);
    (m_Node->*m_pNameChangeFunc)(m_NewText);
}

bool TextEditCommand::mergeWith(const QUndoCommand *command)
{
    const TextEditCommand *textEditCommand = static_cast<const TextEditCommand *>(command);
    QLineEdit *item = textEditCommand->m_TextItem;

    if (m_TextItem != item)
    return false;

    m_NewText = item->text();

    return true;
}

///

ParamEditCommand::ParamEditCommand(const QString &newText, const QString &oldText, const QUuid &cmdKey, const QString &paramKey, NodeCtrl* node, void (NodeCtrl::*paramChangeFunc)(const QUuid &, const QString &, const QString &),
                                 Collapsible *collapsible, CommandBlockTypes type, QUndoCommand *parent)
    : QUndoCommand(parent)
    ,m_NewText(newText)
    ,m_OldText(oldText)
    ,m_pParamChangeFunc(paramChangeFunc)
    ,m_Node(node)
    ,m_cmdKey(cmdKey)
    ,m_paramKey(paramKey)
    ,m_Collapsible(collapsible)
    ,m_PropEdit(collapsible->getParent())
    ,m_type(type)
{
}

void ParamEditCommand::undo()
{
    //used to avoid incorrect memory access when a node is closed, this re-opens it
    m_Node->setSelected(true);
    m_Collapsible = m_PropEdit->getCollapsible(m_Node->getNodeHandle());

    std::vector<std::pair<QLabel*,QLineEdit*>> paramsList;

    switch(m_type)
    {
        case CMD_UNLOCK:
            paramsList = m_Collapsible->getNodeProperties()->getOnUnlockCmds()[m_cmdKey]->getParams();

            for(std::vector<std::pair<QLabel*,QLineEdit*>>::iterator paramIt = paramsList.begin(); paramIt != paramsList.end(); ++paramIt)
            {
                //make sure label text matches parameter key
                QLabel *label = std::get<0>((*paramIt));

                if(label->text() == m_paramKey)
                {
                    m_TextItem = std::get<1>((*paramIt));
                    break;
                }
            }
            break;
        case CMD_FAIL:
            paramsList = m_Collapsible->getNodeProperties()->getOnFailCmds()[m_cmdKey]->getParams();

            for(std::vector<std::pair<QLabel*,QLineEdit*>>::iterator paramIt = paramsList.begin(); paramIt != paramsList.end(); ++paramIt)
            {
                //make sure label text matches parameter key
                QLabel *label = std::get<0>((*paramIt));

                if(label->text() == m_paramKey)
                {
                    m_TextItem = std::get<1>((*paramIt));
                    break;
                }
            }
        break;
        case CMD_UNLOCKED:
            paramsList = m_Collapsible->getNodeProperties()->getOnUnlockedCmds()[m_cmdKey]->getParams();

            for(std::vector<std::pair<QLabel*,QLineEdit*>>::iterator paramIt = paramsList.begin(); paramIt != paramsList.end(); ++paramIt)
            {
                //make sure label text matches parameter key
                QLabel *label = std::get<0>((*paramIt));

                if(label->text() == m_paramKey)
                {
                    m_TextItem = std::get<1>((*paramIt));
                    break;
                }
            }
            break;
    }

    //revert text in field and use command and parameter keys to store the parameter value in the node using function pointer
    m_TextItem->setText(m_OldText);

    (m_Node->*m_pParamChangeFunc)(m_cmdKey, m_paramKey, m_OldText);
}

void ParamEditCommand::redo()
{
    //used to avoid incorrect memory access when a node is closed, this re-opens it
    m_Node->setSelected(true);
    m_Collapsible = m_PropEdit->getCollapsible(m_Node->getNodeHandle());

    std::vector<std::pair<QLabel*,QLineEdit*>> paramsList;

    switch(m_type)
    {
        case CMD_UNLOCK:
            paramsList = m_Collapsible->getNodeProperties()->getOnUnlockCmds()[m_cmdKey]->getParams();

            for(std::vector<std::pair<QLabel*,QLineEdit*>>::iterator paramIt = paramsList.begin(); paramIt != paramsList.end(); ++paramIt)
            {
                //make sure label text matches parameter key
                QLabel *label = std::get<0>((*paramIt));

                if(label->text() == m_paramKey)
                {
                    m_TextItem = std::get<1>((*paramIt));
                    break;
                }
            }
            break;
        case CMD_FAIL:
            paramsList = m_Collapsible->getNodeProperties()->getOnFailCmds()[m_cmdKey]->getParams();

            for(std::vector<std::pair<QLabel*,QLineEdit*>>::iterator paramIt = paramsList.begin(); paramIt != paramsList.end(); ++paramIt)
            {
                //make sure label text matches parameter key
                QLabel *label = std::get<0>((*paramIt));

                if(label->text() == m_paramKey)
                {
                    m_TextItem = std::get<1>((*paramIt));
                    break;
                }
            }
        break;
        case CMD_UNLOCKED:
            paramsList = m_Collapsible->getNodeProperties()->getOnUnlockedCmds()[m_cmdKey]->getParams();

            for(std::vector<std::pair<QLabel*,QLineEdit*>>::iterator paramIt = paramsList.begin(); paramIt != paramsList.end(); ++paramIt)
            {
                //make sure label text matches parameter key
                QLabel *label = std::get<0>((*paramIt));

                if(label->text() == m_paramKey)
                {
                    m_TextItem = std::get<1>((*paramIt));
                    break;
                }
            }
            break;
    }

    //store new text in the field (only has an effect on actual redo) and
    //use command and parameter keys to store the parameter value in the node using function pointer
    m_TextItem->setText(m_NewText);

    (m_Node->*m_pParamChangeFunc)(m_cmdKey, m_paramKey, m_NewText);
}

bool ParamEditCommand::mergeWith(const QUndoCommand *command)
{
    const ParamEditCommand *paramEditCommand = static_cast<const ParamEditCommand *>(command);
    QLineEdit *item = paramEditCommand->m_TextItem;

    if (m_TextItem != item)
    return false;

    m_NewText = item->text();

    return true;
}

///

CommandEditCommand::CommandEditCommand(QComboBox *cmdItem, const QString &oldValue, const QString &oldText, const QString &newValue, const QString &newText, NodeCtrl* node, void (NodeCtrl::*cmdDeleteFunc)(const QUuid&),
                                       void (NodeCtrl::*CmdAddFunc)(const QUuid&, const QString&, const QString&), void (NodeCtrl::*ParamAddFunc)(const QUuid&, const QString&, const QString&), NodeProperties *nodeProperties, void (NodeProperties::*deleteParams) (CommandRow *cmd),
                                       void (NodeProperties::*addParams) (CommandBlockTypes, CommandRow*, const QUuid&),
                                       CommandRow *cmd, CommandBlockTypes type, QHash<QUuid, zodiac::NodeCommand> (NodeCtrl::*getCmdTable)(), QUuid uniqueIdentifier, Collapsible *collapsible, QUndoCommand *parent)
    : QUndoCommand(parent)
    //store command key and label value
    ,m_CmdItem(cmdItem)
    ,m_OldValue(oldValue)
    ,m_NewValue(newValue)
    ,m_OldText(oldText)
    ,m_NewText(newText)
    ,m_uniqueIdentifier(uniqueIdentifier)
    ,m_Type(type)
    ,m_Node(node)
    ,m_pCmd(cmd)
    ,m_pNodeProperties(nodeProperties)
    ,m_Collapsible(collapsible)
    ,m_PropEdit(collapsible->getParent())
    ,m_pCmdDeleteFunc(cmdDeleteFunc)
    ,m_pCmdAddFunc(CmdAddFunc)
    ,m_pParamAddFunc(ParamAddFunc)
    ,m_pAddParams(addParams)
    ,m_pDeleteParams(deleteParams)
    ,m_pGetCmdTable(getCmdTable)
{
    //store old parameters, needed for undo but should not be done every redo;
    m_OldParameters = (m_Node->*m_pGetCmdTable)()[m_OldValue].parameters;
}

void CommandEditCommand::undo()
{
    //used to avoid incorrect memory access when a node is closed, this re-opens it
    m_Node->setSelected(true);
    m_Collapsible = m_PropEdit->getCollapsible(m_Node->getNodeHandle());
    m_pNodeProperties = m_Collapsible->getNodeProperties();
    m_pCmd = m_pNodeProperties->getOnUnlockCmds()[m_uniqueIdentifier];
    m_CmdItem = m_pCmd->getCommandBox();

    //changing the value of the combo box triggers a new command, set bool to avoid
    m_pCmd->SetUndo(true);

    //set the new value and save the parameters of the previous command
    m_CmdItem->setCurrentText(m_OldText);
    m_NewParameters = (m_Node->*m_pGetCmdTable)()[m_uniqueIdentifier].parameters;

    //delete the previous command and add the new one
    (m_Node->*m_pCmdDeleteFunc)(m_NewValue);
    (m_Node->*m_pCmdAddFunc)(m_uniqueIdentifier, m_OldValue, m_OldText);

    //delete the old parameter fields from the layout, load the saved ones and add them to the layout
    (m_pNodeProperties->*m_pDeleteParams)(m_pCmd);

    for (QHash<QString, QString>::iterator i = m_OldParameters.begin(); i != m_OldParameters.end(); ++i)
        (m_Node->*m_pParamAddFunc)(m_OldValue, i.key(), i.value());

    (m_pNodeProperties->*m_pAddParams)(m_Type, m_pCmd, m_uniqueIdentifier);

    //set the name in the command
    m_pCmd->SetName(m_OldValue);
}

void CommandEditCommand::redo()
{
    //used to avoid incorrect memory access when a node is closed, this re-opens it
    m_Node->setSelected(true);
    m_Collapsible = m_PropEdit->getCollapsible(m_Node->getNodeHandle());
    m_pNodeProperties = m_Collapsible->getNodeProperties();
    m_pCmd = m_pNodeProperties->getOnUnlockCmds()[m_uniqueIdentifier];
    m_CmdItem = m_pCmd->getCommandBox();

    //only update the name if it is different (only on actual redo)
    if(m_NewText != m_CmdItem->currentText())
    {
        m_pCmd->SetUndo(true);  //as above, stop triggering command change function
        m_CmdItem->setCurrentText(m_NewText);
    }

    //save old parameters for undo
    m_OldParameters = (m_Node->*m_pGetCmdTable)()[m_uniqueIdentifier].parameters;

    //delete the previous command and add the new one
    (m_Node->*m_pCmdDeleteFunc)(m_OldValue);
    (m_Node->*m_pCmdAddFunc)(m_uniqueIdentifier, m_NewValue, m_NewText);

    //delete the old parameter fields from the layout, load the saved ones and add them to the layout
    (m_pNodeProperties->*m_pDeleteParams)(m_pCmd);

    for (QHash<QString, QString>::iterator i = m_NewParameters.begin(); i != m_NewParameters.end(); ++i)
        (m_Node->*m_pParamAddFunc)(m_NewValue, i.key(), i.value());

    (m_pNodeProperties->*m_pAddParams)(m_Type, m_pCmd, m_uniqueIdentifier);

    //set the name in the command
     m_pCmd->SetName(m_NewValue);
}

bool CommandEditCommand::mergeWith(const QUndoCommand *command)
{
    const CommandEditCommand *cmdEditCommand = static_cast<const CommandEditCommand *>(command);
    QComboBox *item = cmdEditCommand->m_CmdItem;

    if (m_CmdItem != item)
        return false;

    m_NewValue = item->itemData(item->currentIndex()).toString();

    return true;
}

///
CommandAddCommand:: CommandAddCommand(QGridLayout *grid, QHash<QUuid, CommandRow*> *commandRow, CommandBlockTypes type, CommandRow* (NodeProperties::*addCommand) (QGridLayout*, QHash<QUuid, CommandRow*>&, CommandBlockTypes, const QUuid&, zodiac::NodeCommand*),
                                      NodeProperties *nodeProperties, Collapsible *collapsible, NodeCtrl *node, QUndoCommand *parent)
    : QUndoCommand(parent)
    ,m_pGrid(grid)
    ,m_pCommandRow(commandRow)
    ,m_pNodeProperties(nodeProperties)
    ,m_pAddCommand(addCommand)
    ,m_type(type)
    ,m_pCmd(nullptr)
    ,m_Collapsible(collapsible)
    ,m_PropEdit(collapsible->getParent())
    ,m_Node(node)
{
}

void CommandAddCommand::undo()
{
    //used to avoid incorrect memory access when a node is closed, this re-opens it
    m_Node->setSelected(true);
    m_Collapsible = m_PropEdit->getCollapsible(m_Node->getNodeHandle());
    m_pNodeProperties = m_Collapsible->getNodeProperties();

    switch(m_type)
    {
        case CMD_UNLOCK:
            if(m_pNodeProperties->getOnUnlockCmds().contains(m_uniqueIdentifier))
                m_pCmd = m_pNodeProperties->getOnUnlockCmds()[m_uniqueIdentifier];
            break;
        case CMD_FAIL:
            if(m_pNodeProperties->getOnFailCmds().contains(m_uniqueIdentifier))
                m_pCmd = m_pNodeProperties->getOnFailCmds()[m_uniqueIdentifier];
            break;
        case CMD_UNLOCKED:
            if(m_pNodeProperties->getOnUnlockedCmds().contains(m_uniqueIdentifier))
                m_pCmd = m_pNodeProperties->getOnUnlockedCmds()[m_uniqueIdentifier];
            break;
    }

    if(m_pCmd != nullptr)
    {
        m_pCmd->removeCommand();
        m_pCmd = nullptr;
    }
}

void CommandAddCommand::redo()
{
    //used to avoid incorrect memory access when a node is closed, this re-opens it
    m_Node->setSelected(true);
    m_Collapsible = m_PropEdit->getCollapsible(m_Node->getNodeHandle());
    m_pNodeProperties = m_Collapsible->getNodeProperties();

    switch(m_type)
    {
        case CMD_UNLOCK:
            m_pGrid = m_pNodeProperties->getOnUnlockGrid();
            break;
        case CMD_FAIL:
            m_pGrid = m_pNodeProperties->getOnFailGrid();
            break;
        case CMD_UNLOCKED:
            m_pGrid = m_pNodeProperties->getOnUnlockedGrid();
            break;
    }

    m_pCmd = (m_pNodeProperties->*m_pAddCommand)(m_pGrid, *m_pCommandRow, m_type, {00000000-0000-0000-0000-000000000000}, nullptr);
    m_uniqueIdentifier = m_pCmd->getId();
}

///
CommandDeleteCommand::CommandDeleteCommand(QHash<QUuid, CommandRow*> *commandRow, CommandBlockTypes type, CommandRow* (NodeProperties::*addCommand) (QGridLayout*, QHash<QUuid, CommandRow*>&, CommandBlockTypes, const QUuid&, zodiac::NodeCommand*),
                                           NodeProperties *nodeProperties, CommandRow *cmd, const QString &value, const QString &text, void (CommandRow::*deleteParams)(), NodeCtrl *node, void (NodeCtrl::*cmdAddFunc) (const QUuid &, const QString&, const QString&),
                                           void (NodeCtrl::*paramAddFunc) (const QUuid&, const QString&, const QString&), QHash<QUuid, zodiac::NodeCommand> (NodeCtrl::*getCmdTable)(), QUuid uniqueIdentifier, Collapsible *collapsible, QUndoCommand *parent)
    : QUndoCommand(parent)
    ,m_pCommandRow(commandRow)
    ,m_pNodeProperties(nodeProperties)
    ,m_pAddCommand(addCommand)
    ,m_type(type)
    ,m_pCmd(cmd)
    ,m_CommandValue(value)
    ,m_CommandText(text)
    ,m_uniqueIdentifier(uniqueIdentifier)
    ,m_pDeleteParams(deleteParams)
    ,m_pNode(node)
    ,m_pCmdAddFunc(cmdAddFunc)
    ,m_pParamAddFunc(paramAddFunc)
    ,m_pGetCmdTable(getCmdTable)
    ,m_Collapsible(collapsible)
    ,m_PropEdit(collapsible->getParent())
{
}

void CommandDeleteCommand::undo()
{
    //used to avoid incorrect memory access when a node is closed, this re-opens it
    m_pNode->setSelected(true);
    m_Collapsible = m_PropEdit->getCollapsible(m_pNode->getNodeHandle());
    m_pNodeProperties = m_Collapsible->getNodeProperties();

    //add new command
    (m_pNode->*m_pCmdAddFunc)(m_uniqueIdentifier, m_CommandValue, m_CommandText);

    //delete the old parameter fields from the layout, load the saved ones and add them to the layout
    //(m_pCmd->*m_pDeleteParams)();

    for (QHash<QString, QString>::iterator i = m_SavedParameters.begin(); i != m_SavedParameters.end(); ++i)
        (m_pNode->*m_pParamAddFunc)(m_CommandValue, i.key(), i.value());

    switch(m_type)
    {
        case CMD_UNLOCK:
            m_pGrid = m_pNodeProperties->getOnUnlockGrid();
            m_pCommandRow = &m_pNodeProperties->getOnUnlockCmds();
            break;
        case CMD_FAIL:
            m_pGrid = m_pNodeProperties->getOnFailGrid();
            m_pCommandRow = &m_pNodeProperties->getOnFailCmds();
            break;
        case CMD_UNLOCKED:
            m_pGrid = m_pNodeProperties->getOnUnlockedGrid();
            m_pCommandRow = &m_pNodeProperties->getOnUnlockedCmds();
            break;
    }

    m_pCmd = (m_pNodeProperties->*m_pAddCommand)(m_pGrid, *m_pCommandRow, m_type, m_uniqueIdentifier, &(m_pNode->*m_pGetCmdTable)()[m_CommandValue]);
}

void CommandDeleteCommand::redo()
{
    //used to avoid incorrect memory access when a node is closed, this re-opens it
    m_pNode->setSelected(true);
    m_Collapsible = m_PropEdit->getCollapsible(m_pNode->getNodeHandle());
    m_pNodeProperties = m_Collapsible->getNodeProperties();

    switch(m_type)
    {
        case CMD_UNLOCK:
            m_pCmd = m_pNodeProperties->getOnUnlockCmds()[m_uniqueIdentifier];
            break;
        case CMD_FAIL:
            m_pCmd = m_pNodeProperties->getOnFailCmds()[m_uniqueIdentifier];
            break;
        case CMD_UNLOCKED:
            m_pCmd = m_pNodeProperties->getOnUnlockedCmds()[m_uniqueIdentifier];
            break;
    }

    //save parameters
    m_SavedParameters = (m_pNode->*m_pGetCmdTable)()[m_CommandValue].parameters;
    m_pCmd->removeCommand();
}

///
NodeRemoveLink::NodeRemoveLink(zodiac::Plug *outgoingPlug, zodiac::Plug *incomingPlug, PlugRow* row, void (PlugRow::*removePlugConnection)(QPair<QLabel*, QPushButton*> &), QPair<QLabel*, QPushButton*> uiElements,  QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_outgoingPlug(outgoingPlug)
    , m_incomingPlug(incomingPlug)
    , m_pRow(row)
    , m_pRemovePlugConnection(removePlugConnection)
    , m_uiElements(uiElements)
{

}

void NodeRemoveLink::undo()
{
    m_outgoingPlug.connectPlug(m_incomingPlug);
}

void NodeRemoveLink::redo()
{
    m_outgoingPlug.disconnectPlug(m_incomingPlug);

    if(m_pRow)
        (m_pRow->*m_pRemovePlugConnection)(m_uiElements);
}

///
NodeAddLink::NodeAddLink(zodiac::PlugHandle &outgoingPlug, zodiac::PlugHandle &incomingPlug, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_outgoingPlug(outgoingPlug)
    , m_incomingPlug(incomingPlug)
{

}

void NodeAddLink::undo()
{
    m_outgoingPlug.disconnectPlug(m_incomingPlug);
}

void NodeAddLink::redo()
{
    m_outgoingPlug.connectPlug(m_incomingPlug);
}

///
NodeAddLinks::NodeAddLinks(zodiac::PlugHandle &outgoingPlug, QList<zodiac::PlugHandle> &incomingPlugs, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_outgoingPlug(outgoingPlug)
    , m_incomingPlugs(incomingPlugs)
{

}

void NodeAddLinks::undo()
{
    for(QList<zodiac::PlugHandle>::iterator plugIt = m_incomingPlugs.begin(); plugIt != m_incomingPlugs.end(); ++plugIt)
        m_outgoingPlug.disconnectPlug((*plugIt));
}

void NodeAddLinks::redo()
{
    for(QList<zodiac::PlugHandle>::iterator plugIt = m_incomingPlugs.begin(); plugIt != m_incomingPlugs.end(); ++plugIt)
    m_outgoingPlug.connectPlug((*plugIt));
}
