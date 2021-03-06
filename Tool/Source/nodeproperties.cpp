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

bool NodeProperties::s_analyticsMode = false;

static const QString kName_ReqOut = "reqOut";
static const QString kName_ReqOutFull = "Requires:";
static const QString kName_ReqIn = "reqIn";
static const QString kName_ReqInFull = "Provides:";
static const QString kName_storyOut = "storyOut";
static const QString kName_storyOutFull = "Story Nodes:";
static const QString kName_storyIn = "storyIn";
static const QString kName_storyInFull = "Story Parent:";
static const QString kName_narrativeIn = "narrativeIn";
static const QString kName_narrativeInFull = "Narrative Nodes:";

NodeProperties::NodeProperties(NodeCtrl *node, Collapsible *parent, QList<Command> *commands, QUndoStack *undoStack)
    : QWidget(parent)
    , m_node(node)
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

    if(m_node->getType() == zodiac::NODE_NARRATIVE)
        constructNarrativeNodeProperties(mainLayout);
    else
        if(m_node->getType() == zodiac::NODE_STORY)
        constructStoryNodeProperties(mainLayout);

    // define the plug layout
    m_plugLayout = new QGridLayout();
    m_plugLayout->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
    m_plugLayout->setColumnStretch(1,1); // so the add-plug button always stays on the far right
    m_plugLayout->addWidget(new QLabel("<b>Plugs</b>", this), 0, 0, 1, 2, Qt::AlignLeft);

    // define the plugs
    for(zodiac::PlugHandle& plug : m_node->getPlugHandles()){
        addPlugRow(plug);
    }
    mainLayout->addLayout(m_plugLayout);

    QPushButton* removalButton = new QPushButton(" Delete Node", this);
    removalButton->setIcon(QIcon(":/icons/minus.svg"));
    removalButton->setIconSize(QSize(8, 8));
    removalButton->setFlat(true);
    removalButton->setStatusTip("Delete the Node");
    mainLayout->addWidget(removalButton);
    connect(removalButton, &QPushButton::released, [=]{m_node->remove();});

    if(s_analyticsMode)
        removalButton->setEnabled(false);
}

