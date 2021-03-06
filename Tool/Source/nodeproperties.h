//
//    ZodiacGraph - A general-purpose, circular node graph UI module.
//    Copyright (C) 2015  Clemens Sielaff
//
//    The MIT License
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy of
//    this software and associated documentation files (the "Software"), to deal in
//    the Software without restriction, including without limitation the rights to
//    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
//    of the Software, and to permit persons to whom the Software is furnished to do so,
//    subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
//

#ifndef NODEPROPERTIES_H
#define NODEPROPERTIES_H

#include <QHash>
#include <QWidget>
#include <QComboBox>
#include <QUndoStack>
#include <QUuid>

#include "zodiacgraph/nodehandle.h"
#include "zodiacgraph/plughandle.h"

#include "saveandload.h"
#include "undoedits.h"

class QGridLayout;
class QLineEdit;
class QPushButton;

class Collapsible;
class NodeCtrl;
class PlugRow;
class CommandRow;

enum CommandBlockTypes
{
    CMD_UNLOCK,
    CMD_FAIL,
    CMD_UNLOCKED
};

///
/// \brief Node Property widget, is a display widget of a Collapsible.
///
class NodeProperties : public QWidget
{
    Q_OBJECT

    ///
    /// \brief The PlugRow class is a logical part of the this one, but has a 1-n relationship.
    ///
    friend class PlugRow;

    friend class CommandRow;

public: // methods

    ///
    /// \brief Constructor.
    ///
    /// \param [in] node    Node whose properties to display.
    /// \param [in] parent  Collapsible parent object.
    /// \param [in] undoStack       Undo stack for undo and redo.
    ///
    explicit NodeProperties(NodeCtrl* node, Collapsible *parent, QList<Command> *commands, QUndoStack *undoStack);

    QLineEdit *getNameEdit(){return m_nameEdit;}
    QLineEdit *getDescriptionEdit(){return m_descriptionEdit;}
    QHash<QUuid, CommandRow*> getOnUnlockCmds(){return m_onUnlockRows;}
    QHash<QUuid, CommandRow*> getOnFailCmds(){return m_onFailRows;}
    QHash<QUuid, CommandRow*> getOnUnlockedCmds(){return m_onUnlockedRows;}
    QGridLayout *getOnUnlockGrid(){return m_onUnlockLayout;}
    QGridLayout *getOnFailGrid(){return m_onFailLayout;}
    QGridLayout *getOnUnlockedGrid(){return m_onUnlockedLayout;}
    Collapsible* getParent(){return qobject_cast<Collapsible*>(parent());}

public: // static

    static void setAnalyticsMode(bool analyticsMode){s_analyticsMode = analyticsMode;}


private: // for friend

    ///
    /// \brief The controller of the node whose properties are displayed in this NodeProperties widget.
    ///
    /// \return Controller of the managed node.
    ///
    NodeCtrl* getNode() const {return m_node;}

    ///
    /// \brief The layout to be used by PlugRows to place their widgets.
    ///
    /// \return The layout of the widgets related to the plugs of the node.
    ///
    QGridLayout* getPlugLayout() const {return m_plugLayout;}

    ///
    /// \brief Removes a plug from the node and the PlugRow from the editor.
    ///
    /// \param [in] plugName    Name of the plug to remove.
    ///
    void removePlugRow(const QString& plugName);

    //same for command block
    void removeCommandRow(const QUuid& commandId, QHash<QUuid, CommandRow*> *commandRows);

    QUndoStack *getUndoStack(){return m_pUndoStack;}

private slots:

    //for creating blocks for onunlock etc.
    void constructNarrativeNodeProperties(QVBoxLayout* mainLayout);
    CommandRow *createNewCommandBlock(QGridLayout *grid, QHash<QUuid, CommandRow*> &commandRow, CommandBlockTypes type, const QUuid &id = {00000000-0000-0000-0000-000000000000}, zodiac::NodeCommand *cmd = nullptr);

    void AddParametersToCommand(CommandBlockTypes type, CommandRow *cmd, const QUuid &cmdKey);
    void DeleteParametersFromCommand(CommandRow *cmd);

    void updateParam(CommandBlockTypes type, const QUuid &cmdKey, const QString &paramKey, QLineEdit* paramField);

    //story
    void constructStoryNodeProperties(QVBoxLayout* mainLayout);

    ///
    /// \brief Called by the name edit, when the name of the node was changed through user input.
    ///
    void renameNode();

    ///
    /// \brief Called by the description edit, when the description of the node was changed through user input.
    ///
    void changeNodeDescription();

    ///
    /// \brief Called by the filename edit, when the description of the node was changed through user input.
    ///
    void changeNodeFileName();

    ///
    /// \brief Creates a new entry in the plug list of this property editor alongside its PlugRow.
    ///
    void addPlugRow(zodiac::PlugHandle plug);

    //for changing parameters along with command
    void changeCommand(QComboBox *commandField, CommandBlockTypes type, CommandRow *cmd, const QUuid &uniqueId);

private: // members

