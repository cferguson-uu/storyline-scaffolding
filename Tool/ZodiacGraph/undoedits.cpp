#include "undoedits.h"

#include <QDebug>

undoedits::undoedits()
{

}

TextEditCommand::TextEditCommand(QLineEdit *textItem, const QString &oldText, NodeCtrl* node, Collapsible *propEdit,
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
{
    m_TextItem = textItem;
    m_NewText = textItem->text();
    m_OldText = oldText;
    m_Node = node;
    m_PropEdit = propEdit;

    m_Node->rename(m_NewText);
    m_PropEdit->updateTitle(m_NewText);

    qDebug() << "old = " + m_OldText;
    qDebug() << "new = " + m_NewText;
}

void TextEditCommand::undo()
{
    m_TextItem->setText(m_OldText);

    m_Node->rename(m_OldText);
    m_PropEdit->updateTitle(m_OldText);

    //setText(QObject::tr("Edit %1").arg(createCommandString(m_TextItem, m_OldText)));
}

void TextEditCommand::redo()
{
    m_TextItem->setText(m_NewText);

    m_Node->rename(m_NewText);
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