void NodeProperties::constructNarrativeNodeProperties(QVBoxLayout* mainLayout)
{
    //Set initial label names
    QString nameLabel = "Name";
    QString descriptionLabel = "Description";
    QString FileNameLabel = "Filename";

    // define the name and description layout
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QHBoxLayout* descriptionLayout = new QHBoxLayout();
    QHBoxLayout* fileNameLayout = new QHBoxLayout();

    if(m_node->getName() == "SEQ" || m_node->getName() == "INV")
    {
        nameLabel += ": " + m_node->getName();
        descriptionLabel += ": " + m_node->getDescription();
        nameLayout->addWidget(new QLabel(nameLabel, this));
        descriptionLayout->addWidget(new QLabel(descriptionLabel, this));

        nameLayout->setContentsMargins(0, 4, 0, 0);
        mainLayout->addLayout(nameLayout);
        descriptionLayout->setContentsMargins(0, 4, 0, 0);
        mainLayout->addLayout(descriptionLayout);
    }
    else
    {
        nameLayout->addWidget(new QLabel(nameLabel, this));
        m_nameEdit = new QLineEdit(m_node->getName(), this);
        connect(m_nameEdit, SIGNAL(editingFinished()), this, SLOT(renameNode()));
        nameLayout->addWidget(m_nameEdit);

        m_descriptionEdit = new QLineEdit(m_node->getDescription(), this);
        connect(m_descriptionEdit, SIGNAL(editingFinished()), this, SLOT(changeNodeDescription()));
        descriptionLayout->addWidget(new QLabel(descriptionLabel, this));
        descriptionLayout->addWidget(m_descriptionEdit);

        m_fileNameEdit = new QLineEdit(m_node->getFileName(), this);
        connect(m_fileNameEdit, SIGNAL(editingFinished()), this, SLOT(changeNodeFileName()));
        fileNameLayout->addWidget(new QLabel(FileNameLabel, this));
        fileNameLayout->addWidget(m_fileNameEdit);

        if(s_analyticsMode)
        {
            m_nameEdit->setEnabled(false);
            m_descriptionEdit->setEnabled(false);
        }

        nameLayout->setContentsMargins(0, 4, 0, 0);
        mainLayout->addLayout(nameLayout);
        descriptionLayout->setContentsMargins(0, 4, 0, 0);
        mainLayout->addLayout(descriptionLayout);
        mainLayout->addLayout(fileNameLayout);

        // define the on_unlock block
        m_onUnlockLayout = new QGridLayout();
        m_onUnlockLayout->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
        m_onUnlockLayout->setColumnStretch(1,1); // so the add-plug button always stays on the far right
        m_addOnUnlockButton = new QPushButton(this);
        m_addOnUnlockButton->setIconSize(QSize(8, 8));
        m_addOnUnlockButton->setIcon(QIcon(":/icons/plus.svg"));
        m_addOnUnlockButton->setFlat(true);
        m_onUnlockLayout->addWidget(new QLabel("<b>OnUnlock</b>", this), 0, 0, 1, 2, Qt::AlignLeft);
        m_onUnlockLayout->addWidget(m_addOnUnlockButton, 0, 2);
        connect(m_addOnUnlockButton, &QPushButton::released, [=]{m_pUndoStack->push(new CommandAddCommand(m_onUnlockLayout, &m_onUnlockRows, CMD_UNLOCK, &NodeProperties::createNewCommandBlock, this, qobject_cast<Collapsible*>(parent()), m_node));});

        if(s_analyticsMode)
            m_addOnUnlockButton->setEnabled(false);

        if(!m_node->getOnUnlockList().empty())
        {
            QHash<QUuid,zodiac::NodeCommand> *hashPointer = &m_node->getOnUnlockList();
            //need to load command list
            for(QHash<QUuid,zodiac::NodeCommand>::iterator cmdIt = hashPointer->begin(); cmdIt != hashPointer->end(); ++cmdIt)
                if(!cmdIt.key().isNull())
                    createNewCommandBlock(m_onUnlockLayout, m_onUnlockRows, CMD_UNLOCK, cmdIt.key(), &(*cmdIt));
        }

        // define the on_fail block
        m_onFailLayout = new QGridLayout();
        m_onFailLayout->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
        m_onFailLayout->setColumnStretch(1,1); // so the add-plug button always stays on the far right
        m_addOnFailButton = new QPushButton(this);
        m_addOnFailButton->setIconSize(QSize(8, 8));
        m_addOnFailButton->setIcon(QIcon(":/icons/plus.svg"));
        m_addOnFailButton->setFlat(true);
        m_onFailLayout->addWidget(new QLabel("<b>OnFail</b>", this), 0, 0, 1, 2, Qt::AlignLeft);
        m_onFailLayout->addWidget(m_addOnFailButton, 0, 2);
        connect(m_addOnFailButton, &QPushButton::released, [=]{m_pUndoStack->push(new CommandAddCommand(m_onFailLayout, &m_onFailRows, CMD_FAIL, &NodeProperties::createNewCommandBlock, this, qobject_cast<Collapsible*>(parent()), m_node));});

        if(s_analyticsMode)
            m_addOnFailButton->setEnabled(false);

        if(!m_node->getOnFailList().empty())
        {
            QHash<QUuid,zodiac::NodeCommand> *hashPointer = &m_node->getOnFailList();
            //need to load command list
            for(QHash<QUuid,zodiac::NodeCommand>::iterator cmdIt = hashPointer->begin(); cmdIt != hashPointer->end(); ++cmdIt)
                if(!cmdIt.key().isNull())
                    createNewCommandBlock(m_onFailLayout, m_onFailRows, CMD_FAIL, cmdIt.key(), &(*cmdIt));
        }

        // define the on_unlocked block
        m_onUnlockedLayout = new QGridLayout();
        m_onUnlockedLayout->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
        m_onUnlockedLayout->setColumnStretch(1,1); // so the add-plug button always stays on the far right
        m_addOnUnlockedButton = new QPushButton(this);
        m_addOnUnlockedButton->setIconSize(QSize(8, 8));
        m_addOnUnlockedButton->setIcon(QIcon(":/icons/plus.svg"));
        m_addOnUnlockedButton->setFlat(true);
        m_onUnlockedLayout->addWidget(new QLabel("<b>OnUnlocked</b>", this), 0, 0, 1, 2, Qt::AlignLeft);
        m_onUnlockedLayout->addWidget(m_addOnUnlockedButton, 0, 2);
        connect(m_addOnUnlockedButton, &QPushButton::released, [=]{m_pUndoStack->push(new CommandAddCommand(m_onUnlockedLayout, &m_onUnlockedRows, CMD_UNLOCKED, &NodeProperties::createNewCommandBlock, this, qobject_cast<Collapsible*>(parent()), m_node));});

        if(s_analyticsMode)
            m_addOnUnlockedButton->setEnabled(false);

        if(!m_node->getOnUnlockedList().empty())
        {
            QHash<QUuid,zodiac::NodeCommand> *hashPointer = &m_node->getOnUnlockedList();
            //need to load command list
            for(QHash<QUuid,zodiac::NodeCommand>::iterator cmdIt = hashPointer->begin(); cmdIt != hashPointer->end(); ++cmdIt)
                if(!cmdIt.key().isNull())
                    createNewCommandBlock(m_onUnlockedLayout, m_onUnlockedRows, CMD_UNLOCKED, cmdIt.key(), &(*cmdIt));
        }

        mainLayout->addLayout(m_onUnlockLayout);
        mainLayout->addLayout(m_onFailLayout);
        mainLayout->addLayout(m_onUnlockedLayout);
    }
}

