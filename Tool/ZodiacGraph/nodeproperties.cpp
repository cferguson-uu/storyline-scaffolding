#include "nodeproperties.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>

#include "collapsible.h"
#include "nodectrl.h"

QString NodeProperties::s_defaultPlugName = "plug";

NodeProperties::NodeProperties(NodeCtrl *node, Collapsible *parent, std::list<Command> *commands, QUndoStack *undoStack)
    : QWidget(parent)
    , m_node(node)
    , m_nextPlugIsIncoming(true)
    , m_pCommands(commands)
    , m_pUndoStack(undoStack)
{
    // define the main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2,2,2,2);
    mainLayout->setSpacing(2);
    setLayout(mainLayout);

    // update the title of the collapsible container
    parent->updateTitle(m_node->getName());

    // define the name edit
    QHBoxLayout* nameLayout = new QHBoxLayout();
    m_nameEdit = new QLineEdit(m_node->getName(), this);
    connect(m_nameEdit, SIGNAL(editingFinished()), this, SLOT(renameNode()));
    nameLayout->addWidget(new QLabel("Name", this));
    nameLayout->addWidget(m_nameEdit);    
    nameLayout->setContentsMargins(0, 4, 0, 0);
    mainLayout->addLayout(nameLayout);

    QHBoxLayout* descriptionLayout = new QHBoxLayout();
    m_descriptionEdit = new QLineEdit(m_node->getDescription(), this);
    connect(m_descriptionEdit, SIGNAL(editingFinished()), this, SLOT(changeNodeDescription()));
    descriptionLayout->addWidget(new QLabel("Description", this));
    descriptionLayout->addWidget(m_descriptionEdit);
    descriptionLayout->setContentsMargins(0, 4, 0, 0);
    mainLayout->addLayout(descriptionLayout);


    if(m_node->getType() == zodiac::NODE_NARRATIVE)
        constructNarrativeNodeProperties(mainLayout);

    // define the plugs
    for(zodiac::PlugHandle& plug : m_node->getPlugHandles()){
        addPlugRow(plug);
    }
    mainLayout->addLayout(m_plugLayout);
}

