#include "saveandload.h"

saveandload::saveandload()
{
}

bool saveandload::LoadStoryFromFile(QWidget *widget)
{
    QFile file(QFileDialog::getOpenFileName(widget,
                                                     QObject::tr("Load Story Graph"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty()&& !file.fileName().isNull())
    {
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString settings = file.readAll();
            file.close();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(settings.toUtf8());
            //qDebug() << jsonDoc.toJson();

            if(jsonDoc.isNull() || !jsonDoc.isObject() || jsonDoc.isEmpty())
            {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
                messageBox.setFixedSize(500,200);
                return false;
            }

            QJsonObject jsonObject=jsonDoc.object();

            //on_pushButton_reset_clicked();

            QJsonValue jsonStoryName = jsonObject["storyName"];
            //qDebug() <<  jsonStoryName.toString();
            m_storyName = jsonStoryName.toString();

            QJsonObject jsonSetting = jsonObject["setting"].toObject();
            QJsonObject jsonTheme = jsonObject["theme"].toObject();
            QJsonObject jsonPlot = jsonObject["plot"].toObject();
            QJsonObject jsonResolution = jsonObject["resolution"].toObject();

            ReadCharacters(jsonSetting["characters"].toArray());
            ReadLocations(jsonSetting["locations"].toArray());
            ReadTimes(jsonSetting["times"].toArray());

            ReadEvents(jsonTheme["events"].toArray());
            ReadGoals(jsonTheme["goals"].toArray());

            ReadEpisodes(jsonPlot["episodes"].toArray());

            ReadResolution(jsonResolution);

            return true;
        }
        else
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
            messageBox.setFixedSize(500,200);
            return false;
        }
    }
    else
    {
        qDebug() << "Load aborted by user";
        return false;
    }
}

void saveandload::ReadCharacters(QJsonArray &jsonCharacters)
{
    foreach (const QJsonValue &charValue, jsonCharacters)
    {
        m_characters.push_back(SettingItem());
        QList<SettingItem>::iterator charIt = m_characters.end();
        --charIt;

        ReadSettingsItem(charValue.toObject(), (*charIt));
    }
}

void saveandload::ReadLocations(QJsonArray &jsonLocations)
{
  foreach (const QJsonValue &locValue, jsonLocations)
  {
      m_locations.push_back(SettingItem());
      QList<SettingItem>::iterator locIt = m_locations.end();
      --locIt;

      ReadSettingsItem(locValue.toObject(), (*locIt));
  }
}

void saveandload::ReadTimes(QJsonArray &jsonTimes)
{
    foreach (const QJsonValue &timValue, jsonTimes)
    {
        m_times.push_back(SettingItem());
        QList<SettingItem>::iterator timIt = m_times.end();
        --timIt;

        ReadSettingsItem(timValue.toObject(), (*timIt));
    }
}

void saveandload::ReadSettingsItem(QJsonObject &jsonCharacter, SettingItem &s)
{
    QJsonValue jsonCharacterId = jsonCharacter[kName_Id];

    s.id = jsonCharacterId.toString().section('_', 1);
    qDebug() << jsonCharacterId.toString().section('_', 1);

    QJsonValue jsonCharacterDesc = jsonCharacter[kName_Description];

    s.description = jsonCharacterDesc.toString();
    qDebug() << jsonCharacterDesc.toString();

    QJsonArray jsonCharacterDetails = jsonCharacter[kName_Details].toArray();

    foreach (const QJsonValue &detailValue, jsonCharacterDetails)
    {
        s.details.push_back(SimpleNode());
        QList<SimpleNode>::iterator detailIt = (s).details.end();
        --detailIt;

        QJsonObject detail = detailValue.toObject();

        (*detailIt).id = detail[kName_Id].toString().section('_', 1);
        qDebug() << detail[kName_Id].toString().section('_', 1);

        (*detailIt).description = detail[kName_Description].toString();
        qDebug() << detail[kName_Description].toString();

        if(detail.contains(kName_State))
        {
            QJsonObject jsonState = detail[kName_State].toObject();
        }
    }
}

void saveandload::ReadEvents(QJsonArray &jsonEvents)
{
    foreach (const QJsonValue &eventValue, jsonEvents)
    {
        m_events.push_back(EventGoal());
        QList<EventGoal>::iterator evIt = m_events.end();
        --evIt;

        ReadEventGoal(eventValue.toObject(), kName_SubEvents, (*evIt));
    }
}

void saveandload::ReadGoals(QJsonArray &jsonGoals)
{
    foreach (const QJsonValue &goalValue, jsonGoals)
    {
        m_goals.push_back(EventGoal());
        QList<EventGoal>::iterator goIt = m_goals.end();
        --goIt;

        ReadEventGoal(goalValue.toObject(), kName_SubGoals, (*goIt));
    }
}