void NodeProperties::constructStoryNodeProperties(QVBoxLayout* mainLayout)
{
    //Set initial label names
    QString nameLabel = "Name";
    QString descriptionLabel = "Description";

    // define the name and description layout
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QHBoxLayout* descriptionLayout = new QHBoxLayout();

    zodiac::StoryNodeType type = m_node->getStoryNodeType();

    if(type == zodiac::STORY_SETTING || type == zodiac::STORY_THEME || type == zodiac::STORY_PLOT || type == zodiac::STORY_RESOLUTION ||
            type == zodiac::STORY_SETTING_CHARACTER_GROUP || type == zodiac::STORY_SETTING_LOCATION_GROUP || type == zodiac::STORY_SETTING_TIME_GROUP ||
            type == zodiac::STORY_THEME_EVENT_GROUP || type == zodiac::STORY_THEME_GOAL_GROUP || type == zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP ||
            type == zodiac::STORY_PLOT_EPISODE_OUTCOME_GROUP || type == zodiac::STORY_RESOLUTION_EVENT_GROUP || type == zodiac::STORY_RESOLUTION_STATE_GROUP)
    {
        nameLabel += ": " + m_node->getName();
        descriptionLabel += ": " + m_node->getDescription();
        nameLayout->addWidget(new QLabel(nameLabel, this));
        descriptionLayout->addWidget(new QLabel(descriptionLabel, this));

        nameLayout->setContentsMargins(0, 4, 0, 0);
        mainLayout->addLayout(nameLayout);
        descriptionLayout->setContentsMargins(0, 4, 0, 0);
        mainLayout->addLayout(descriptionLayout);
    }
    else
    {
        nameLayout->addWidget(new QLabel(nameLabel, this));
        m_nameEdit = new QLineEdit(m_node->getName(), this);
        connect(m_nameEdit, SIGNAL(editingFinished()), this, SLOT(renameNode()));
        nameLayout->addWidget(m_nameEdit);
        m_descriptionEdit = new QLineEdit(m_node->getDescription(), this);
        connect(m_descriptionEdit, SIGNAL(editingFinished()), this, SLOT(changeNodeDescription()));
        descriptionLayout->addWidget(new QLabel(descriptionLabel, this));
        descriptionLayout->addWidget(m_descriptionEdit);

        nameLayout->setContentsMargins(0, 4, 0, 0);
        mainLayout->addLayout(nameLayout);
        descriptionLayout->setContentsMargins(0, 4, 0, 0);
        mainLayout->addLayout(descriptionLayout);

        if(s_analyticsMode)
        {
            m_nameEdit->setEnabled(false);
            m_descriptionEdit->setEnabled(false);
        }

    }
}