void NodeProperties::constructNarrativeNodeProperties(QVBoxLayout* mainLayout)
{
    // define the on_unlock block
    m_onUnlockLayout = new QGridLayout();
    m_onUnlockLayout->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
    m_onUnlockLayout->setColumnStretch(1,1); // so the add-plug button always stays on the far right
    m_addOnUnlockButton = new QPushButton(this);
    m_addOnUnlockButton->setIconSize(QSize(8, 8));
    m_addOnUnlockButton->setIcon(QIcon(":/icons/plus.svg"));
    m_addOnUnlockButton->setFlat(true);
    m_onUnlockLayout->addWidget(new QLabel("OnUnlock", this), 0, 0, 1, 2, Qt::AlignLeft);
    m_onUnlockLayout->addWidget(m_addOnUnlockButton, 0, 2);
    //connect(m_addOnUnlockButton, &QPushButton::released, [=]{createNewCommandBlock(m_onUnlockLayout, m_onUnlockRows, CMD_UNLOCK);});
    connect(m_addOnUnlockButton, &QPushButton::released, [=]{m_pUndoStack->push(new CommandAddCommand(m_onUnlockLayout, &m_onUnlockRows, CMD_UNLOCK, &NodeProperties::createNewCommandBlock, this));});

    if(!m_node->getOnUnlockList().empty())
    {
        QHash<QString,zodiac::NodeCommand> *hashPointer = &m_node->getOnUnlockList();
        //need to load command list
        for(QHash<QString,zodiac::NodeCommand>::iterator cmdIt = hashPointer->begin(); cmdIt != hashPointer->end(); ++cmdIt)
            createNewCommandBlock(m_onUnlockLayout, m_onUnlockRows, CMD_UNLOCK, &(*cmdIt));
    }

    // define the on_fail block
    m_onFailLayout = new QGridLayout();
    m_onFailLayout->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
    m_onFailLayout->setColumnStretch(1,1); // so the add-plug button always stays on the far right
    m_addOnFailButton = new QPushButton(this);
    m_addOnFailButton->setIconSize(QSize(8, 8));
    m_addOnFailButton->setIcon(QIcon(":/icons/plus.svg"));
    m_addOnFailButton->setFlat(true);
    m_onFailLayout->addWidget(new QLabel("OnFail", this), 0, 0, 1, 2, Qt::AlignLeft);
    m_onFailLayout->addWidget(m_addOnFailButton, 0, 2);
    //connect(m_addOnFailButton, &QPushButton::released, [=]{createNewCommandBlock(m_onFailLayout, m_onFailRows, CMD_FAIL);});
     connect(m_addOnFailButton, &QPushButton::released, [=]{m_pUndoStack->push(new CommandAddCommand(m_onFailLayout, &m_onFailRows, CMD_FAIL, &NodeProperties::createNewCommandBlock, this));});

    if(!m_node->getOnFailList().empty())
    {
        QHash<QString,zodiac::NodeCommand> *hashPointer = &m_node->getOnFailList();
        //need to load command list
        for(QHash<QString,zodiac::NodeCommand>::iterator cmdIt = hashPointer->begin(); cmdIt != hashPointer->end(); ++cmdIt)
            createNewCommandBlock(m_onFailLayout, m_onFailRows, CMD_FAIL, &(*cmdIt));
    }

    // define the on_unlocked block
    m_onUnlockedLayout = new QGridLayout();
    m_onUnlockedLayout->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
    m_onUnlockedLayout->setColumnStretch(1,1); // so the add-plug button always stays on the far right
    m_addOnUnlockedButton = new QPushButton(this);
    m_addOnUnlockedButton->setIconSize(QSize(8, 8));
    m_addOnUnlockedButton->setIcon(QIcon(":/icons/plus.svg"));
    m_addOnUnlockedButton->setFlat(true);
    m_onUnlockedLayout->addWidget(new QLabel("OnUnlocked", this), 0, 0, 1, 2, Qt::AlignLeft);
    m_onUnlockedLayout->addWidget(m_addOnUnlockedButton, 0, 2);
    //connect(m_addOnUnlockedButton, &QPushButton::released, [=]{createNewCommandBlock(m_onUnlockedLayout, m_onUnlockedRows, CMD_UNLOCKED);});
    connect(m_addOnUnlockedButton, &QPushButton::released, [=]{m_pUndoStack->push(new CommandAddCommand(m_onUnlockedLayout, &m_onUnlockedRows, CMD_UNLOCKED, &NodeProperties::createNewCommandBlock, this));});

    if(!m_node->getOnUnlockedList().empty())
    {
        QHash<QString,zodiac::NodeCommand> *hashPointer = &m_node->getOnUnlockedList();
        //need to load command list
        for(QHash<QString,zodiac::NodeCommand>::iterator cmdIt = hashPointer->begin(); cmdIt != hashPointer->end(); ++cmdIt)
            createNewCommandBlock(m_onUnlockedLayout, m_onUnlockedRows, CMD_UNLOCKED, &(*cmdIt));
    }

    // define the add plug button
    m_plugLayout = new QGridLayout();
    m_plugLayout->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
    m_plugLayout->setColumnStretch(1,1); // so the add-plug button always stays on the far right
    m_addPlugButton = new QPushButton(this);
    m_addPlugButton->setIconSize(QSize(8, 8));
    m_addPlugButton->setIcon(QIcon(":/icons/plus.svg"));
    m_addPlugButton->setFlat(true);
    m_plugLayout->addWidget(new QLabel("Plugs", this), 0, 0, 1, 2, Qt::AlignLeft);
    m_plugLayout->addWidget(m_addPlugButton, 0, 2);
    connect(m_addPlugButton, SIGNAL(pressed()), this, SLOT(createNewPlug()));

    mainLayout->addLayout(m_onUnlockLayout);
    mainLayout->addLayout(m_onFailLayout);
    mainLayout->addLayout(m_onUnlockedLayout);
}

void NodeProperties::renameNode()
{
    QString newName = m_nameEdit->text();
    if(m_node->getName() == newName)
    {
        return;
    }

    m_pUndoStack->push(new TextEditCommand(m_nameEdit, m_node->getName(), m_node, &NodeCtrl::rename, qobject_cast<Collapsible*>(parent())));
}

void NodeProperties::changeNodeDescription()
{
    QString newDescription = m_descriptionEdit->text();
    if(m_node->getDescription() == newDescription){
        return;
    }

    m_pUndoStack->push(new TextEditCommand(m_descriptionEdit, m_node->getDescription(), m_node, &NodeCtrl::changeDescription));
}

