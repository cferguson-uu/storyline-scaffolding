#ifndef SAVEANDLOAD_H
#define SAVEANDLOAD_H

#include "graphstructures.h"

#include <QFile>
#include <QFileDialog>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QMessageBox>
#include <QObject>

#include <QApplication>
#include <QTimer>

#include "zodiacgraph/node.h"

class saveandload
{
public:
    saveandload();

    //story
    bool LoadStoryFromFile(QWidget *widget);
    void SaveStoryToFile(QWidget *widget);

    //getter functions
    const QString GetStoryName(){return m_storyName;}
    const std::list<SettingItem> GetCharacters(){return m_characters;}
    const std::list<SettingItem> GetLocations(){return m_locations;}
    const std::list<SettingItem> GetTimes(){return m_times;}
    const std::list<Episode> GetEpisodes(){return m_episodes;}
    const std::list<EventGoal> GetEvents(){return m_events;}
    const std::list<EventGoal> GetGoals(){return m_goals;}
    const Resolution GetResolution(){return m_resolution;}

    //setter functions
    void setStoryName(QString name);

    SettingItem *addCharacter(QString id, QString description);
    SettingItem *addLocation(QString id, QString description);
    SettingItem *addTime(QString id, QString description);
    void addDetail(SettingItem *item, QString id, QString description, QString stateId, QString stateDescription);

    EventGoal *addEvent(QString id, QString description, EventGoal* parent = nullptr);
    EventGoal *addGoal(QString id, QString description, EventGoal* parent = nullptr);

    Episode *addEpisode(QString id, QString description, Episode* parent = nullptr, zodiac::StoryNodeType type = zodiac::STORY_PLOT_EPISODE);
    SimpleNodeWithState *addAttempt(QString id, QString description, QString stateId, QString stateDescription, Episode* parent);
    SimpleNodeWithState *addOutcome(QString id, QString description, QString stateId, QString stateDescription, Episode* parent);
    void addSubGoal(QString id, QString description, Episode* parent);

    void addResolutionEvent(QString id, QString description);
    void addResolutionState(QString id, QString description);

    void DeleteAllStoryItems();

    //commands
    void LoadNarrativeParamsAndCommands(QWidget *widget);

    //narrative
    bool LoadNarrativeFromFile(QWidget *widget);
    void SaveNarrativeToFile(QWidget *widget);

    const std::list<Command> GetCommands(){return m_commands;}
    const std::list<Parameter> GetParameters(){return m_parameters;}
    const std::list<NarNode> GetNarrativeNodes(){return m_narrativeNodes;}

    std::list<Command> *GetCommandListPointer(){return &m_commands;}

private:
    //story

    //load functions
    void ReadCharacters(QJsonArray &jsonCharacters);
    void ReadLocations(QJsonArray &jsonLocations);
    void ReadTimes(QJsonArray &jsonTimes);
    void ReadSettingsItem(QJsonObject &jsonCharacter, SettingItem &s);
    void ReadEvents(QJsonArray &jsonEvents);
    void ReadGoals(QJsonArray &jsonGoals);
    void ReadEventGoal(QJsonObject &eventGoal, QString SubItemId, EventGoal &e);
    void ReadSubItem(QJsonObject &jsonSubItem, EventGoal &e, QString SubItemId);
    void ReadEpisodes(QJsonArray &jsonEpisodes);
    void ReadEpisode(QJsonObject jsonSubEpisode, Episode &e);
    void ReadResolution(QJsonObject &jsonResolution);

    //save functions
    void WriteSettingItem(QJsonObject &jsonSetting, std::list<SettingItem> &settingList, QString elementName, const QString &prefix);
    void WriteEpisodes(QJsonObject &jsonPlot, const QString &prefix);
    void WriteEpisode(QJsonArray &jsonEpisodes, const Episode &e, const QString &prefix);
    void WriteEventGoals(QJsonObject &jsonTheme, std::list<EventGoal> &eVList, QString eventGoalId, QString subItemId, const QString &prefix);
    void WriteEventGoal(QJsonArray &jsonEvents, const EventGoal &e, QString subItemId, const QString &prefix);
    void WriteResolution(QJsonObject &jsonResolution);

    QString m_storyName;

    std::list<SettingItem> m_characters;
    std::list<SettingItem> m_locations;
    std::list<SettingItem> m_times;

    std::list<Episode> m_episodes;
    std::list<EventGoal> m_events;
    std::list<EventGoal> m_goals;

    Resolution m_resolution;

    //commands
    std::list<Command> m_commands;
    std::list<Parameter> m_parameters;

    void LoadParams(QJsonArray &jsonParams);
    void LoadCommands(QJsonArray &jsonCommands);

    //narrative
    //load functions
    void readNodeList(QJsonArray &jsonNodeList);
    void readRequirements(QJsonObject &requirements, NarNode &node);
    void readRequirementsChildren(QJsonObject &children, NarRequirements &req);
    void readCommandBlock(QJsonArray &jsonCommandBlock, std::list<NarCommand> &cmdList);

    //save
    void WriteCommandBlock(std::list<NarCommand> cmd, QJsonArray &block);
    void WriteRequirements(NarRequirements &req, QJsonObject &node, QString objectName);

    std::list<NarNode> m_narrativeNodes;
};

#endif // SAVEANDLOAD_H