void NodeProperties::renameNode()
{
    QString newName = m_nameEdit->text();
    if(m_node->getName() == newName)
    {
        return;
    }

    m_pUndoStack->push(new TextEditCommand(false, m_node->getName(), newName, m_node, &NodeCtrl::rename, qobject_cast<Collapsible*>(parent())));
}

void NodeProperties::changeNodeDescription()
{
    QString newDescription = m_descriptionEdit->text();
    if(m_node->getDescription() == newDescription){
        return;
    }

    m_pUndoStack->push(new TextEditCommand(true, m_node->getDescription(), newDescription, m_node, &NodeCtrl::changeDescription, qobject_cast<Collapsible*>(parent())));
}

void NodeProperties::changeNodeFileName()
{
    QString newFileName = m_fileNameEdit->text();
    if(m_node->getFileName() == newFileName){
        return;
    }

    m_pUndoStack->push(new TextEditCommand(true, m_node->getFileName(), newFileName, m_node, &NodeCtrl::setFileName, qobject_cast<Collapsible*>(parent())));
}

CommandRow *NodeProperties::createNewCommandBlock(QGridLayout *grid, QHash<QUuid, CommandRow*> &commandRow, CommandBlockTypes type, const QUuid &id, zodiac::NodeCommand *cmd)
{
    int row = grid->rowCount();

    QGridLayout *commandBlockGrid = new QGridLayout();
    commandBlockGrid->setContentsMargins(0, 8, 0, 0);   // leave space between the plug list and the name
    commandBlockGrid->setColumnStretch(1,1); // so the add-plug button always stays on the far right

    QComboBox* commandBox = new QComboBox();

    for(QList<Command>::iterator it = m_pCommands->begin(); it != m_pCommands->end(); ++it)
        commandBox->addItem((*it).label, (*it).id);

    QUuid u;

    if(cmd == nullptr)
    {
        u = QUuid::createUuid();

        switch(type)
        {
            case CMD_UNLOCK:
                m_node->addOnUnlockCommand(u, commandBox->itemData(commandBox->currentIndex()).toString(), commandBox->currentText());
                break;
            case CMD_FAIL:
                m_node->addOnFailCommand(u, commandBox->itemData(commandBox->currentIndex()).toString(), commandBox->currentText());
                break;
            case CMD_UNLOCKED:
                m_node->addOnUnlockedCommand(u, commandBox->itemData(commandBox->currentIndex()).toString(), commandBox->currentText());
                break;
        }
    }
    else
    {
        u = id;
        commandBox->setCurrentText(cmd->description);
    }

    commandBlockGrid->addWidget(commandBox);

    QPushButton* removalButton = new QPushButton(this);
    removalButton->setIcon(QIcon(":/icons/minus.svg"));
    removalButton->setIconSize(QSize(8, 8));
    removalButton->setFlat(true);
    removalButton->setStatusTip("Delete the Command from its Node");
    commandBlockGrid->addWidget(removalButton, 0, 2);

    grid->addLayout(commandBlockGrid, row, 0);

    commandRow.insert(u, new CommandRow(this, commandBox, removalButton, commandBox->itemData(commandBox->currentIndex()).toString(), &commandRow, u, grid, commandBlockGrid, type));

    AddParametersToCommand(type, commandRow[u], u);
    connect(commandBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=] { changeCommand(commandBox, type, commandRow[u], u); });

    if(s_analyticsMode)
    {
        commandBox->setEnabled(false);
        removalButton->setEnabled(false);
    }

    return commandRow[u];
}