CommandRow *NodeProperties::createNewCommandBlock(QGridLayout *grid, QHash<QUuid, CommandRow*> &commandRow, CommandBlockTypes type, zodiac::NodeCommand *cmd)
{
    int row = grid->rowCount();

    QGridLayout *commandBlockGrid = new QGridLayout();

    QComboBox* commandBox = new QComboBox();

    for(std::list<Command>::iterator it = m_pCommands->begin(); it != m_pCommands->end(); ++it)
        commandBox->addItem((*it).label, (*it).id);

    if(cmd == nullptr)
    {
        switch(type)
        {
            case CMD_UNLOCK:
                m_node->addOnUnlockCommand(commandBox->itemData(commandBox->currentIndex()).toString(), commandBox->currentText());
                break;
            case CMD_FAIL:
                m_node->addOnFailCommand(commandBox->itemData(commandBox->currentIndex()).toString(), commandBox->currentText());
                break;
            case CMD_UNLOCKED:
                m_node->addOnUnlockedCommand(commandBox->itemData(commandBox->currentIndex()).toString(), commandBox->currentText());
                break;
        }
    }
    else
        commandBox->setCurrentText(cmd->description);

    commandBlockGrid->addWidget(commandBox);

    QPushButton* removalButton = new QPushButton(this);
    removalButton->setIcon(QIcon(":/icons/minus.svg"));
    removalButton->setIconSize(QSize(8, 8));
    removalButton->setFlat(true);
    removalButton->setStatusTip("Delete the Plug from its Node");
    commandBlockGrid->addWidget(removalButton, 0, 1);

    grid->addLayout(commandBlockGrid, row, 0);

    QUuid u = QUuid::createUuid();

    commandRow.insert(u, new CommandRow(this, commandBox, removalButton, commandBox->itemData(commandBox->currentIndex()).toString(), commandRow, u, grid, commandBlockGrid));

    AddParametersToCommand(type, commandRow[u], commandBox->itemData(commandBox->currentIndex()).toString());
    connect(commandBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=] { changeCommand(commandBox, type, commandRow[u]); });

    return commandRow[u];
}

void NodeProperties::AddParametersToCommand(CommandBlockTypes type, CommandRow *cmd, const QString &cmdKey)
{
    bool load;

    switch(type)
    {
        case CMD_UNLOCK:
            load = m_node->getOnUnlockList()[cmdKey].parameters.empty();
            break;
        case CMD_FAIL:
            load = m_node->getOnFailList()[cmdKey].parameters.empty();
            break;
        case CMD_UNLOCKED:
            load = m_node->getOnUnlockedList()[cmdKey].parameters.empty();
            break;
    }

    //get parameters from the command pointer
    for(std::list<Command>::iterator cmdIt = m_pCommands->begin(); cmdIt != m_pCommands->end(); ++cmdIt)
        if((*cmdIt).id == cmdKey)
            for(std::list<Parameter>::iterator paramIt = (*cmdIt).commandParams.begin(); paramIt != (*cmdIt).commandParams.end(); ++paramIt)
            {
                //create fields and labels
                QLabel *label = new QLabel((*paramIt).label);
                QLineEdit *text;

                //load params if possible
                if(load)
                    text = new QLineEdit();
                else
                    switch(type)
                    {
                        case CMD_UNLOCK:
                            text = new QLineEdit(m_node->getOnUnlockList()[cmdKey].parameters[(*paramIt).label]);
                            break;
                        case CMD_FAIL:
                        text = new QLineEdit(m_node->getOnFailList()[cmdKey].parameters[(*paramIt).label]);
                            break;
                        case CMD_UNLOCKED:
                        text = new QLineEdit(m_node->getOnUnlockedList()[cmdKey].parameters[(*paramIt).label]);
                            break;
                    }

                //make fields auto-update the stored info in the node
                connect(text, &QLineEdit::editingFinished, [=]{updateParam(type, cmdKey, (*paramIt).label, text); });

                //add fields and labels to the correct grid
                cmd->addParameterToGrid(label, text);
                //store fields and labels as part of the command row
                cmd->addParameterToList(label, text);
                if(load)
                    //add parameters to node
                    switch(type)
                    {
                        case CMD_UNLOCK:
                            m_node->addParameterToOnUnlockCommand(cmdKey, (*paramIt).label, "");
                            break;
                        case CMD_FAIL:
                            m_node->addParameterToOnFailCommand(cmdKey, (*paramIt).label, "");
                            break;
                        case CMD_UNLOCKED:
                            m_node->addParameterToOnUnlockedCommand(cmdKey, (*paramIt).label, "");
                            break;
                    }
            }
}

