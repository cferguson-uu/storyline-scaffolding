#include "undoedits.h"

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