void NodeProperties::AddParametersToCommand(CommandBlockTypes type, CommandRow *cmd, const QUuid &cmdKey)
{
    bool load;

    QHash<QUuid,zodiac::NodeCommand> *hashPointer;

    switch(type)
    {
        case CMD_UNLOCK:
            load = m_node->getOnUnlockList()[cmdKey].parameters.empty();
            hashPointer = &m_node->getOnUnlockList();
            break;
        case CMD_FAIL:
            load = m_node->getOnFailList()[cmdKey].parameters.empty();
            hashPointer = &m_node->getOnFailList();
            break;
        case CMD_UNLOCKED:
            load = m_node->getOnUnlockedList()[cmdKey].parameters.empty();
            hashPointer = &m_node->getOnUnlockedList();
            break;
    }
    //get parameters from the command pointer
    for(QList<Command>::iterator cmdIt = m_pCommands->begin(); cmdIt != m_pCommands->end(); ++cmdIt)
    {
        if((*cmdIt).id == hashPointer->value(cmdKey).id)
            for(QList<Parameter>::iterator paramIt = (*cmdIt).commandParams.begin(); paramIt != (*cmdIt).commandParams.end(); ++paramIt)
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

                if(s_analyticsMode)
                {
                    text->setEnabled(false);
                }
            }
    }
}

void NodeProperties::updateParam(CommandBlockTypes type, const QUuid &cmdKey, const QString &paramKey, QLineEdit* paramField)
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
            m_pUndoStack->push(new ParamEditCommand(newName, oldName, cmdKey, paramKey, m_node, &NodeCtrl::editParameterInOnUnlockCommand, qobject_cast<Collapsible*>(parent()), type));
            break;
        case CMD_FAIL:
            newName = paramField->text();
            oldName = m_node->getParameterFromOnFailCommand(cmdKey, paramKey);
            if(oldName == newName)
                return;
            m_pUndoStack->push(new ParamEditCommand(newName, oldName, cmdKey, paramKey, m_node, &NodeCtrl::editParameterInOnFailCommand, qobject_cast<Collapsible*>(parent()), type));
            break;
        case CMD_UNLOCKED:
            newName = paramField->text();
            oldName = m_node->getParameterFromOnUnlockedCommand(cmdKey, paramKey);
            if(oldName == newName)
                return;
            m_pUndoStack->push(new ParamEditCommand(newName, oldName, cmdKey, paramKey, m_node, &NodeCtrl::editParameterInOnUnlockedCommand, qobject_cast<Collapsible*>(parent()), type));
            break;
    }
}

void NodeProperties::DeleteParametersFromCommand(CommandRow *cmd)
{
    cmd->DeleteParameters();
}

void NodeProperties::changeCommand(QComboBox *commandField, CommandBlockTypes type, CommandRow *cmd, const QUuid &uniqueId)
{
    if(cmd->GetUndo())
    {
        cmd->SetUndo(false);
        return;
    }

    switch(type)
    {
        case CMD_UNLOCK:
            m_pUndoStack->push(new CommandEditCommand(commandField, cmd->GetName(), m_node->getOnUnlockList()[uniqueId].description, commandField->itemData(commandField->currentIndex()).toString(), commandField->currentText(), m_node, &NodeCtrl::removeOnUnlockCommand, &NodeCtrl::addOnUnlockCommand,
                                                      &NodeCtrl::addParameterToOnUnlockCommand, this, &NodeProperties::DeleteParametersFromCommand, &NodeProperties::AddParametersToCommand,
                                                      cmd, CMD_UNLOCK, &NodeCtrl::getOnUnlockList, uniqueId, qobject_cast<Collapsible*>(parent())));
            break;
        case CMD_FAIL:
            m_pUndoStack->push(new CommandEditCommand(commandField, cmd->GetName(), m_node->getOnFailList()[uniqueId].description, commandField->itemData(commandField->currentIndex()).toString(), commandField->currentText(), m_node, &NodeCtrl::removeOnFailCommand, &NodeCtrl::addOnFailCommand,
                                                  &NodeCtrl::addParameterToOnFailCommand, this, &NodeProperties::DeleteParametersFromCommand, &NodeProperties::AddParametersToCommand,
                                                  cmd, CMD_FAIL, &NodeCtrl::getOnFailList, uniqueId, qobject_cast<Collapsible*>(parent())));
            break;
        case CMD_UNLOCKED:
            m_pUndoStack->push(new CommandEditCommand(commandField, cmd->GetName(), m_node->getOnUnlockedList()[uniqueId].description, commandField->itemData(commandField->currentIndex()).toString(), commandField->currentText(), m_node, &NodeCtrl::removeOnUnlockedCommand, &NodeCtrl::addOnUnlockedCommand,
                                                  &NodeCtrl::addParameterToOnUnlockedCommand, this, &NodeProperties::DeleteParametersFromCommand, &NodeProperties::AddParametersToCommand,
                                                  cmd, CMD_UNLOCKED, &NodeCtrl::getOnUnlockedList, uniqueId, qobject_cast<Collapsible*>(parent())));
            break;
     }
}

