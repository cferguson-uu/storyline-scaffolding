#ifndef UNDOEDITS_H
#define UNDOEDITS_H

#include <QUndoCommand>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>

#include "nodectrl.h"
#include "collapsible.h"
#include "graphstructures.h"

class NodeProperties;
class CommandRow;
enum CommandBlockTypes;

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

    void (NodeCtrl::*m_pNameChangeFunc) (const QString&);
};

class ParamEditCommand : public QUndoCommand
{
public:
    enum { Id = 2345 };

    ParamEditCommand(QLineEdit *textItem, const QString &oldText, const QString &cmdKey, const QString &paramKey, NodeCtrl* node, void (NodeCtrl::*paramChangeFunc)(const QString &, const QString &, const QString &),
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
    QString m_cmdKey;
    QString m_paramKey;
    void (NodeCtrl::*m_pParamChangeFunc) (const QString&, const QString&, const QString&);
};

class CommandEditCommand : public QUndoCommand
{
public:
    enum { Id = 3456 };

    CommandEditCommand(QComboBox *cmdItem, const QString &oldValue, NodeCtrl* node, void (NodeCtrl::*cmdDeleteFunc)(const QString&),
                       void (NodeCtrl::*CmdAddFunc)(const QString&, const QString&), void (NodeCtrl::*ParamAddFunc)(const QString&, const QString&, const QString&), NodeProperties *nodeProperties, void (NodeProperties::*deleteParams) (CommandRow *cmd),
                       void (NodeProperties::*addParams) (CommandBlockTypes, CommandRow*, const QString&), const QString &oldText,
                       CommandRow *cmd, CommandBlockTypes type, QHash<QString, zodiac::NodeCommand> (NodeCtrl::*getCmdTable)(), QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }

private:
    QComboBox *m_CmdItem;

    QString m_OldValue;
    QString m_NewValue;
    QString m_OldText;
    QString m_NewText;

    NodeCtrl* m_Node;

    CommandBlockTypes m_Type;
    CommandRow *m_pCmd;
    NodeProperties *m_pNodeProperties;

    QHash<QString, QString> m_OldParameters;
    QHash<QString, QString> m_NewParameters;

    void (NodeCtrl::*m_pCmdDeleteFunc) (const QString&);
    void (NodeCtrl::*m_pCmdAddFunc) (const QString&, const QString&);
    void (NodeCtrl::*m_pParamAddFunc) (const QString&, const QString&, const QString&);
    QHash<QString, zodiac::NodeCommand> (NodeCtrl::*m_pGetCmdTable)();

    void (NodeProperties::*m_pDeleteParams) (CommandRow *cmd);
    void (NodeProperties::*m_pAddParams) (CommandBlockTypes, CommandRow*, const QString&);


};

class CommandAddCommand : public QUndoCommand
{
public:
    enum { Id = 4567 };

    CommandAddCommand(QGridLayout *grid, QHash<QUuid, CommandRow*> *commandRow, CommandBlockTypes type, CommandRow* (NodeProperties::*addCommand) (QGridLayout*, QHash<QUuid, CommandRow*>&, CommandBlockTypes, zodiac::NodeCommand*),
                      NodeProperties *nodeProperties, QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    int id() const override { return Id; }

private:

    QGridLayout *m_pGrid;
    QHash<QUuid, CommandRow*> *m_pCommandRow;
    CommandBlockTypes m_type;
    NodeProperties *m_pNodeProperties;
    CommandRow* (NodeProperties::*m_pAddCommand) (QGridLayout*, QHash<QUuid, CommandRow*>&, CommandBlockTypes, zodiac::NodeCommand*);
    CommandRow *m_pCmd;
};

class CommandDeleteCommand : public QUndoCommand
{
public:
    enum { Id = 5678 };

    CommandDeleteCommand(QGridLayout *grid, QHash<QUuid, CommandRow*> *commandRow, CommandBlockTypes type, CommandRow* (NodeProperties::*addCommand) (QGridLayout*, QHash<QUuid, CommandRow*>&, CommandBlockTypes, zodiac::NodeCommand*),
                         NodeProperties *nodeProperties, CommandRow *cmd, const QString &value, const QString &text, void (CommandRow::*deleteParams)(), NodeCtrl *node, void (NodeCtrl::*cmdAddFunc) (const QString&, const QString&),
                         void (NodeCtrl::*paramAddFunc) (const QString&, const QString&, const QString&), QHash<QString, zodiac::NodeCommand> (NodeCtrl::*getCmdTable)(), QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    int id() const override { return Id; }

private:

    QGridLayout *m_pGrid;
    QHash<QUuid, CommandRow*> *m_pCommandRow;
    CommandBlockTypes m_type;
    //std::list<Command> *m_pCommands;
    NodeProperties *m_pNodeProperties;
    void (CommandRow::*m_pDeleteParams)();
    CommandRow* (NodeProperties::*m_pAddCommand) (QGridLayout*, QHash<QUuid, CommandRow*>&, CommandBlockTypes, zodiac::NodeCommand*);
    CommandRow *m_pCmd;

    QHash<QString, QString> m_SavedParameters;
    QString m_CommandValue;
    QString m_CommandText;

    NodeCtrl *m_pNode;
    void (NodeCtrl::*m_pCmdAddFunc) (const QString&, const QString&);
    void (NodeCtrl::*m_pParamAddFunc) (const QString&, const QString&, const QString&);
    QHash<QString, zodiac::NodeCommand> (NodeCtrl::*m_pGetCmdTable)();
};

#endif // UNDOEDITS_H
