#include "saveandload.h"

saveandload::saveandload()
{

}

void saveandload::LoadStoryFromFile(QWidget *widget)
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

            //on_pushButton_reset_clicked();

            QJsonValue jsonStoryName = jsonObject["storyName"];
            //qDebug() <<  jsonStoryName.toString();
            storyName = jsonStoryName.toString();

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
        }
        else
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
            messageBox.setFixedSize(500,200);
        }
    }
    else
        qDebug() << "Load aborted by user";
}

void saveandload::ReadCharacters(QJsonArray &jsonCharacters)
{
    foreach (const QJsonValue &charValue, jsonCharacters)
    {
        characters.push_back(SettingItem());
        std::list<SettingItem>::iterator charIt = characters.end();
        --charIt;

        ReadSettingsItem(charValue.toObject(), (*charIt));
    }
}

void saveandload::ReadLocations(QJsonArray &jsonLocations)
{
  foreach (const QJsonValue &locValue, jsonLocations)
  {
      locations.push_back(SettingItem());
      std::list<SettingItem>::iterator locIt = locations.end();
      --locIt;

      ReadSettingsItem(locValue.toObject(), (*locIt));
  }
}

void saveandload::ReadTimes(QJsonArray &jsonTimes)
{
    foreach (const QJsonValue &timValue, jsonTimes)
    {
        times.push_back(SettingItem());
        std::list<SettingItem>::iterator timIt = times.end();
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
        s.details.push_back(SimpleNodeWithState());
        std::list<SimpleNodeWithState>::iterator detailIt = (s).details.end();
        --detailIt;

        QJsonObject detail = detailValue.toObject();

        (*detailIt).id = detail[kName_Id].toString().section('_', 1);
        qDebug() << detail[kName_Id].toString().section('_', 1);

        (*detailIt).description = detail[kName_Description].toString();
        qDebug() << detail[kName_Description].toString();

        if(detail.contains(kName_State))
        {
            QJsonObject jsonState = detail[kName_State].toObject();

            qDebug() << jsonState[kName_Id].toString();
            (*detailIt).stateID = jsonState[kName_Id].toString();

            qDebug() << jsonState[kName_Description].toString();
            (*detailIt).stateDescription = jsonState[kName_Description].toString();
        }
    }
}

void saveandload::ReadEvents(QJsonArray &jsonEvents)
{
    foreach (const QJsonValue &eventValue, jsonEvents)
    {
        events.push_back(EventGoal());
        std::list<EventGoal>::iterator evIt = events.end();
        --evIt;

        ReadEventGoal(eventValue.toObject(), kName_SubEvents, (*evIt));
    }
}

void saveandload::ReadGoals(QJsonArray &jsonGoals)
{
    foreach (const QJsonValue &goalValue, jsonGoals)
    {
        goals.push_back(EventGoal());
        std::list<EventGoal>::iterator goIt = goals.end();
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

                std::list<EventGoal>::iterator subEvIt = e.subItems.end();
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

            std::list<EventGoal>::iterator subEvIt = e.subItems.end();
            --subEvIt;

            ReadSubItem(subItemValue.toObject(), (*subEvIt), SubItemId);
        }
    }
}