void NodeProperties::updateParam(CommandBlockTypes type, const QString &cmdKey, const QString &paramKey, QLineEdit* paramField)
{ 
    QString newName;
    QString oldName;

    switch(type)
    {
        case CMD_UNLOCK:
            newName = paramField->text();
            oldName = m_node->getParameterFromOnUnlockCommand(cmdKey, paramKey);
            if(oldName == newName)
                return;
            m_pUndoStack->push(new ParamEditCommand(paramField, oldName, cmdKey, paramKey, m_node, &NodeCtrl::editParameterInOnUnlockCommand));
            break;
        case CMD_FAIL:
            newName = paramField->text();
            oldName = m_node->getParameterFromOnFailCommand(cmdKey, paramKey);
            if(oldName == newName)
                return;
            m_pUndoStack->push(new ParamEditCommand(paramField, oldName, cmdKey, paramKey, m_node, &NodeCtrl::editParameterInOnFailCommand));
            break;
        case CMD_UNLOCKED:
            newName = paramField->text();
            oldName = m_node->getParameterFromOnUnlockedCommand(cmdKey, paramKey);
            if(oldName == newName)
                return;
            m_pUndoStack->push(new ParamEditCommand(paramField, oldName, cmdKey, paramKey, m_node, &NodeCtrl::editParameterInOnUnlockedCommand));
            break;
    }
}

void NodeProperties::DeleteParametersFromCommand(CommandRow *cmd)
{
    cmd->DeleteParameters();
}

void NodeProperties::changeCommand(QComboBox *commandField, CommandBlockTypes type, CommandRow *cmd)
{
    if(cmd->GetUndo())
    {
        cmd->SetUndo(false);
        return;
    }

    switch(type)
    {
        case CMD_UNLOCK:
            m_pUndoStack->push(new CommandEditCommand(commandField, cmd->GetName(), m_node, &NodeCtrl::removeOnUnlockCommand, &NodeCtrl::addOnUnlockCommand,
                                                      &NodeCtrl::addParameterToOnUnlockCommand, this, &NodeProperties::DeleteParametersFromCommand, &NodeProperties::AddParametersToCommand,
                                                      m_node->getOnUnlockList()[cmd->GetName()].description, cmd, CMD_UNLOCK, &NodeCtrl::getOnUnlockList));
            break;
        case CMD_FAIL:
            m_pUndoStack->push(new CommandEditCommand(commandField, cmd->GetName(), m_node, &NodeCtrl::removeOnFailCommand, &NodeCtrl::addOnFailCommand,
                                                  &NodeCtrl::addParameterToOnFailCommand, this, &NodeProperties::DeleteParametersFromCommand, &NodeProperties::AddParametersToCommand,
                                                  m_node->getOnFailList()[cmd->GetName()].description, cmd, CMD_FAIL, &NodeCtrl::getOnFailList));
            break;
        case CMD_UNLOCKED:
            m_pUndoStack->push(new CommandEditCommand(commandField, cmd->GetName(), m_node, &NodeCtrl::removeOnUnlockedCommand, &NodeCtrl::addOnUnlockedCommand,
                                                  &NodeCtrl::addParameterToOnUnlockedCommand, this, &NodeProperties::DeleteParametersFromCommand, &NodeProperties::AddParametersToCommand,
                                                  m_node->getOnUnlockedList()[cmd->GetName()].description, cmd, CMD_UNLOCKED, &NodeCtrl::getOnUnlockedList));
            break;
     }
}

void NodeProperties::createNewPlug()
{
    // duplicate plug names are automatically resolved by the zodiac::Node
    if(m_nextPlugIsIncoming){
        addPlugRow(m_node->addIncomingPlug(s_defaultPlugName));
    } else {
        addPlugRow(m_node->addOutgoingPlug(s_defaultPlugName));
    }
    m_nextPlugIsIncoming = !m_nextPlugIsIncoming;
}

void NodeProperties::addPlugRow(zodiac::PlugHandle plug)
{
    int row = m_plugLayout->rowCount();

    QPushButton* directionButton = new QPushButton(this);
    directionButton->setIconSize(QSize(16, 16));
    directionButton->setFlat(true);
    directionButton->setStatusTip("Toggle the direction of the Plug from 'incoming' to 'outoing' and vice versa.");
    m_plugLayout->addWidget(directionButton, row, 0);

    QLineEdit* plugNameEdit = new QLineEdit(plug.getName(), this);
    m_plugLayout->addWidget(plugNameEdit, row, 1);

    QPushButton* removalButton = new QPushButton(this);
    removalButton->setIcon(QIcon(":/icons/minus.svg"));
    removalButton->setIconSize(QSize(8, 8));
    removalButton->setFlat(true);
    removalButton->setStatusTip("Delete the Plug from its Node");
    m_plugLayout->addWidget(removalButton, row, 2);

    m_plugRows.insert(plug.getName(), new PlugRow(this, plug, plugNameEdit, directionButton, removalButton));
}