void NodeProperties::addPlugRow(zodiac::PlugHandle plug)
{
    int row = m_plugLayout->rowCount();

    QGridLayout *rowLayout = new QGridLayout();

    QString name = plug.getName();

    //show full name for the plug, if none of these then original name will stick
    if (name == kName_ReqOut)
        name = "<i>" + kName_ReqOutFull + "</i>";
    else
        if (name == kName_ReqIn)
            name = "<i>" + kName_ReqInFull + "</i>";
        else
            if (name == kName_storyOut)
                name = "<i>" + kName_storyOutFull + "</i>";
            else
                if (name == kName_storyIn)
                    name = "<i>" + kName_storyInFull + "</i>";
                else
                    if (name == kName_narrativeIn)
                        name = "<i>" + kName_narrativeInFull + "</i>";


    QLabel* plugName = new QLabel(name, this);
    //m_plugLayout->addWidget(plugName, row, 1);
    rowLayout->addWidget(plugName, 0, 0);

    m_plugLayout->addLayout(rowLayout, row, 0);

    m_plugRows.insert(plug.getName(), new PlugRow(this, plug, plugName, rowLayout));
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

PlugRow::PlugRow(NodeProperties* editor, zodiac::PlugHandle plug, QLabel* nameLabel, QGridLayout *rowLayout)
    : QObject(editor)
    , m_editor(editor)
    , m_plug(plug)
    , m_nameLabel(nameLabel)
{

    int row = rowLayout->rowCount();

    if(plug.getName() == kName_storyOut || plug.getName() == kName_storyIn)
    {
        QList<zodiac::PlugHandle> connections =  plug.getConnectedPlugs();
        for(QList<zodiac::PlugHandle>::iterator conIt = connections.begin(); conIt != connections.end(); ++conIt)
        {
            QLabel *connectionLabel = new QLabel((*conIt).getNode().getName());
            rowLayout->addWidget(connectionLabel, row, 1);

            QPushButton* connectionButton;

            if(plug.getNode().getType() == zodiac::NODE_STORY)
                connectionButton = nullptr;
            else
            {
                connectionButton = new QPushButton();
                connectionButton->setIcon(QIcon(":/icons/minus.svg"));
                connectionButton->setIconSize(QSize(8, 8));
                connectionButton->setFlat(true);
                connectionButton->setStatusTip("Delete the Connection");
                rowLayout->addWidget(connectionButton, row, 2);
            }

            ++row;

            zodiac::Plug *outgoing = plug.data();
            zodiac::Plug *incoming = (*conIt).data();
            QPair<QLabel*, QPushButton*> connection(connectionLabel, connectionButton);

            if(editor->s_analyticsMode)
            {
                if(connectionButton)
                    connectionButton->setEnabled(false);
            }

            if(connectionButton)
                connect(connectionButton, &QPushButton::released, [=]{m_editor->getUndoStack()->push(new NodeRemoveLink(outgoing, incoming, this, &PlugRow::removePlugConnection, connection));});

            m_storyConnections.push_back(connection);
        }
    }

    if(plug.getName() == kName_ReqOut || plug.getName() == kName_ReqIn || plug.getName() == kName_narrativeIn)
    {
        QList<zodiac::PlugHandle> connections =  plug.getConnectedPlugs();
        for(QList<zodiac::PlugHandle>::iterator conIt = connections.begin(); conIt != connections.end(); ++conIt)
        {
            if((*conIt).getNode().getName() == "SEQ" || (*conIt).getNode().getName() == "INV")
            {
                QString nodeName = (*conIt).getNode().getName();
                QString plugName = plug.getName();

                QList<zodiac::PlugHandle> secondConnections =  (*conIt).getNode().getPlug(plugName).getConnectedPlugs();
                for(QList<zodiac::PlugHandle>::iterator secConIt = secondConnections.begin(); secConIt != secondConnections.end(); ++secConIt)
                {
                    QLabel *connectionLabel = new QLabel((*secConIt).getNode().getName() + " - " + nodeName);
                    QPushButton* connectionButton = new QPushButton();
                    connectionButton->setIcon(QIcon(":/icons/minus.svg"));
                    connectionButton->setIconSize(QSize(8, 8));
                    connectionButton->setFlat(true);
                    connectionButton->setStatusTip("Delete the Connection");

                    rowLayout->addWidget(connectionLabel, row, 1);
                    rowLayout->addWidget(connectionButton, row, 2);
                    ++row;

                    zodiac::Plug *outgoing = plug.data();
                    zodiac::Plug *incoming = (*conIt).data();
                    QPair<QLabel*, QPushButton*> connection(connectionLabel, connectionButton);

                    if(editor->s_analyticsMode)
                    {
                        connectionButton->setEnabled(false);
                    }

                    connect(connectionButton, &QPushButton::released, [=]{m_editor->getUndoStack()->push(new NodeRemoveLink(outgoing, incoming, this, &PlugRow::removePlugConnection, connection));});

                    m_narrativeConnections.push_back(connection);
                }
            }
            else
            {
                QLabel *connectionLabel = new QLabel((*conIt).getNode().getName());
                QPushButton* connectionButton = new QPushButton();
                connectionButton->setIcon(QIcon(":/icons/minus.svg"));
                connectionButton->setIconSize(QSize(8, 8));
                connectionButton->setFlat(true);
                connectionButton->setStatusTip("Delete the Connection");

                rowLayout->addWidget(connectionLabel, row, 1);
                rowLayout->addWidget(connectionButton, row, 2);
                ++row;

                zodiac::Plug *outgoing = plug.data();
                zodiac::Plug *incoming = (*conIt).data();
                QPair<QLabel*, QPushButton*> connection(connectionLabel, connectionButton);

                if(editor->s_analyticsMode)
                {
                    connectionButton->setEnabled(false);
                }

                connect(connectionButton, &QPushButton::released, [=]{m_editor->getUndoStack()->push(new NodeRemoveLink(outgoing, incoming, this, &PlugRow::removePlugConnection, connection));});

                m_narrativeConnections.push_back(connection);
            }
        }
    }
}

void PlugRow::renamePlug()
{
    m_nameLabel->setText(m_editor->getNode()->renamePlug(m_plug.getName(), m_nameLabel->text()));
}

void PlugRow::removePlugConnection(QPair<QLabel*, QPushButton*> &connection)
{
    //remove both elements
    connection.first->deleteLater();
    connection.second->deleteLater();

    //remove from list
}

CommandRow::CommandRow(NodeProperties* editor, QComboBox* nameEdit,
                       QPushButton* removalButton, QString &name, QHash<QUuid, CommandRow*> *commandRows, QUuid &uniqueId, QGridLayout* blockLayout,
                       QGridLayout* commandLayout, CommandBlockTypes type)
    : QObject(editor)
    , m_editor(editor)
    , m_nameEdit(nameEdit)
    , m_removalButton(removalButton)
    , m_commandName(name)
    , m_rowPointer(commandRows)
    , m_identifier(uniqueId)
    , m_commandLayout(commandLayout)
    , m_blockLayout(blockLayout)
    , m_commandType(type)
{
    //connect(m_removalButton, SIGNAL(clicked()), this, SLOT(removeCommand()));
    switch(type)
    {
        case CMD_UNLOCK:
            connect(m_removalButton, &QPushButton::released, [=]{m_editor->getUndoStack()->push(new CommandDeleteCommand(m_rowPointer, CMD_UNLOCK, &NodeProperties::createNewCommandBlock, m_editor, this, m_nameEdit->itemData(m_nameEdit->currentIndex()).toString(), m_nameEdit->currentText(),
                                                                                                                           &CommandRow::DeleteParameters, m_editor->getNode(), &NodeCtrl::addOnUnlockCommand, &NodeCtrl::addParameterToOnUnlockCommand,
                                                                                                                           &NodeCtrl::getOnUnlockList, uniqueId, m_editor->getParent()));});
            break;
        case CMD_FAIL:
            connect(m_removalButton, &QPushButton::released, [=]{m_editor->getUndoStack()->push(new CommandDeleteCommand(m_rowPointer, CMD_FAIL, &NodeProperties::createNewCommandBlock, m_editor, this, m_nameEdit->itemData(m_nameEdit->currentIndex()).toString(), m_nameEdit->currentText(),
                                                                                                                           &CommandRow::DeleteParameters, m_editor->getNode(), &NodeCtrl::addOnFailCommand, &NodeCtrl::addParameterToOnFailCommand,
                                                                                                                           &NodeCtrl::getOnFailList, uniqueId, m_editor->getParent()));});
            break;
        case CMD_UNLOCKED:
            connect(m_removalButton, &QPushButton::released, [=]{m_editor->getUndoStack()->push(new CommandDeleteCommand(m_rowPointer, CMD_UNLOCKED, &NodeProperties::createNewCommandBlock, m_editor, this, m_nameEdit->itemData(m_nameEdit->currentIndex()).toString(), m_nameEdit->currentText(),
                                                                                                                           &CommandRow::DeleteParameters, m_editor->getNode(), &NodeCtrl::addOnUnlockedCommand, &NodeCtrl::addParameterToOnUnlockedCommand,
                                                                                                                           &NodeCtrl::getOnUnlockedList, uniqueId, m_editor->getParent()));});
            break;
    }
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

    //remove from the logical node
    switch(m_commandType)
    {
        case CMD_UNLOCK:
            m_editor->getNode()->removeOnUnlockCommand(m_identifier);
            break;
        case CMD_FAIL:
            m_editor->getNode()->removeOnFailCommand(m_identifier);
            break;
        case CMD_UNLOCKED:
            m_editor->getNode()->removeOnUnlockedCommand(m_identifier);
            break;
    }
}

void CommandRow::addParameterToList(QLabel *label, QLineEdit *text)
{
    m_params.push_back(std::make_pair(label,text));
}

void CommandRow::addParameterToGrid(QLabel *label, QLineEdit *text)
{
    int row = m_commandLayout->rowCount();
    m_commandLayout->addWidget(label, row, 0);
    m_commandLayout->addWidget(text, row, 1);
}

void CommandRow::DeleteParameters()
{
    for(std::vector<std::pair<QLabel*,QLineEdit*>>::iterator paramIt = m_params.begin(); paramIt != m_params.end(); ++paramIt)
    {

        //delete fields and labels from the correct grid
        std::get<0>((*paramIt))->deleteLater();
        std::get<1>((*paramIt))->deleteLater();

        m_commandLayout->removeWidget(std::get<0>((*paramIt)));
        m_commandLayout->removeWidget(std::get<1>((*paramIt)));
    }

    //delete references from command row
    m_params.clear();
}