void saveandload::ReadEventGoal(QJsonObject &eventGoal, QString SubItemId, EventGoal &e)
{
        QJsonValue eventGoalId = eventGoal[kName_Id];
        QJsonValue eventGoalDescription = eventGoal[kName_Description];

        e.id = eventGoalId.toString().section('_', 1);
        qDebug() << eventGoalId.toString().section('_', 1);

        e.description = eventGoalDescription.toString();
        qDebug() << eventGoalDescription.toString();

        if(eventGoal.contains(SubItemId))
        {
            QJsonArray jsonSubItems = eventGoal[SubItemId].toArray();

            foreach (const QJsonValue &subItemValue, jsonSubItems)
            {
                e.subItems.push_back(EventGoal());

                QList<EventGoal>::iterator subEvIt = e.subItems.end();
                --subEvIt;

                ReadSubItem(subItemValue.toObject(), (*subEvIt), SubItemId);
            }
        }
}

void saveandload::ReadSubItem(QJsonObject &jsonSubItem, EventGoal &e, QString SubItemId)
{
    e.id = jsonSubItem[kName_Id].toString().section('_', 1);
    qDebug() << jsonSubItem[kName_Id].toString().section('_', 1);

    e.description = jsonSubItem[kName_Description].toString();
    qDebug() << jsonSubItem[kName_Description].toString();

    if(jsonSubItem.contains(SubItemId))
    {
        QJsonArray jsonSubEvents = jsonSubItem[SubItemId].toArray();

        foreach (const QJsonValue &subItemValue, jsonSubEvents)
        {
            e.subItems.push_back(EventGoal());

            QList<EventGoal>::iterator subEvIt = e.subItems.end();
            --subEvIt;

            ReadSubItem(subItemValue.toObject(), (*subEvIt), SubItemId);
        }
    }
}

void saveandload::ReadEpisodes(QJsonArray &jsonEpisodes)
{
    foreach (const QJsonValue &episodeValue, jsonEpisodes)
    {
        m_episodes.push_back(Episode());
        QList<Episode>::iterator epIt = m_episodes.end();
        --epIt;

        ReadEpisode(episodeValue.toObject(), (*epIt));
    }
}

void saveandload::ReadEpisode(QJsonObject jsonSubEpisode, Episode &e)
{
    QJsonArray jsonAttempts = jsonSubEpisode[kName_Attempts].toArray();
    QJsonArray jsonOutcomes = jsonSubEpisode[kName_Outcomes].toArray();
    QJsonValue id = jsonSubEpisode[kName_Id];
    QJsonValue description = jsonSubEpisode[kName_Description];

    QJsonObject subGoal = jsonSubEpisode[kName_SubGoal].toObject();
    QJsonValue subGoalId = subGoal[kName_Id];
    QJsonValue subGoalDescription = subGoal[kName_Description];

    e.id = id.toString().section('_', 1);
    qDebug() << id.toString();

    e.description = description.toString();
    qDebug() << description.toString();

    e.subGoal.id = subGoalId.toString().section('_', 1);
    qDebug() << subGoalId.toString();

    e.subGoal.description = subGoalDescription.toString();
    qDebug() << subGoalDescription.toString();

    foreach (const QJsonValue &attemptValue, jsonAttempts)
    {
        QJsonObject attempt = attemptValue.toObject();

        if(attempt[kName_Id].toString().section('_', 0, 0) == kPrefix_SubEpisode)  //handle sub episode
        {
            e.attemptSubEpisodes.push_back(Episode());
            QList<Episode>::iterator subEpIt = e.attemptSubEpisodes.end();
            --subEpIt;
            ReadEpisode(attempt, (*subEpIt));
        }
        else    //handle attempt as normal
        {
            e.attempts.push_back(SimpleNode());
            QList<SimpleNode>::iterator attIt = e.attempts.end();
            --attIt;

            qDebug() << attempt[kName_Id].toString().section('_', 1);
            (*attIt).id = attempt[kName_Id].toString().section('_', 1);

            qDebug() << attempt[kName_Description].toString();
            (*attIt).description = attempt[kName_Description].toString();
        }
    }

    foreach (const QJsonValue &outcomeValue, jsonOutcomes)
    {
        QJsonObject outcome = outcomeValue.toObject();

        if(outcome[kName_Id].toString().section('_', 0, 0) == "SUBEP")  //handle sub episode
        {
            e.outcomeSubEpisodes.push_back(Episode());
            QList<Episode>::iterator subEpIt = e.outcomeSubEpisodes.end();
            --subEpIt;
            ReadEpisode(outcome, (*subEpIt));
        }
        else    //handle attempt as normal
        {
            e.outcomes.push_back(SimpleNode());
            QList<SimpleNode>::iterator outIt = e.outcomes.end();
            --outIt;

            qDebug() << outcome[kName_Id].toString().section('_', 1);
            (*outIt).id = outcome[kName_Id].toString().section('_', 1);

            qDebug() << outcome[kName_Description].toString();
            (*outIt).description = outcome[kName_Description].toString();
        }
    }
}