    ///
    /// \brief Controller of the edited node.
    ///
    NodeCtrl* m_node;

    ///
    /// \brief Node name edit.
    ///
    QLineEdit* m_nameEdit;

    ///
    /// \brief Node description edit.
    ///
    QLineEdit* m_descriptionEdit;

    ///
    /// \brief Node filename edit.
    ///
    QLineEdit* m_fileNameEdit;

    ///
    /// \brief Layout of the widgets related to the plugs of the node.
    ///
    QGridLayout* m_plugLayout;

    ///
    /// \brief All PlugRows contained in this editor.
    ///
    QHash<QString, PlugRow*> m_plugRows;

    QGridLayout* m_onUnlockLayout;
    QPushButton* m_addOnUnlockButton;
    QHash<QUuid, CommandRow*> m_onUnlockRows;

    QGridLayout* m_onFailLayout;
    QPushButton* m_addOnFailButton;
    QHash<QUuid, CommandRow*> m_onFailRows;

    QGridLayout* m_onUnlockedLayout;
    QPushButton* m_addOnUnlockedButton;
    QHash<QUuid, CommandRow*> m_onUnlockedRows;

    QList<Command> *m_pCommands;

    //pointer to carry out undo functions
    QUndoStack *m_pUndoStack;

private: // static members

    ///
    /// \brief Default plug name.
    ///
    static QString s_defaultPlugName;

    ///
    /// \brief Analytics mode to determine if anything is editable
    ///
    static bool s_analyticsMode;

};


///
/// \brief An extension to the NodeProperties class, responsible to manage a single row of Plug-related widgets.
///
class PlugRow : public QObject
{
    Q_OBJECT

public: // methods

    ///
    /// \brief Constructor.
    ///
    /// \param [in] editor          NodeProperties that this PlugRow is part of.
    /// \param [in] plug            Handle of the plug whose name to edit / display.
    /// \param [in] nameLabel        Plug name label.
    ///
    PlugRow(NodeProperties *editor, zodiac::PlugHandle plug, QLabel *nameLabel, QGridLayout *rowLayout);

    ///
    /// \brief remove a plug connection from the panel.
    ///
    /// \param [in] connection          QPair consisting of the label and button.
    ///
    void removePlugConnection(QPair<QLabel*, QPushButton*> &connection);

private slots:

    ///
    /// \brief Called when the name of the plug was changed through user input.
    ///
    void renamePlug();

private: // methods

private: // members

    ///
    /// \brief Controller of the edited node.
    ///
    NodeProperties* m_editor;

    ///
    /// \brief Handle of the plug whose name to edit / display.
    ///
    zodiac::PlugHandle m_plug;

    ///
    /// \brief Plug name edit.
    ///
    QLabel* m_nameLabel;

    ///
    /// \brief List of story connections
    ///
    QList<QPair<QLabel*, QPushButton*>> m_storyConnections;

    ///
    /// \brief List of narrative connections
    ///
    QList<QPair<QLabel*, QPushButton*>> m_narrativeConnections;
};

class CommandRow : public QObject
{
    Q_OBJECT

public: // methods

    ///
    /// \brief Constructor.
    ///
    /// \param [in] editor          NodeProperties that this CommandRow is part of.
    /// \param [in] nameEdit        Plug name edit.
    /// \param [in] removalButton   Plug-removal button.
    ///
    CommandRow(NodeProperties *editor, QComboBox *nameEdit, QPushButton *removalButton, QString &name,
               QHash<QUuid, CommandRow*> *commandRows, QUuid &uniqueId, QGridLayout* blockLayout, QGridLayout* commandLayout, CommandBlockTypes type);

    void addParameterToList(QLabel *label, QLineEdit *text);
    void addParameterToGrid(QLabel *label, QLineEdit *text);
    void DeleteParameters();

    QString GetName(){return m_commandName;}
    void SetName(const QString &name){m_commandName = name;}

    bool GetUndo(){return m_undo;}
    void SetUndo(bool undo){m_undo = undo;}

    QUuid getId(){return m_identifier;}

    QComboBox *getCommandBox(){return m_nameEdit;}

    std::vector<std::pair<QLabel*,QLineEdit*>> getParams(){return m_params;}

public slots:

    ///
    /// \brief Called when the command-removal button is pressed.
    ///
    void removeCommand();

private: // members

    ///
    /// \brief Controller of the edited node.
    ///
    NodeProperties* m_editor;

    ///
    /// \brief Command name edit.
    ///
    QComboBox* m_nameEdit;

    ///
    /// \brief command-removal button.
    ///
    QPushButton* m_removalButton;

    //save the name
    QString m_commandName;

    QHash<QUuid, CommandRow*> *m_rowPointer;

    QGridLayout *m_commandLayout;

    QGridLayout *m_blockLayout;

    std::vector<std::pair<QLabel*,QLineEdit*>> m_params;

    bool m_undo = false;

    QUuid m_identifier;

    //on unlock, fail or unlocked
    CommandBlockTypes m_commandType;

};

#endif // NODEPROPERTIES_H
