#include "undoedits.h"
#include "nodeproperties.h"

#include <QDebug>

TextEditCommand::TextEditCommand(QLineEdit *textItem, const QString &oldText, NodeCtrl* node, void (NodeCtrl::*nameChangeFunc)(const QString &), Collapsible *propEdit,
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
{
    m_TextItem = textItem;
    m_NewText = textItem->text();
    m_OldText = oldText;
    m_PropEdit = propEdit;
    m_pNameChangeFunc = nameChangeFunc;
    m_Node = node;
}

void TextEditCommand::undo()
{
    //revert to old text in the field and store in node
    m_TextItem->setText(m_OldText);
    (m_Node->*m_pNameChangeFunc)(m_OldText);

    //update name in the property editor if necessary
    if(m_PropEdit)
        m_PropEdit->updateTitle(m_OldText);
}

void TextEditCommand::redo()
{
    //store new text in the field (only has an effect on actual redo) and store in node
    m_TextItem->setText(m_NewText);
    (m_Node->*m_pNameChangeFunc)(m_NewText);

    //update name in the property editor if necessary
    if(m_PropEdit)
        m_PropEdit->updateTitle(m_NewText);
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

ParamEditCommand::ParamEditCommand(QLineEdit *textItem, const QString &oldText, const QUuid &cmdKey, const QString &paramKey, NodeCtrl* node, void (NodeCtrl::*paramChangeFunc)(const QUuid &, const QString &, const QString &),
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
{
    m_TextItem = textItem;
    m_NewText = textItem->text();
    m_OldText = oldText;
    m_pParamChangeFunc = paramChangeFunc;   //different for each command block so pointer needed
    m_Node = node;
    m_cmdKey = cmdKey;
    m_paramKey = paramKey;
}

void ParamEditCommand::undo()
{
    //revert text in field and use command and parameter keys to store the parameter value in the node using function pointer
    m_TextItem->setText(m_OldText);

    (m_Node->*m_pParamChangeFunc)(m_cmdKey, m_paramKey, m_OldText);
}

void ParamEditCommand::redo()
{
    //store new text in the field (only has an effect on actual redo) and
    //use command and parameter keys to store the parameter value in the node using function pointer
    m_TextItem->setText(m_NewText);

    (m_Node->*m_pParamChangeFunc)(m_cmdKey, m_paramKey ,m_NewText);
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

CommandEditCommand::CommandEditCommand(QComboBox *cmdItem, const QString &oldValue, NodeCtrl* node, void (NodeCtrl::*cmdDeleteFunc)(const QUuid&),
                                       void (NodeCtrl::*CmdAddFunc)(const QUuid&, const QString&, const QString&), void (NodeCtrl::*ParamAddFunc)(const QUuid&, const QString&, const QString&), NodeProperties *nodeProperties, void (NodeProperties::*deleteParams) (CommandRow *cmd),
                                       void (NodeProperties::*addParams) (CommandBlockTypes, CommandRow*, const QUuid&), const QString &oldText,
                                       CommandRow *cmd, CommandBlockTypes type, QHash<QUuid, zodiac::NodeCommand> (NodeCtrl::*getCmdTable)(), QUuid uniqueIdentifier, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    //store command key and label value
    m_CmdItem = cmdItem;
    m_OldValue = oldValue;
    m_NewValue = cmdItem->itemData(cmdItem->currentIndex()).toString();
    m_OldText = oldText;
    m_NewText = cmdItem->currentText();
    m_uniqueIdentifier = uniqueIdentifier,

    m_Type = type;

    //store pointers to instances needed to carry out update operations
    m_Node = node;
    m_pCmd = cmd;
    m_pNodeProperties = nodeProperties;

    //function pointers
    m_pCmdDeleteFunc = cmdDeleteFunc;
    m_pCmdAddFunc = CmdAddFunc;
    m_pParamAddFunc = ParamAddFunc;
    m_pAddParams = addParams;
    m_pDeleteParams = deleteParams;
    m_pGetCmdTable = getCmdTable;

    //store old parameters, needed for undo but should not be done every redo;
    m_OldParameters = (m_Node->*m_pGetCmdTable)()[m_OldValue].parameters;
}

void CommandEditCommand::undo()
{
    //changing the value of the combo box triggers a new command, set bool to avoid
    m_pCmd->SetUndo(true);

    //set the new value and save the parameters of the previous command
    m_CmdItem->setCurrentText(m_OldText);
    m_NewParameters = (m_Node->*m_pGetCmdTable)()[m_NewValue].parameters;

    //delete the previous command and add the new one
    (m_Node->*m_pCmdDeleteFunc)(m_NewValue);
    (m_Node->*m_pCmdAddFunc)(m_uniqueIdentifier, m_OldValue, m_OldText);

    //delete the old parameter fields from the layout, load the saved ones and add them to the layout
    (m_pNodeProperties->*m_pDeleteParams)(m_pCmd);

    for (QHash<QString, QString>::iterator i = m_OldParameters.begin(); i != m_OldParameters.end(); ++i)
        (m_Node->*m_pParamAddFunc)(m_OldValue, i.key(), i.value());

    (m_pNodeProperties->*m_pAddParams)(m_Type, m_pCmd, m_OldValue);

    //set the name in the command
    m_pCmd->SetName(m_OldValue);
}

void CommandEditCommand::redo()
{
    //only update the name if it is different (only on actual redo)
    if(m_NewText != m_CmdItem->currentText())
    {
        m_pCmd->SetUndo(true);  //as above, stop triggering command change function
        m_CmdItem->setCurrentText(m_NewText);
    }

    //save old parameters for undo
    m_OldParameters = (m_Node->*m_pGetCmdTable)()[m_OldValue].parameters;

    //delete the previous command and add the new one
    (m_Node->*m_pCmdDeleteFunc)(m_OldValue);
    (m_Node->*m_pCmdAddFunc)(m_uniqueIdentifier, m_NewValue, m_NewText);

    //delete the old parameter fields from the layout, load the saved ones and add them to the layout
    (m_pNodeProperties->*m_pDeleteParams)(m_pCmd);

    for (QHash<QString, QString>::iterator i = m_NewParameters.begin(); i != m_NewParameters.end(); ++i)
        (m_Node->*m_pParamAddFunc)(m_NewValue, i.key(), i.value());

    (m_pNodeProperties->*m_pAddParams)(m_Type, m_pCmd, m_NewValue);

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
                                      NodeProperties *nodeProperties,  QUndoCommand *parent)
    : QUndoCommand(parent)
{

    m_pGrid = grid;
    m_pCommandRow = commandRow;

    m_pNodeProperties = nodeProperties;
    m_pAddCommand = addCommand;
    m_type = type;
    m_pCmd = nullptr;
}

void CommandAddCommand::undo()
{
    if(m_pCmd != nullptr)
        m_pCmd->removeCommand();
}

void CommandAddCommand::redo()
{
     m_pCmd = (m_pNodeProperties->*m_pAddCommand)(m_pGrid, *m_pCommandRow, m_type, {00000000-0000-0000-0000-000000000000}, nullptr);
}

///
CommandDeleteCommand::CommandDeleteCommand(QGridLayout *grid, QHash<QUuid, CommandRow*> *commandRow, CommandBlockTypes type, CommandRow* (NodeProperties::*addCommand) (QGridLayout*, QHash<QUuid, CommandRow*>&, CommandBlockTypes, const QUuid&, zodiac::NodeCommand*),
                                           NodeProperties *nodeProperties, CommandRow *cmd, const QString &value, const QString &text, void (CommandRow::*deleteParams)(), NodeCtrl *node, void (NodeCtrl::*cmdAddFunc) (const QUuid &, const QString&, const QString&),
                                           void (NodeCtrl::*paramAddFunc) (const QUuid&, const QString&, const QString&), QHash<QUuid, zodiac::NodeCommand> (NodeCtrl::*getCmdTable)(), QUuid uniqueIdentifier, QUndoCommand *parent)
    : QUndoCommand(parent)
{

    m_pGrid = grid;
    m_pCommandRow = commandRow;
    m_pNodeProperties = nodeProperties;
    m_pAddCommand = addCommand;
    m_type = type;
    m_pCmd = cmd;  

   m_CommandValue = value;
   m_CommandText = text;
   m_uniqueIdentifier = uniqueIdentifier;

   m_pDeleteParams = deleteParams;

   m_pNode = node;
   m_pCmdAddFunc = cmdAddFunc;
   m_pParamAddFunc = paramAddFunc;
   m_pGetCmdTable = getCmdTable;
}

void CommandDeleteCommand::undo()
{
    //add new command
    (m_pNode->*m_pCmdAddFunc)(m_uniqueIdentifier, m_CommandValue, m_CommandText);

    //delete the old parameter fields from the layout, load the saved ones and add them to the layout
    //(m_pCmd->*m_pDeleteParams)();

    for (QHash<QString, QString>::iterator i = m_SavedParameters.begin(); i != m_SavedParameters.end(); ++i)
        (m_pNode->*m_pParamAddFunc)(m_CommandValue, i.key(), i.value());

    m_pCmd = (m_pNodeProperties->*m_pAddCommand)(m_pGrid, *m_pCommandRow, m_type, m_uniqueIdentifier, &(m_pNode->*m_pGetCmdTable)()[m_CommandValue]);
}

void CommandDeleteCommand::redo()
{
    //save parameters
    m_SavedParameters = (m_pNode->*m_pGetCmdTable)()[m_CommandValue].parameters;
    m_pCmd->removeCommand();
}