void saveandload::ReadEpisodes(QJsonArray &jsonEpisodes)
{
    foreach (const QJsonValue &episodeValue, jsonEpisodes)
    {
        episodes.push_back(Episode());
        std::list<Episode>::iterator epIt = episodes.end();
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

    e.stateID = subGoalId.toString().section('_', 1);
    qDebug() << subGoalId.toString();

    e.stateDescription = subGoalDescription.toString();
    qDebug() << subGoalDescription.toString();

    foreach (const QJsonValue &attemptValue, jsonAttempts)
    {
        QJsonObject attempt = attemptValue.toObject();

        if(attempt[kName_Id].toString().section('_', 0, 0) == kPrefix_SubEpisode)  //handle sub episode
        {
            e.attemptSubEpisodes.push_back(Episode());
            std::list<Episode>::iterator subEpIt = e.attemptSubEpisodes.end();
            --subEpIt;
            ReadEpisode(attempt, (*subEpIt));
        }
        else    //handle attempt as normal
        {
            e.attempts.push_back(SimpleNodeWithState());
            std::list<SimpleNodeWithState>::iterator attIt = e.attempts.end();
            --attIt;

            qDebug() << attempt[kName_Id].toString().section('_', 1);
            (*attIt).id = attempt[kName_Id].toString().section('_', 1);

            qDebug() << attempt[kName_Description].toString();
            (*attIt).description = attempt[kName_Description].toString();

            if(attempt.contains(kName_State))
            {
                QJsonObject jsonState = attempt[kName_State].toObject();

                qDebug() << jsonState[kName_Id].toString();
                (*attIt).stateID = jsonState[kName_Id].toString();

                qDebug() << jsonState[kName_Description].toString();
                (*attIt).stateDescription = jsonState[kName_Description].toString();
            }
        }
    }

    foreach (const QJsonValue &outcomeValue, jsonOutcomes)
    {
        QJsonObject outcome = outcomeValue.toObject();

        if(outcome[kName_Id].toString().section('_', 0, 0) == "SUBEP")  //handle sub episode
        {
            e.outcomeSubEpisodes.push_back(Episode());
            std::list<Episode>::iterator subEpIt = e.outcomeSubEpisodes.end();
            --subEpIt;
            ReadEpisode(outcome, (*subEpIt));
        }
        else    //handle attempt as normal
        {
            e.outcomes.push_back(SimpleNodeWithState());
            std::list<SimpleNodeWithState>::iterator outIt = e.outcomes.end();
            --outIt;

            qDebug() << outcome[kName_Id].toString().section('_', 1);
            (*outIt).id = outcome[kName_Id].toString().section('_', 1);

            qDebug() << outcome[kName_Description].toString();
            (*outIt).description = outcome[kName_Description].toString();

            if(outcome.contains(kName_State))
            {
                QJsonObject jsonState = outcome[kName_State].toObject();

                qDebug() << jsonState[kName_Id].toString().section('_', 1);
                (*outIt).stateID = jsonState[kName_Id].toString().section('_', 1);

                qDebug() << jsonState[kName_Description].toString();
                (*outIt).stateDescription = jsonState[kName_Description].toString();
            }
        }
    }
}

void saveandload::ReadResolution(QJsonObject &jsonResolution)
{
    QJsonObject::iterator it;

    QJsonArray jsonEvents = jsonResolution[kName_Events].toArray();

    foreach (const QJsonValue &eventValue, jsonEvents)
    {
        resolution.events.push_back(EventGoal());
        std::list<EventGoal>::iterator evIt = resolution.events.end();
        --evIt;

        QJsonObject jsonEvent = eventValue.toObject();

        (*evIt).id = jsonEvent[kName_Id].toString().section('_', 1);
        qDebug() << jsonEvent[kName_Id].toString().section('_', 1);

        (*evIt).description = jsonEvent[kName_Description].toString();
        qDebug() << jsonEvent[kName_Description].toString();
    }

    QJsonArray jsonStates = jsonResolution[kName_States].toArray();

    foreach (const QJsonValue &stateValue, jsonStates)
    {
       resolution.states.push_back(SimpleNode());
        std::list<SimpleNode>::iterator stIt = resolution.states.end();
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
        jsonData["storyName"] = storyName;

        QJsonObject jsonSetting;
        if(!characters.empty())
            WriteSettingItem(jsonSetting, characters, "characters", kPrefix_Characters);
        if(!locations.empty())
            WriteSettingItem(jsonSetting, locations, "locations", kPrefix_Locations);
        if(!times.empty())
            WriteSettingItem(jsonSetting, times, "times", kPrefix_Times);
        jsonData["setting"] = jsonSetting;

        QJsonObject jsonTheme;
        WriteEventGoals(jsonTheme, events, "events", "subEvents", kPrefix_ThemeEvent);
        WriteEventGoals(jsonTheme, goals, "goals", "subGoals", kPrefix_ThemeGoal);
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

void saveandload::WriteSettingItem(QJsonObject &jsonSetting, std::list<SettingItem> &settingList, QString elementName, const QString &prefix)
{
    QJsonArray jsonSettingItems;

    for(std::list<SettingItem>::iterator it = settingList.begin(); it != settingList.end(); ++it)
    {
        QJsonObject jsonSettingItem;
        jsonSettingItem[kName_Id] = QString(prefix + "_" + (*it).id);
        jsonSettingItem[kName_Description] = (*it).description;

        if(!(*it).details.empty())
        {
            QJsonArray jsonDetails;

            for(std::list<SimpleNodeWithState>::iterator it2 = (*it).details.begin(); it2 != (*it).details.end(); ++it2)
            {
                QJsonObject jsonDetail;

                jsonDetail[kName_Id] = QString(kPrefix_Detail + "_" + (*it2).id);
                jsonDetail[kName_Description] = (*it2).description;

                if((*it2).stateID.length() > 0)
                {
                    QJsonObject jsonState;
                    jsonState[kName_Id] = QString(kPrefix_State + "_" + (*it2).stateID);
                    jsonState[kName_Description] = (*it2).stateDescription;
                    jsonDetail[kName_State] = jsonState;
                }

                jsonDetails.append(jsonDetail);


            }

            jsonSettingItem[kName_Details] = jsonDetails;
        }

        jsonSettingItems.append(jsonSettingItem);
    }

    jsonSetting[elementName] = jsonSettingItems;
}

void saveandload::WriteEventGoals(QJsonObject &jsonTheme, std::list<EventGoal> &eVList, QString eventGoalId, QString subItemId, const QString &prefix)
{
    if(!eVList.empty())
    {
        QJsonArray jsonEventGoals;
        for(std::list<EventGoal>::iterator it = eVList.begin(); it != eVList.end(); ++it)
        {
                WriteEventGoal(jsonEventGoals, (*it), subItemId, prefix);
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

        for(std::list<EventGoal>::const_iterator it = e.subItems.begin(); it != e.subItems.end(); ++it)
        {
            WriteEventGoal(jsonSubEvents, (*it), subItemId, prefix);
        }

        jsonEvent[subItemId] = jsonSubEvents;
    }

    jsonEvent[kName_Id] = QString(prefix + "_" + e.id);
    jsonEvent[kName_Description] = e.description;

    jsonEvents.append(jsonEvent);
}

void saveandload::WriteEpisodes(QJsonObject &jsonPlot, const QString &prefix)
{
    if(!episodes.empty())
    {
        QJsonArray jsonEpisodes;

        for(std::list<Episode>::iterator it = episodes.begin(); it != episodes.end(); ++it)
        {
                WriteEpisode(jsonEpisodes, (*it), prefix);
        }

        jsonPlot[kName_Episodes] = jsonEpisodes;
    }
}

void saveandload::WriteEpisode(QJsonArray &jsonEpisodes, const Episode &e, const QString &prefix)
{
    QJsonObject jsonEpisode;

    jsonEpisode[kName_Id] = QString(prefix + "_" + e.id);
    jsonEpisode[kName_Description] = e.description;

    jsonEpisode[kName_SubGoal] = e.stateID; //SHOULD BE ID DESCRIPTION PAIR
    //jsonEpisode[kName_SubGoal] = e.stateDescription; //SHOULD BE ID DESCRIPTION PAIR

    if(!e.attempts.empty() || !e.attemptSubEpisodes.empty())
    {
        QJsonArray jsonAttempts;

        for(std::list<SimpleNodeWithState>::const_iterator it = e.attempts.begin(); it != e.attempts.end(); ++it)
        {
            QJsonObject jsonAttempt;

            jsonAttempt[kName_Id] = QString(kPrefix_Attempt + "_" + (*it).id);
            jsonAttempt[kName_Description] = (*it).description;

            if(!(*it).stateID.isEmpty())
            {
                QJsonObject jsonState;
                jsonState[kName_Id] = QString(kPrefix_State + "_" + (*it).stateID);
                jsonState[kName_Description] = (*it).stateDescription;
                jsonAttempt[kName_State] = jsonState;
            }

            jsonAttempts.append(jsonAttempt);
        }

        for(std::list<Episode>::const_iterator it2 = e.attemptSubEpisodes.begin(); it2 != e.attemptSubEpisodes.end(); ++it2)
        {
            WriteEpisode(jsonAttempts, (*it2), kPrefix_SubEpisode);
        }

         jsonEpisode[kName_Attempts] = jsonAttempts;
    }

    if(!e.outcomes.empty() || !e.outcomeSubEpisodes.empty())
    {
        QJsonArray jsonOutcomes;

        for(std::list<SimpleNodeWithState>::const_iterator it = e.outcomes.begin(); it != e.outcomes.end(); ++it)
        {
            QJsonObject jsonOutcome;
            jsonOutcome[kName_Id] = QString(kPrefix_Outcome + "_" +(*it).id);
            jsonOutcome[kName_Description] = (*it).description;

            if(!(*it).stateID.isEmpty())
            {
                QJsonObject jsonState;
                jsonState[kName_Id] = QString(kPrefix_State + "_" + (*it).stateID);
                jsonState[kName_Description] = (*it).stateDescription;
                jsonOutcome[kName_State] = jsonState;
            }

            jsonOutcomes.append(jsonOutcome);
        }

        for(std::list<Episode>::const_iterator it2 = e.outcomeSubEpisodes.begin(); it2 != e.outcomeSubEpisodes.end(); ++it2)
        {
            WriteEpisode(jsonOutcomes, (*it2), kPrefix_SubEpisode);
        }

        jsonEpisode[kName_Outcomes] = jsonOutcomes;
    }
        jsonEpisodes.append(jsonEpisode);
}

void saveandload::WriteResolution(QJsonObject &jsonResolution)
{
    if(!resolution.events.empty())
    {
        QJsonArray jsonEvents;

        for(std::list<EventGoal>::iterator it = resolution.events.begin(); it != resolution.events.end(); ++it)
        {
            QJsonObject jsonEvent;

            jsonEvent[kName_Id] = QString(kPrefix_ResolutionEvent + "_" + (*it).id);
            jsonEvent[kName_Description] = (*it).description;

            jsonEvents.append(jsonEvent);
        }

        jsonResolution[kName_Events] = jsonEvents;
    }

    if(!resolution.states.empty())
    {
        QJsonArray jsonStates;

        for(std::list<SimpleNode>::iterator it = resolution.states.begin(); it != resolution.states.end(); ++it)
        {
            QJsonObject jsonState;

            jsonState[kName_Id] = (*it).id;
            jsonState[kName_Description] = (*it).description;

            jsonStates.append(jsonState);
        }

        jsonResolution[kName_States] = jsonStates;
    }
}

void saveandload::LoadParams(QJsonArray &jsonParams)
{
    foreach (const QJsonValue & value, jsonParams)
    {
        QJsonObject obj = value.toObject();

        parameters.push_back(Parameter(obj["label"].toString(), obj["id_name"].toString(), obj["type"].toString()));
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
        std::list<Parameter> params;

        foreach (const QJsonValue &value2, jsonCommandParams)
        {
            //qDebug() << value2.toString();
            for(std::list<Parameter>::iterator it = parameters.begin(); it != parameters.end(); ++it)
            {
                if((*it).id == value2.toString())
                {
                    params.push_back((*it));
                    break;
                }
            }
        }

        commands.push_back(Command(obj["label"].toString(), obj["id_name"].toString(), obj["type"].toString(), params));
    }
}

void saveandload::LoadNarrativeParamsAndCommands()
{
    QString settings;
    QFile file;
    file.setFileName("commandsandparams.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
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

void saveandload::ReadNarrativeFromFile(QWidget *widget)
{
    QString settings;
    QFile file;

    //file.setFileName("narrative.json");

    file.setFileName(QFileDialog::getOpenFileName(widget,
                                                     QObject::tr("Load Narrative File"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty()&& !file.fileName().isNull())
    {
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            settings = file.readAll();
            file.close();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(settings.toUtf8());
            //qDebug() << "json: " << jsonDoc.toJson();


            if(jsonDoc.isNull()){
                qDebug()<<"Failed to create JSON doc.";
                exit(2);
            }
            if(!jsonDoc.isArray()){
                qDebug()<<"JSON is not an array.";
                exit(3);
            }

            QJsonArray jsonArray=jsonDoc.array();

            if(jsonArray.isEmpty()){
                qDebug()<<"JSON array is empty.";
                exit(4);
            }

            /*if(jsonObject.contains("node_list") || jsonObject.contains("blocks"))
            {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","You appear to be opening a compiled narrative graph.");
                messageBox.setFixedSize(500,200);
            }
            else
            {

            }*/

            readNodeList(jsonArray);
        }
        else
        {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","File could not be loaded, please ensure that it is the correct format.");
                messageBox.setFixedSize(500,200);
        }
    }
    else
        qDebug() << "Load aborted by user";
}

void saveandload::readNodeList(QJsonArray &jsonNodeList)
{
    foreach (const QJsonValue &value, jsonNodeList)
    {
        narrativeNodes.push_back(NarrativeNode());

        std::list<NarrativeNode>::iterator it = narrativeNodes.end();
        --it;

        QJsonObject obj = value.toObject();

        if(obj.contains("comments"))
            //qDebug() << obj["id"].toString();
            (*it).comments = obj["comments"].toString();

        if(obj.contains("id"))
            //qDebug() << obj["id"].toString();
            (*it).id = obj["id"].toString();

        if(obj.contains("on_unlock") && obj["on_unlock"].isArray())
            //qDebug() << obj["on_unlock"].toString();
            readCommandBlock(obj["on_unlock"].toArray(), (*it).onUnlockCommands);

        if(obj.contains("on_fail") && obj["on_fail"].isArray())
            //qDebug() << obj["on_fail"].toString();
            readCommandBlock(obj["on_fail"].toArray(), (*it).onFailCommands);

        if(obj.contains("on_unlocked") && obj["on_unlocked"].isArray())
            //qDebug() << obj["on_unlocked"].toString();
            readCommandBlock(obj["on_unlocked"].toArray(), (*it).onUnlockedCommands);

        if(obj.contains("requirements") && obj["requirements"].isObject())
            readRequirements(obj["requirements"].toObject(), (*it));
    }
}

void saveandload::readRequirements(QJsonObject &requirements, NarrativeNode &node)
{
    if(!requirements["type"].isUndefined())
        //qDebug() << requirements["type"].toString();
        node.requirements.type = requirements["type"].toString();

    if(!requirements["id"].isUndefined())
        //qDebug() << requirements["id"].toString();
        node.requirements.id = requirements["id"].toString();

    if(!requirements["children"].isUndefined() && requirements["children"].isArray())
    {
        QJsonArray childrenArray = requirements["children"].toArray();
        foreach (const QJsonValue &value, childrenArray)
        {
            QJsonObject child = value.toObject();
            readRequirementsChildren(child, node.requirements);
        }
    }
}

void saveandload::readRequirementsChildren(QJsonObject &children, NarrativeRequirements &req)
{
    req.children.push_back(NarrativeRequirements());

    std::list<NarrativeRequirements>::iterator it = req.children.end();
    --it;

    if(!children["type"].isUndefined())
        //qDebug() << requirements["type"].toString();
        (*it).type = children["type"].toString();

    if(!children["id"].isUndefined())
        //qDebug() << requirements["id"].toString();
        (*it).id = children["id"].toString();

    if(!children["children"].isUndefined() && children["children"].isArray())
    {
        QJsonArray childrenArray = children["children"].toArray();
        foreach (const QJsonValue &value, childrenArray)
        {
            QJsonObject child = value.toObject();
            readRequirementsChildren(child, (*it));
        }
    }
}

void saveandload::readCommandBlock(QJsonArray &jsonCommandBlock, std::list<NarrativeCommand> &cmdList)
{
    foreach (const QJsonValue &value, jsonCommandBlock)
    {
        QJsonObject obj = value.toObject();
        //qDebug() << obj["cmd"].toString();

        QString command = obj["cmd"].toString();

        cmdList.push_back(NarrativeCommand());
        std::list<NarrativeCommand>::iterator cmdIt = cmdList.end();
        --cmdIt;

        (*cmdIt).command = command;

        for(std::list<Command>::iterator it = commands.begin(); it != commands.end(); ++it)
        {
            if((*it).id == command)
            {
                for(std::list<Parameter>::iterator it2 = (*it).commandParams.begin(); it2 != (*it).commandParams.end(); ++it2)
                {
                    //qDebug() << (*it2).id;
                    (*cmdIt).params.push_back(SimpleNode());
                    std::list<SimpleNode>::iterator paramIt = (*cmdIt).params.end();
                    --paramIt;

                    (*paramIt).id = (*it2).label;

                    if(obj[(*it2).id].isString())
                        //qDebug() << obj[(*it2).id].toString();
                        (*paramIt).description = obj[(*it2).id].toString();
                    else
                        if(obj[(*it2).id].isDouble())
                        {
                            //qDebug() << obj[(*it2).id].toDouble();
                            if((*it2).type == VAL_FLOAT)
                                (*paramIt).description = QString::number(obj[(*it2).id].toDouble());
                            else
                                (*paramIt).description = QString::number(obj[(*it2).id].toInt());
                        }
                }
            }
        }
    }
}

void saveandload::WriteNarrativeToFile(QWidget *widget)
{
    QFile file(QFileDialog::getSaveFileName(widget,
                                                         QObject::tr("Load Story Graph"), "",
                                                        QObject:: tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty()&& !file.fileName().isNull())
    {

        QJsonObject jsonData;
        QJsonArray nodeList;
        //QJsonArray blocks;

        for(std::list<NarrativeNode>::iterator it = narrativeNodes.begin(); it != narrativeNodes.end(); ++it)
        {
            QJsonObject node;

            node["id"] = (*it).id;

            if(!((*it).requirements.id.isEmpty() && (*it).requirements.type.isEmpty() && (*it).requirements.children.empty()))
            {
                QJsonObject requirements;
                WriteRequirements((*it).requirements, requirements, "requirements");
                node["requirements"] = requirements;
            }

            if(!(*it).onUnlockCommands.empty())
            {
                QJsonArray onUnlockBlock;
                WriteCommandBlock((*it).onUnlockCommands, onUnlockBlock);
                node["on_unlock"] = onUnlockBlock;
            }

            if(!(*it).onFailCommands.empty())
            {
                QJsonArray onFailBlock;
                WriteCommandBlock((*it).onFailCommands, onFailBlock);
                node["on_fail"] = onFailBlock;
            }

            if(!(*it).onUnlockedCommands.empty())
            {
                QJsonArray onUnlockedBlock;
                WriteCommandBlock((*it).onUnlockedCommands, onUnlockedBlock);
                node["on_unlocked"] = onUnlockedBlock;
            }

            nodeList.push_back(node);
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
        qDebug() << "Save aborted by user";
}

void saveandload::WriteRequirements(NarrativeRequirements &req, QJsonObject &node, QString objectName)
{
    if(!req.type.isEmpty())
        node["type"] = req.type;

    if(!req.id.isEmpty())
        node["id"] = req.id;

    if(!req.children.empty())
    {
        QJsonArray children;
        for(std::list<NarrativeRequirements>::iterator it = req.children.begin(); it != req.children.end(); ++it)
        {
            QJsonObject child;
            WriteRequirements((*it), child, "children");
            children.push_back(child);
        }

        node["children"] = children;
    }
}

void saveandload::WriteCommandBlock(std::list<NarrativeCommand> cmd, QJsonArray &block)
{
    for(std::list<NarrativeCommand>::iterator it = cmd.begin(); it != cmd.end(); ++it)
    {
        QJsonObject com;
        com["cmd"] = (*it).command;

        for(std::list<SimpleNode>::iterator it2 = (*it).params.begin(); it2 != (*it).params.end(); ++it2)
        {
            for(std::list<Parameter>::iterator it3 = parameters.begin(); it3 != parameters.end(); ++it3)
            {
                if((*it2).id == (*it3).label)
                {
                    qDebug() << "id " << (*it3).id;
                    qDebug() << "data" << (*it2).description;

                    if((*it3).type == VAL_FLOAT)
                    {
                        com[(*it3).id] = (*it2).description.toDouble();
                    }
                    else
                        if((*it3).type == VAL_INT)
                        {
                            com[(*it3).id] = (*it2).description.toInt();
                        }
                        else
                            if((*it3).type == VAL_STRING)
                            {
                                com[(*it3).id] = (*it2).description;
                            }
                    break;
                }
            }
        }
        block.push_back(com);
    }
}
