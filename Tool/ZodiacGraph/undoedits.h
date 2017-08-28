#ifndef UNDOEDITS_H
#define UNDOEDITS_H

#include <QUndoCommand>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

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

    TextEditCommand(bool isDescription, const QString &oldText, const QString &newText, NodeCtrl* node, void (NodeCtrl::*nameChangeFunc)(const QString &), Collapsible *collapsible,
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
    Collapsible *m_Collapsible = nullptr;
    PropertyEditor *m_PropEdit = nullptr;
    bool m_isDescription;

    void (NodeCtrl::*m_pNameChangeFunc) (const QString&);
};

class ParamEditCommand : public QUndoCommand
{
public:
    enum { Id = 2345 };

    ParamEditCommand(const QString &newText, const QString &oldText, const QUuid &cmdKey, const QString &paramKey, NodeCtrl* node, void (NodeCtrl::*paramChangeFunc)(const QUuid &, const QString &, const QString &),
                Collapsible *collapsible, CommandBlockTypes type, QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }

private:
    QLineEdit *m_TextItem;
    QString m_OldText;
    QString m_NewText;
    NodeCtrl *m_Node;
    QUuid m_cmdKey;
    QString m_paramKey;
    void (NodeCtrl::*m_pParamChangeFunc) (const QUuid&, const QString&, const QString&);
    Collapsible *m_Collapsible = nullptr;
    PropertyEditor *m_PropEdit = nullptr;
    CommandBlockTypes m_type;
};

class CommandEditCommand : public QUndoCommand
{
public:
    enum { Id = 3456 };

    CommandEditCommand(QComboBox *cmdItem, const QString &oldValue, const QString &oldText, const QString &newValue, const QString &newText, NodeCtrl* node, void (NodeCtrl::*cmdDeleteFunc)(const QUuid&),
                       void (NodeCtrl::*CmdAddFunc)(const QUuid&, const QString&, const QString&), void (NodeCtrl::*ParamAddFunc)(const QUuid&, const QString&, const QString&),
                       NodeProperties *nodeProperties, void (NodeProperties::*deleteParams) (CommandRow *cmd),
                       void (NodeProperties::*addParams) (CommandBlockTypes, CommandRow*, const QUuid&),
                       CommandRow *cmd, CommandBlockTypes type, QHash<QUuid, zodiac::NodeCommand> (NodeCtrl::*getCmdTable)(), QUuid uniqueIdentifier, Collapsible *collapsible, QUndoCommand *parent = 0);

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

    void (NodeCtrl::*m_pCmdDeleteFunc) (const QUuid&);
    void (NodeCtrl::*m_pCmdAddFunc) (const QUuid&, const QString&, const QString&);
    void (NodeCtrl::*m_pParamAddFunc) (const QUuid&, const QString&, const QString&);
    QHash<QUuid, zodiac::NodeCommand> (NodeCtrl::*m_pGetCmdTable)();

    void (NodeProperties::*m_pDeleteParams) (CommandRow *cmd);
    void (NodeProperties::*m_pAddParams) (CommandBlockTypes, CommandRow*, const QUuid&);

    QUuid m_uniqueIdentifier;

    Collapsible *m_Collapsible = nullptr;
    PropertyEditor *m_PropEdit = nullptr;


};

class CommandAddCommand : public QUndoCommand
{
public:
    enum { Id = 4567 };

    CommandAddCommand(QGridLayout *grid, QHash<QUuid, CommandRow*> *commandRow, CommandBlockTypes type, CommandRow* (NodeProperties::*addCommand) (QGridLayout*, QHash<QUuid, CommandRow*>&, CommandBlockTypes, const QUuid&, zodiac::NodeCommand*),
                      NodeProperties *nodeProperties, Collapsible *collapsible, NodeCtrl *node, QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    int id() const override { return Id; }

private:

    QGridLayout *m_pGrid;
    QHash<QUuid, CommandRow*> *m_pCommandRow;
    CommandBlockTypes m_type;
    NodeProperties *m_pNodeProperties;
    CommandRow* (NodeProperties::*m_pAddCommand) (QGridLayout*, QHash<QUuid, CommandRow*>&, CommandBlockTypes, const QUuid&, zodiac::NodeCommand*);
    CommandRow *m_pCmd;

    Collapsible *m_Collapsible = nullptr;
    PropertyEditor *m_PropEdit = nullptr;
    NodeCtrl *m_Node;

    QUuid m_uniqueIdentifier;
};

class CommandDeleteCommand : public QUndoCommand
{
public:
    enum { Id = 5678 };

    CommandDeleteCommand(QHash<QUuid, CommandRow*> *commandRow, CommandBlockTypes type, CommandRow* (NodeProperties::*addCommand) (QGridLayout*, QHash<QUuid, CommandRow*>&, CommandBlockTypes, const QUuid&, zodiac::NodeCommand*),
                         NodeProperties *nodeProperties, CommandRow *cmd, const QString &value, const QString &text, void (CommandRow::*deleteParams)(), NodeCtrl *node, void (NodeCtrl::*cmdAddFunc) (const QUuid &, const QString&, const QString&),
                         void (NodeCtrl::*paramAddFunc) (const QUuid&, const QString&, const QString&), QHash<QUuid, zodiac::NodeCommand> (NodeCtrl::*getCmdTable)(), QUuid uniqueIdentifier, Collapsible *collapsible, QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    int id() const override { return Id; }

private:

    QGridLayout *m_pGrid;
    QHash<QUuid, CommandRow*> *m_pCommandRow;
    CommandBlockTypes m_type;
    NodeProperties *m_pNodeProperties;
    void (CommandRow::*m_pDeleteParams)();
    CommandRow* (NodeProperties::*m_pAddCommand) (QGridLayout*, QHash<QUuid, CommandRow*>&, CommandBlockTypes, const QUuid&, zodiac::NodeCommand*);
    CommandRow *m_pCmd;

    QHash<QString, QString> m_SavedParameters;
    QString m_CommandValue;
    QString m_CommandText;

    NodeCtrl *m_pNode;
    void (NodeCtrl::*m_pCmdAddFunc) (const QUuid &, const QString&, const QString&);
    void (NodeCtrl::*m_pParamAddFunc) (const QUuid&, const QString&, const QString&);
    QHash<QUuid, zodiac::NodeCommand> (NodeCtrl::*m_pGetCmdTable)();

    QUuid m_uniqueIdentifier;
    Collapsible *m_Collapsible = nullptr;
    PropertyEditor *m_PropEdit = nullptr;
};

#endif // UNDOEDITS_H
