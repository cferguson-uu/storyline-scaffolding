#include "undoedits.h"
#include "nodeproperties.h"

#include <QDebug>

undoedits::undoedits()
{

}

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

    (m_Node->*m_pNameChangeFunc)(m_NewText);

    if(propEdit)
        m_PropEdit->updateTitle(m_NewText);
}

void TextEditCommand::undo()
{
    m_TextItem->setText(m_OldText);

    (m_Node->*m_pNameChangeFunc)(m_OldText);

    if(m_PropEdit)
        m_PropEdit->updateTitle(m_OldText);

    //setText(QObject::tr("Edit %1").arg(createCommandString(m_TextItem, m_OldText)));
}

void TextEditCommand::redo()
{
    m_TextItem->setText(m_NewText);

    (m_Node->*m_pNameChangeFunc)(m_NewText);
    if(m_PropEdit)
        m_PropEdit->updateTitle(m_NewText);

    //setText(QObject::tr("Edit %1").arg(createCommandString(m_TextItem, m_NewText)));
}

bool TextEditCommand::mergeWith(const QUndoCommand *command)
{
    const TextEditCommand *textEditCommand = static_cast<const TextEditCommand *>(command);
    QLineEdit *item = textEditCommand->m_TextItem;

    if (m_TextItem != item)
    return false;

    m_NewText = item->text();

    //setText(QObject::tr("Edit %1").arg(createCommandString(m_TextItem, m_NewText)));

    return true;
}

///

ParamEditCommand::ParamEditCommand(QLineEdit *textItem, const QString &oldText, const QString &cmdKey, const QString &paramKey, NodeCtrl* node, void (NodeCtrl::*paramChangeFunc)(const QString &, const QString &, const QString &),
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
{
    m_TextItem = textItem;
    m_NewText = textItem->text();
    m_OldText = oldText;
    m_pParamChangeFunc = paramChangeFunc;
    m_Node = node;
    m_cmdKey = cmdKey;
    m_paramKey = paramKey;

    (m_Node->*m_pParamChangeFunc)(m_cmdKey, m_paramKey ,m_NewText);
}

void ParamEditCommand::undo()
{
    m_TextItem->setText(m_OldText);

    (m_Node->*m_pParamChangeFunc)(m_cmdKey, m_paramKey, m_OldText);

    //setText(QObject::tr("Edit %1").arg(createCommandString(m_TextItem, m_OldText)));
}

void ParamEditCommand::redo()
{
    m_TextItem->setText(m_NewText);

    (m_Node->*m_pParamChangeFunc)(m_cmdKey, m_paramKey ,m_NewText);

    //setText(QObject::tr("Edit %1").arg(createCommandString(m_TextItem, m_NewText)));
}

bool ParamEditCommand::mergeWith(const QUndoCommand *command)
{
    const ParamEditCommand *paramEditCommand = static_cast<const ParamEditCommand *>(command);
    QLineEdit *item = paramEditCommand->m_TextItem;

    if (m_TextItem != item)
    return false;

    m_NewText = item->text();

    //setText(QObject::tr("Edit %1").arg(createCommandString(m_TextItem, m_NewText)));

    return true;
}

CommandEditCommand::CommandEditCommand(QComboBox *cmdItem, const QString &oldValue, NodeCtrl* node, void (NodeCtrl::*cmdDeleteFunc)(const QString&),
                                       void (NodeCtrl::*CmdAddFunc)(const QString&, const QString&), NodeProperties *nodeProperties, void (NodeProperties::*deleteParams) (CommandRow *cmd),
                                       void (NodeProperties::*addParams) (CommandBlockTypes, CommandRow *, const QString&), const QString &oldText,
                                       CommandRow *cmd, CommandBlockTypes type, QHash<QString, zodiac::NodeCommand> (NodeCtrl::*getCmdTable)(), QUndoCommand *parent)
    : QUndoCommand(parent)
{
    m_CmdItem = cmdItem;
    m_OldValue = oldValue;
    m_NewValue = cmdItem->itemData(cmdItem->currentIndex()).toString();
    m_OldText = oldText;
    m_NewText = cmdItem->currentText();

    m_Node = node;

    m_Type = type;
    m_pCmd = cmd;
    m_pNodeProperties = nodeProperties;

    m_pCmdDeleteFunc = cmdDeleteFunc;
    m_pCmdAddFunc = CmdAddFunc;
    m_pAddParams = addParams;
    m_pDeleteParams = deleteParams;

    m_pGetCmdTable = getCmdTable;

    m_OldParameters = (m_Node->*m_pGetCmdTable)()[m_OldValue].parameters;

    m_CurrentValue = m_NewValue;
    m_CurrentText = m_NewText;
}

void CommandEditCommand::undo()
{
    m_pCmd->SetUndo(true);

    m_CmdItem->setCurrentText(m_OldText);

    (m_Node->*m_pCmdDeleteFunc)(m_OldValue);
    (m_Node->*m_pCmdAddFunc)(m_OldValue, m_OldText);

    (m_pNodeProperties->*m_pDeleteParams)(m_pCmd);
    (m_Node->*m_pGetCmdTable)()[m_OldValue].parameters = m_OldParameters;
    (m_pNodeProperties->*m_pAddParams)(m_Type, m_pCmd, m_OldValue);

    m_pCmd->SetName(m_OldValue);
}

void CommandEditCommand::redo()
{
    qDebug() << "old " << m_OldValue;
    qDebug() << "new " << m_NewValue;

    if(m_NewText != m_CmdItem->currentText())
    {
        m_pCmd->SetUndo(true);
        m_CmdItem->setCurrentText(m_NewText);
    }

    (m_Node->*m_pCmdDeleteFunc)(m_OldValue);
    (m_Node->*m_pCmdAddFunc)(m_NewValue, m_NewText);

    (m_pNodeProperties->*m_pDeleteParams)(m_pCmd);

    (m_pNodeProperties->*m_pAddParams)(m_Type, m_pCmd, m_NewValue);

     m_NewParameters = (m_Node->*m_pGetCmdTable)()[m_NewValue].parameters;

     m_pCmd->SetName(m_NewValue);

    /*

    (m_Node->*m_pGetCmdTable)()[m_NewValue].parameters = m_NewParameters;
    (m_pNodeProperties->*m_pAddParams)(m_Type, m_pCmd, m_OldValue);*/
}

bool CommandEditCommand::mergeWith(const QUndoCommand *command)
{
    const CommandEditCommand *cmdEditCommand = static_cast<const CommandEditCommand *>(command);
    QComboBox *item = cmdEditCommand->m_CmdItem;

    if (m_CmdItem != item)
        return false;

    m_NewValue = item->itemData(item->currentIndex()).toString();

    //setText(QObject::tr("Edit %1").arg(createCommandString(m_TextItem, m_NewText)));

    return true;
}