void NodeProperties::removePlugRow(const QString& plugName)
{
    Q_ASSERT(m_plugRows.contains(plugName));
    m_plugRows.remove(plugName);
}

void NodeProperties::removeCommandRow(const QUuid& commandId, QHash<QUuid, CommandRow*> *commandRows)
{
    Q_ASSERT(commandRows->contains(commandId));
    commandRows->remove(commandId);
}

PlugRow::PlugRow(NodeProperties* editor, zodiac::PlugHandle plug,
                 QLineEdit* nameEdit, QPushButton* directionToggle, QPushButton* removalButton)
    : QObject(editor)
    , m_editor(editor)
    , m_plug(plug)
    , m_nameEdit(nameEdit)
    , m_directionToggle(directionToggle)
    , m_removalButton(removalButton)
{
    connect(m_nameEdit, SIGNAL(editingFinished()), this, SLOT(renamePlug()));
    connect(m_directionToggle, SIGNAL(clicked()), this, SLOT(togglePlugDirection()));
    connect(m_removalButton, SIGNAL(clicked()), this, SLOT(removePlug()));

    updateDirectionIcon();
}

void PlugRow::renamePlug()
{
    m_nameEdit->setText(m_editor->getNode()->renamePlug(m_plug.getName(), m_nameEdit->text()));
}

void PlugRow::updateDirectionIcon()
{
    if(m_plug.isIncoming()){
        m_directionToggle->setIcon(QIcon(":/icons/incoming.svg"));
    } else {
        m_directionToggle->setIcon(QIcon(":/icons/outgoing.svg"));
    }
}

void PlugRow::togglePlugDirection()
{
    if(!m_editor->getNode()->togglePlugDirection(m_plug.getName())){
        return;
    }
    updateDirectionIcon();
}

void PlugRow::removePlug()
{
    // do nothing, if the plug cannot be removed
    if(!m_plug.isRemovable()){
        return;
    }

    // unregister from the editor
    m_editor->removePlugRow(m_plug.getName());

    // remove widgets from the editor
    QGridLayout* plugLayout = m_editor->getPlugLayout();
    plugLayout->removeWidget(m_directionToggle);
    plugLayout->removeWidget(m_nameEdit);
    plugLayout->removeWidget(m_removalButton);

    // delete the widgets, they are no longer needed
    m_directionToggle->deleteLater();
    m_nameEdit->deleteLater();
    m_removalButton->deleteLater();

    // finally, remove the plug from the logical node
    m_editor->getNode()->removePlug(m_plug.getName());
}

CommandRow::CommandRow(NodeProperties* editor, QComboBox* nameEdit,
                       QPushButton* removalButton, QString &name, QHash<QUuid, CommandRow*> &commandRows, QUuid &uniqueId, QGridLayout* blockLayout, QGridLayout* commandLayout)
    : QObject(editor)
    , m_editor(editor)
    , m_nameEdit(nameEdit)
    , m_removalButton(removalButton)
    , m_commandName(name)
    , m_rowPointer(&commandRows)
    , m_identifier(uniqueId)
    , m_commandLayout(commandLayout)
    , m_blockLayout(blockLayout)
{
    connect(m_removalButton, SIGNAL(clicked()), this, SLOT(removeCommand()));
}

void CommandRow::removeCommand()
{
    // unregister from the editor
    m_editor->removeCommandRow(m_identifier, m_rowPointer);

    // remove widgets from the editor
    m_commandLayout->removeWidget(m_nameEdit);
    m_commandLayout->removeWidget(m_removalButton);

    //remove params
    DeleteParameters();

    // delete the widgets, they are no longer needed
    m_nameEdit->deleteLater();
    m_removalButton->deleteLater();
    m_commandLayout->deleteLater();
}

void CommandRow::addParameterToList(QLabel *label, QLineEdit *text)
{
    params.push_back(std::make_pair(label,text));
}

void CommandRow::addParameterToGrid(QLabel *label, QLineEdit *text)
{
    int row = m_commandLayout->rowCount();
    m_commandLayout->addWidget(label, row, 0);
    m_commandLayout->addWidget(text, row, 1);
}

void CommandRow::DeleteParameters()
{
    for(std::vector<std::pair<QLabel*,QLineEdit*>>::iterator paramIt = params.begin(); paramIt != params.end(); ++paramIt)
    {

        //delete fields and labels from the correct grid
        std::get<0>((*paramIt))->deleteLater();
        std::get<1>((*paramIt))->deleteLater();

        m_commandLayout->removeWidget(std::get<0>((*paramIt)));
        m_commandLayout->removeWidget(std::get<1>((*paramIt)));
    }

    //delete references from command row
    params.clear();
}