void saveandload::ReadResolution(QJsonObject &jsonResolution)
{
    QJsonObject::iterator it;

    QJsonArray jsonEvents = jsonResolution[kName_Events].toArray();


    foreach (const QJsonValue &eventValue, jsonEvents)
    {
        m_resolution.events.push_back(EventGoal());
        QList<EventGoal>::iterator evIt = m_resolution.events.end();
        --evIt;

        ReadEventGoal(eventValue.toObject(), kName_SubEvents, (*evIt)); //same as loading them event so use the same function
    }

    QJsonArray jsonStates = jsonResolution[kName_States].toArray();

    foreach (const QJsonValue &stateValue, jsonStates)
    {
       m_resolution.states.push_back(SimpleNode());
        QList<SimpleNode>::iterator stIt = m_resolution.states.end();
        --stIt;

        QJsonObject jsonState = stateValue.toObject();

        (*stIt).id = jsonState[kName_Id].toString().section('_', 1);
        qDebug() << jsonState[kName_Id].toString().section('_', 1);

        (*stIt).description = jsonState[kName_Description].toString();
        qDebug() << jsonState[kName_Description].toString();
    }

}

void saveandload::SaveStoryToFile(QWidget *widget)
{
    QFile file(QFileDialog::getSaveFileName(widget,
                                                     QObject::tr("Save Story Graph"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty()&& !file.fileName().isNull())
    {
        QJsonObject jsonData;
        jsonData["storyName"] = m_storyName;

        QJsonObject jsonSetting;
        if(!m_characters.empty())
            WriteSettingItem(jsonSetting, m_characters, "characters", kPrefix_Characters);
        if(!m_locations.empty())
            WriteSettingItem(jsonSetting, m_locations, "locations", kPrefix_Locations);
        if(!m_times.empty())
            WriteSettingItem(jsonSetting, m_times, "times", kPrefix_Times);
        jsonData["setting"] = jsonSetting;

        QJsonObject jsonTheme;
        WriteEventGoals(jsonTheme, m_events, "events", "subEvents", kPrefix_ThemeEvent);
        WriteEventGoals(jsonTheme, m_goals, "goals", "subGoals", kPrefix_ThemeGoal);
        jsonData["theme"] = jsonTheme;

        QJsonObject jsonPlot;
        WriteEpisodes(jsonPlot, kPrefix_Episode);
        jsonData["plot"] = jsonPlot;

        QJsonObject jsonResolution;
        WriteResolution(jsonResolution);
        jsonData["resolution"] = jsonResolution;

        QJsonDocument jsonDoc(jsonData);

        qDebug() << jsonDoc.toJson();

        if(file.open(QFile::WriteOnly))
            file.write(jsonDoc.toJson());
        else
        {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
                messageBox.setFixedSize(500,200);
        }
    }
    else
        qDebug() << "Save aborted by user";
}

void saveandload::WriteSettingItem(QJsonObject &jsonSetting, QList<SettingItem> &settingList, QString elementName, const QString &prefix)
{
    QJsonArray jsonSettingItems;

    foreach (SettingItem settingItem, settingList)
    {
        QJsonObject jsonSettingItem;
        jsonSettingItem[kName_Id] = QString(prefix + "_" + settingItem.id);
        jsonSettingItem[kName_Description] = settingItem.description;

        if(!settingItem.details.empty())
        {
            QJsonArray jsonDetails;

            foreach (SimpleNode detail, settingItem.details)
            {
                QJsonObject jsonDetail;

                jsonDetail[kName_Id] = QString(kPrefix_Detail + "_" + detail.id);
                jsonDetail[kName_Description] = detail.description;

                jsonDetails.append(jsonDetail);

            }

            jsonSettingItem[kName_Details] = jsonDetails;
        }

        jsonSettingItems.append(jsonSettingItem);
    }

    jsonSetting[elementName] = jsonSettingItems;
}

void saveandload::WriteEventGoals(QJsonObject &jsonTheme, QList<EventGoal> &eVList, QString eventGoalId, QString subItemId, const QString &prefix)
{
    if(!eVList.empty())
    {
        QJsonArray jsonEventGoals;
        foreach (EventGoal event, eVList)
        {
                WriteEventGoal(jsonEventGoals, event, subItemId, prefix);
        }

        jsonTheme[eventGoalId] = jsonEventGoals;
    }
}

void saveandload::WriteEventGoal(QJsonArray &jsonEvents, const EventGoal &e, QString subItemId, const QString &prefix)
{
    QJsonObject jsonEvent;

    if(!e.subItems.empty())
    {
        QJsonArray jsonSubEvents;

        foreach (EventGoal subEvent, e.subItems)
        {
            WriteEventGoal(jsonSubEvents, subEvent, subItemId, prefix);
        }

        jsonEvent[subItemId] = jsonSubEvents;
    }

    jsonEvent[kName_Id] = QString(prefix + "_" + e.id);
    jsonEvent[kName_Description] = e.description;

    jsonEvents.append(jsonEvent);
}

void saveandload::WriteEpisodes(QJsonObject &jsonPlot, const QString &prefix)
{
    if(!m_episodes.empty())
    {
        QJsonArray jsonEpisodes;

        foreach (Episode ep, m_episodes)
        {
                WriteEpisode(jsonEpisodes, ep, prefix);
        }

        jsonPlot[kName_Episodes] = jsonEpisodes;
    }
}

void saveandload::WriteEpisode(QJsonArray &jsonEpisodes, const Episode &e, const QString &prefix)
{
    QJsonObject jsonEpisode;

    jsonEpisode[kName_Id] = QString(prefix + "_" + e.id);
    jsonEpisode[kName_Description] = e.description;

    QJsonObject jsonSubGoal;

    jsonSubGoal[kName_Id] = QString(kPrefix_SubGoal + "_" + e.subGoal.id);
    jsonSubGoal[kName_Description] = e.subGoal.description;

    jsonEpisode[kName_SubGoal] = jsonSubGoal;

    if(!e.attempts.empty() || !e.attemptSubEpisodes.empty())
    {
        QJsonArray jsonAttempts;

        foreach (SimpleNode attempt, e.attempts)
        {
            QJsonObject jsonAttempt;

            jsonAttempt[kName_Id] = QString(kPrefix_Attempt + "_" + attempt.id);
            jsonAttempt[kName_Description] = attempt.description;

            jsonAttempts.append(jsonAttempt);
        }

        foreach (Episode subEp, e.attemptSubEpisodes)
        {
            WriteEpisode(jsonAttempts, subEp, kPrefix_SubEpisode);
        }

         jsonEpisode[kName_Attempts] = jsonAttempts;
    }

    if(!e.outcomes.empty() || !e.outcomeSubEpisodes.empty())
    {
        QJsonArray jsonOutcomes;

        foreach (SimpleNode outcome, e.outcomes)
        {
            QJsonObject jsonOutcome;
            jsonOutcome[kName_Id] = QString(kPrefix_Outcome + "_" + outcome.id);
            jsonOutcome[kName_Description] = outcome.description;

            jsonOutcomes.append(jsonOutcome);
        }

        foreach (Episode subEp, e.outcomeSubEpisodes)
        {
            WriteEpisode(jsonOutcomes, subEp, kPrefix_SubEpisode);
        }

        jsonEpisode[kName_Outcomes] = jsonOutcomes;
    }
        jsonEpisodes.append(jsonEpisode);
}

void saveandload::WriteResolution(QJsonObject &jsonResolution)
{
    //use theme function for this, resolution events are identical
    WriteEventGoals(jsonResolution, m_resolution.events, "events", "subEvents", kPrefix_ResolutionEvent);

    if(!m_resolution.states.empty())
    {
        QJsonArray jsonStates;

        foreach (SimpleNode state, m_resolution.states)
        {
            QJsonObject jsonState;

            jsonState[kName_Id] = QString(kPrefix_ResolutionState + "_" + state.id);
            jsonState[kName_Description] = state.description;

            jsonStates.append(jsonState);
        }

        jsonResolution[kName_States] = jsonStates;
    }
}

void saveandload::DeleteAllStoryItems()
{
    m_storyName = "";
    m_characters.clear();
    m_locations.clear();
    m_times.clear();
    m_episodes.clear();
    m_events.clear();
    m_goals.clear();
    m_resolution.events.clear();
    m_resolution.states.clear();

}

SettingItem *saveandload::addCharacter(QString id, QString description)
{
    SettingItem newCharacter;
    newCharacter.id = id;
    newCharacter.description = description;

    m_characters.push_back(newCharacter);

    return &m_characters.back();
}

SettingItem *saveandload::addLocation(QString id, QString description)
{
    SettingItem newLocation;
    newLocation.id = id;
    newLocation.description = description;

    m_locations.push_back(newLocation);

    return &m_locations.back();

    return nullptr; //if a problem
}

SettingItem *saveandload::addTime(QString id, QString description)
{
    SettingItem newTime;
    newTime.id = id;
    newTime.description = description;

    m_times.push_back(newTime);

    return &m_times.back();

    return nullptr; //if a problem
}

void saveandload::addDetail(SettingItem *item, QString id, QString description, QString stateId, QString stateDescription)
{
    SimpleNode newDetail;
    newDetail.id = id;
    newDetail.description = description;

    item->details.push_back(newDetail);
}

EventGoal *saveandload::addEventGoal(QString id, QString description,zodiac::StoryNodeType type, EventGoal* parent)
{
    EventGoal newEvent;
    newEvent.id = id;
    newEvent.description = description;

    if(parent != nullptr)
    {
        parent->subItems.push_back(newEvent);
        return &parent->subItems.back();
    }
    else
    {
        if(type == zodiac::STORY_THEME_EVENT)
        {
            m_events.push_back(newEvent);
            return &m_events.back();
        }
        else
            if(type == zodiac::STORY_THEME_GOAL)
            {
                m_events.push_back(newEvent);
                return &m_events.back();
            }
            else
                if(type == zodiac::STORY_RESOLUTION_EVENT)
                {
                    m_resolution.events.push_back(newEvent);
                    return &m_resolution.events.back();
                }
                else
                    return nullptr; //if a problem
    }

}

Episode *saveandload::addEpisode(QString id, QString description, Episode* parent, zodiac::StoryNodeType type)
{
    Episode newEpisode;
    newEpisode.id = id;
    newEpisode.description = description;

    if(parent != nullptr)
    {
        if(type == zodiac::STORY_PLOT_EPISODE_ATTEMPT_GROUP)
        {
            parent->attemptSubEpisodes.push_back(newEpisode);
            return &parent->attemptSubEpisodes.back();
        }
        else    //STORY_PLOT_EPISODE_OUTCOME_GROUP
        {
            parent->outcomeSubEpisodes.push_back(newEpisode);
            return &parent->outcomeSubEpisodes.back();
        }
    }
    else
    {
        m_episodes.push_back(newEpisode);
        return &m_episodes.back();
    }

    return nullptr; //if a problem
}

SimpleNode *saveandload::addAttempt(QString id, QString description, Episode* parent)
{
    SimpleNode newAttempt;
    newAttempt.id = id;
    newAttempt.description = description;

    parent->attempts.push_back(newAttempt);

    return &parent->attempts.back();
}

SimpleNode *saveandload::addOutcome(QString id, QString description, Episode* parent)
{
    SimpleNode newOutcome;
    newOutcome.id = id;
    newOutcome.description = description;

    parent->outcomes.push_back(newOutcome);

    return &parent->outcomes.back();
}

void saveandload::addResolutionState(QString id, QString description)
{
    SimpleNode newState;
    newState.id = id;
    newState.description = description;

    m_resolution.states.push_back(newState);
}

void saveandload::addSubGoal(QString id, QString description, Episode* parent)
{
    parent->subGoal.id = id;
    parent->subGoal.description = description;
}

void saveandload::LoadParams(QJsonArray &jsonParams)
{
    foreach (const QJsonValue & value, jsonParams)
    {
        QJsonObject obj = value.toObject();

        m_parameters.push_back(Parameter(obj["label"].toString(), obj["id_name"].toString(), obj["type"].toString()));
    }
}

void saveandload::LoadCommands(QJsonArray &jsonCommands)
{
    foreach (const QJsonValue & value, jsonCommands)
    {
        QJsonObject obj = value.toObject();
        //qDebug() << obj["label"].toString();
        //qDebug() << obj["id_name"].toString();
        //qDebug() << obj["type"].toString();

        QJsonArray jsonCommandParams = obj["params"].toArray();
        QList<Parameter> params;

        foreach (const QJsonValue &value2, jsonCommandParams)
            foreach (Parameter param, m_parameters)
            {
                if(param.id == value2.toString())
                {
                    params.push_back(param);
                    break;
                }
            }

        m_commands.push_back(Command(obj["label"].toString(), obj["id_name"].toString(), obj["type"].toString(), params));
    }
}

NarNode *saveandload::addNarrativeNode(QString id, QString description, QString fileName)
{
    //check if node already exists
    foreach(NarNode node, m_narrativeNodes)
    {
        if(node.id == id)
            return nullptr; //return nullptr if node exists
    }

    //check if the filename is new, if not then add it to the list
    bool fileNameExists = false;
    foreach(QString name, m_fileNames)
        if(name == fileName)
        {
            fileNameExists = true;
            break;
        }

    if(!fileNameExists)
    {
        qDebug() << "Filename" << fileName << "added";
        m_fileNames.push_back(fileName);
    }

    //create the node
    NarNode newNode;
    newNode.id = id;
    newNode.comments = description;
    newNode.fileName = fileName;
    newNode.requirements.type = REQ_NONE; //will be changed if requirements are specified

    m_narrativeNodes.push_back(newNode);

    return &m_narrativeNodes.back();
}

NarCommand *saveandload::addOnUnlock(NarNode *node, QString cmdId, QString cmdDescription)
{
    NarCommand newCmd;
    newCmd.command = cmdId;
    newCmd.description = cmdDescription;

    node->onUnlockCommands.push_back(newCmd);
    return &node->onUnlockCommands.back(); //if a problem
}

NarCommand *saveandload::addOnFail(NarNode *node, QString cmdId, QString cmdDescription)
{
    NarCommand newCmd;
    newCmd.command = cmdId;
    newCmd.description = cmdDescription;

    node->onFailCommands.push_back(newCmd);
    return &node->onFailCommands.back(); //if a problem
}

NarCommand *saveandload::addOnUnlocked(NarNode *node, QString cmdId, QString cmdDescription)
{
    NarCommand newCmd;
    newCmd.command = cmdId;
    newCmd.description = cmdDescription;

    node->onUnlockedCommands.push_back(newCmd);
    return &node->onUnlockedCommands.back(); //if a problem
}

void saveandload::addParameterToCommand(NarCommand *cmd, QString paramID, QString paramVal)
{
    SimpleNode newParam;
    newParam.id = paramID;
    newParam.description = paramVal;

    cmd->params.push_back(newParam);
}

void saveandload::addStoryTagToNarrativeNode(NarNode *node, QString storyTag)
{
    node->storyTags.push_back(storyTag);
}

NarRequirements *saveandload::addRequirementToNarrativeNode(NarNode *node, QString type, QString id)
{
    node->requirements.id = id;

    if(type.toUpper() == "SEQ")
    {
        node->requirements.type = REQ_SEQ;
    }
    else
        if(type.toUpper() == "LEAF")
        {
            node->requirements.type = REQ_LEAF;
        }
        else
            if(type.toUpper() == "INV")
            {
                node->requirements.type = REQ_INV;
            }

    return &node->requirements;
}

NarRequirements *saveandload::addChildRequirement(NarRequirements *req, QString type, QString id)
{
    NarRequirements newReq;
    newReq.id = id;

    if(type.toUpper() == "SEQ")
    {
        newReq.type = REQ_SEQ;
    }
    else
        if(type.toUpper() == "LEAF")
        {
            newReq.type = REQ_LEAF;
        }
        else
            if(type.toUpper() == "INV")
            {
                newReq.type = REQ_INV;
            }

    req->children.push_back(newReq);
    return &req->children.back(); //if there is a problem
}

void saveandload::LoadNarrativeParamsAndCommands(QWidget *widget)
{
    QString settings;
    QFile file;
    file.setFileName("commandsandparams.json");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        settings = file.readAll();
        file.close();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(settings.toUtf8());
        //qDebug() << jsonDoc.toJson();


        if(jsonDoc.isNull()){
            qDebug()<<"Failed to create JSON doc.";
            exit(2);
        }
        if(!jsonDoc.isObject()){
            qDebug()<<"JSON is not an object.";
            exit(3);
        }

        QJsonObject jsonObject=jsonDoc.object();

        if(jsonDoc.isEmpty()){
            qDebug()<<"JSON object is empty.";
            exit(4);
        }

        LoadParams(jsonObject["params"].toArray());
        LoadCommands(jsonObject["commands"].toArray());

        //qDebug() << "Parameters: " << parameters.size() << " Commands: " << commands.size();
    }
    else
    {
        QMessageBox messageBox;
        messageBox.setFixedSize(500,200);
        messageBox.critical(0,"Error","Command and parameters file could not be loaded, please ensure that it exists.\nApplication will now close.");
        QTimer::singleShot(0, widget, SLOT(close()));
    }

}

bool saveandload::LoadNarrativeFromFile(QWidget *widget)
{
    QString settings;

    QStringList filenames = QFileDialog::getOpenFileNames(widget,
                                                     QObject::tr("Load Narrative File"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)"));

    if(!filenames.isEmpty())
    {
        for (int i =0; i<filenames.count(); i++)
        {
            qDebug() << filenames.at(i);

            QFile file = filenames.at(i);

            if(file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QFileInfo fileInfo(file.fileName());
                QString filename(fileInfo.fileName());//get filename from path
                m_fileNames.push_back(filename);

                settings = file.readAll();
                file.close();

                QJsonDocument jsonDoc = QJsonDocument::fromJson(settings.toUtf8());

                if(jsonDoc.isObject())
                    if(jsonDoc.object().contains("node_list") || jsonDoc.object().contains("blocks"))
                    {
                        QMessageBox messageBox;
                        messageBox.critical(0,"Error","You appear to be opening a compiled narrative graph.");
                        messageBox.setFixedSize(500,200);
                        return false;
                    }

                if(jsonDoc.isNull() || !jsonDoc.isArray() || jsonDoc.isEmpty())
                {
                    QMessageBox messageBox;
                    messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
                    messageBox.setFixedSize(500,200);
                    return false;
                }

                readNodeList(jsonDoc.array(), filename);
            }
            else
            {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
                messageBox.setFixedSize(500,200);
                return false;
            }
        }
    }
    else
    {
        qDebug() << "Load aborted by user";
        return false;
    }

    if(m_narrativeNodes.empty())
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","Empty narrative loaded.");
        messageBox.setFixedSize(500,200);
        return false;
    }

    return true;    //will return true unless error found
}

void saveandload::readNodeList(QJsonArray &jsonNodeList, QString fileName)
{
    foreach (const QJsonValue &value, jsonNodeList)
    {
        m_narrativeNodes.push_back(NarNode());

        NarNode *narNode = &m_narrativeNodes.back();   //get newly created node

        QJsonObject obj = value.toObject();

        if(obj.contains("comments"))
            //qDebug() << obj["id"].toString();
            narNode->comments = obj["comments"].toString();

        if(obj.contains("id"))
            //qDebug() << obj["id"].toString();
            narNode->id = obj["id"].toString();

        narNode->fileName = fileName;

        if(obj.contains("on_unlock") && obj["on_unlock"].isArray())
            //qDebug() << obj["on_unlock"].toString();
            readCommandBlock(obj["on_unlock"].toArray(), narNode->onUnlockCommands);

        if(obj.contains("on_fail") && obj["on_fail"].isArray())
            //qDebug() << obj["on_fail"].toString();
            readCommandBlock(obj["on_fail"].toArray(), narNode->onFailCommands);

        if(obj.contains("on_unlocked") && obj["on_unlocked"].isArray())
            //qDebug() << obj["on_unlocked"].toString();
            readCommandBlock(obj["on_unlocked"].toArray(), narNode->onUnlockedCommands);

        if(obj.contains("requirements") && obj["requirements"].isObject())
            readRequirements(obj["requirements"].toObject(), (*narNode));
        else
            narNode->requirements.type = REQ_NONE; //set requirements to none if it doesn't have any

        if(obj.contains("story_tags") && obj["story_tags"].isArray())
            readStoryTags(obj["story_tags"].toArray(), (*narNode));
    }
}

void saveandload::readRequirements(QJsonObject &requirements, NarNode &node)
{
    if(!requirements["type"].isUndefined())
    {
        //qDebug() << requirements["type"].toString();
        //node.requirements.type = requirements["type"].toString();

        if(requirements["type"].toString().toUpper() == "SEQ")
            node.requirements.type = REQ_SEQ;

        if(requirements["type"].toString().toUpper() == "LEAF")
            node.requirements.type = REQ_LEAF;

        if(requirements["type"].toString().toUpper() == "INV")
            node.requirements.type = REQ_INV;
    }

    if(!requirements["id"].isUndefined())
        //qDebug() << requirements["id"].toString();
        node.requirements.id = requirements["id"].toString();

    if(!requirements["children"].isUndefined() && requirements["children"].isArray())
    {
        QJsonArray childrenArray = requirements["children"].toArray();
        foreach (const QJsonValue &value, childrenArray)
        {
            QJsonObject jsonChild = value.toObject();
            readRequirementsChildren(jsonChild, node.requirements);
        }
    }
}

void saveandload::readRequirementsChildren(QJsonObject &children, NarRequirements &req)
{
    req.children.push_back(NarRequirements());

    NarRequirements *child = &req.children.back();

    if(!children["type"].isUndefined())
    {
        if(children["type"].toString().toUpper() == "SEQ")
            child->type = REQ_SEQ;

        if(children["type"].toString().toUpper() == "LEAF")
            child->type = REQ_LEAF;

        if(children["type"].toString().toUpper() == "INV")
            child->type = REQ_INV;
    }
    else
        child->type = REQ_NONE;

    if(!children["id"].isUndefined())
        //qDebug() << requirements["id"].toString();
        child->id = children["id"].toString();

    if(!children["children"].isUndefined() && children["children"].isArray())
    {
        QJsonArray childrenArray = children["children"].toArray();
        foreach (const QJsonValue &value, childrenArray)
        {
            QJsonObject jsonChild = value.toObject();
            readRequirementsChildren(jsonChild, (*child));
        }
    }
}

void saveandload::readCommandBlock(QJsonArray &jsonCommandBlock, QList<NarCommand> &cmdList)
{
    foreach (const QJsonValue &value, jsonCommandBlock)
    {
        QJsonObject obj = value.toObject();
        //qDebug() << obj["cmd"].toString();

        QString command = obj["cmd"].toString();

        QString description;

        foreach (Command cmd, m_commands)
        {
            if(cmd.id == command)
            {
                description = cmd.label;
                break;
            }
        }

        if(description == "")
            qDebug() << "Warning. " << command << " not found in table.";

        cmdList.push_back(NarCommand());
        NarCommand *narCmd = &cmdList.back();

        narCmd->command = command;
        narCmd->description = description;

        foreach (Command cmd, m_commands)
        {
            if(cmd.id == command)
                foreach (Parameter cmdParam, cmd.commandParams)
                {
                    narCmd->params.push_back(SimpleNode());
                    SimpleNode *param = &narCmd->params.back();

                    param->id = cmdParam.label;

                    if(obj[cmdParam.id].isString())
                        param->description = obj[cmdParam.id].toString();
                    else
                        if(obj[cmdParam.id].isDouble())
                        {
                            if(cmdParam.type == VAL_FLOAT)
                                param->description = QString::number(obj[cmdParam.id].toDouble());
                            else
                                param->description = QString::number(obj[cmdParam.id].toInt());
                        }
                }
        }
    }
}

void saveandload::readStoryTags(QJsonArray &jsonStoryTags, NarNode &node)
{
    foreach (const QJsonValue &value, jsonStoryTags)
    {
        if(value.isString())
        {
            QString tag = value.toString();
            node.storyTags.push_back(tag);
        }
    }
}

void saveandload::SaveNarrativeToFile(QWidget *widget)
{
    foreach (QString fileName, m_fileNames)
    {
        QString windowTitle = "Save narrative graph with filename " + fileName;
        QFile file(QFileDialog::getSaveFileName(widget,
                                                             QObject::tr(windowTitle.toStdString().c_str()), fileName,
                                                            QObject:: tr("JSON File (*.json);;All Files (*)")));

        if(!file.fileName().isEmpty()&& !file.fileName().isNull())
        {
            QJsonArray nodeList;

            foreach (NarNode narNode, m_narrativeNodes)
            {
                if(narNode.fileName == fileName)
                {
                    QJsonObject jsonNode;

                    jsonNode["id"] = narNode.id;

                    if(narNode.requirements.type != REQ_NONE)
                    {
                        QJsonObject requirements;
                        WriteRequirements(narNode.requirements, requirements, "requirements");
                        jsonNode["requirements"] = requirements;
                    }

                    if(!narNode.onUnlockCommands.empty())
                    {
                        QJsonArray onUnlockBlock;
                        WriteCommandBlock(narNode.onUnlockCommands, onUnlockBlock);
                        jsonNode["on_unlock"] = onUnlockBlock;
                    }

                    if(!narNode.onFailCommands.empty())
                    {
                        QJsonArray onFailBlock;
                        WriteCommandBlock(narNode.onFailCommands, onFailBlock);
                        jsonNode["on_fail"] = onFailBlock;
                    }

                    if(!narNode.onUnlockedCommands.empty())
                    {
                        QJsonArray onUnlockedBlock;
                        WriteCommandBlock(narNode.onUnlockedCommands, onUnlockedBlock);
                        jsonNode["on_unlocked"] = onUnlockedBlock;
                    }

                    if(!narNode.storyTags.empty())
                    {
                        QJsonArray storyTagArray;
                        writeStoryTags(narNode.storyTags, storyTagArray);
                        jsonNode["story_tags"] = storyTagArray;
                    }

                    nodeList.push_back(jsonNode);
                }
            }

            QJsonDocument jsonDoc(nodeList);
            //qDebug() << jsonDoc.toJson();

            if(file.open(QFile::WriteOnly))
                file.write(jsonDoc.toJson());
            else
            {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
                messageBox.setFixedSize(500,200);
            }
        }
        else
            qDebug() << "Save of" << fileName << "nodes aborted by user";
    }
}

void saveandload::WriteRequirements(NarRequirements &req, QJsonObject &node, QString objectName)
{
    if(req.type == REQ_SEQ)
        node["type"] = "SEQ";

    if(req.type == REQ_LEAF)
        node["type"] = "LEAF";

    if(req.type == REQ_INV)
        node["type"] = "INV";

    if(req.id != "")
        node["id"] = req.id;

    if(!req.children.empty())
    {
        QJsonArray children;
        foreach (NarRequirements r, req.children)
        {
            QJsonObject child;
            WriteRequirements(r, child, "children");
            children.push_back(child);
        }

        node["children"] = children;
    }
}

void saveandload::WriteCommandBlock(QList<NarCommand> cmd, QJsonArray &block)
{
    foreach (NarCommand nC, cmd)
    {
        QJsonObject com;
        com["cmd"] = nC.command;

        foreach (SimpleNode nCParam, nC.params)
            foreach (Parameter memParam, m_parameters)
            {
                if(nCParam.id == memParam.label)
                {
                    qDebug() << "id " << memParam.id;
                    qDebug() << "data" << nCParam.description;

                    if(memParam.type == VAL_FLOAT)
                    {
                        com[memParam.id] = nCParam.description.toDouble();
                    }
                    else
                        if(memParam.type == VAL_INT)
                        {
                            com[memParam.id] = nCParam.description.toInt();
                        }
                        else
                            if(memParam.type == VAL_STRING)
                            {
                                com[memParam.id] = nCParam.description;
                            }
                    break;
                }
            }
        block.push_back(com);
    }
}

void saveandload::writeStoryTags(QList<QString> storyTags, QJsonArray &tagArray)
{
    foreach (QString tag, storyTags)
    {
        tagArray.push_back(tag);
    }
}

void saveandload::DeleteAllNarrativeItems()
{
    m_narrativeNodes.clear();
    m_fileNames.clear();
}

void saveandload::removeFileName(QString fileName)
{
    m_fileNames.removeOne(fileName);
}
