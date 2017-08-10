#ifndef UNDOEDITS_H
#define UNDOEDITS_H

#include <QUndoCommand>
#include <QLineEdit>
#include "nodectrl.h"
#include "collapsible.h"

class undoedits
{
public:
    undoedits();
};

class TextEditCommand : public QUndoCommand
{
public:
    enum { Id = 1234 };

    TextEditCommand(QLineEdit *textItem, const QString &oldText, NodeCtrl* node, void (NodeCtrl::*nameChangeFunc)(const QString &), Collapsible *propEdit = 0,
                QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }

private:
    QLineEdit *m_TextItem;
    QString m_OldText;
    QString m_NewText;
    NodeCtrl *m_Node;
    Collapsible *m_PropEdit = nullptr;

    //void (*m_pNameChangeFunc)(const QString &);
    void (NodeCtrl::*m_pNameChangeFunc) (const QString&);
};

#endif // UNDOEDITS_H
