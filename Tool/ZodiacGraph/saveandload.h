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

class saveandload
{
public:
    saveandload();

    //story
    void LoadStoryFromFile(QWidget *widget);
    void SaveStoryToFile(QWidget *widget);

    //commands
    void LoadNarrativeParamsAndCommands();

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

    QString storyName;

    std::list<SettingItem> characters;
    std::list<SettingItem> locations;
    std::list<SettingItem> times;

    std::list<Episode> episodes;
    std::list<EventGoal> events;
    std::list<EventGoal> goals;

    Resolution resolution;

    //commands
    std::list<Command> commands;
    std::list<Parameter> parameters;

    void LoadParams(QJsonArray &jsonParams);
    void LoadCommands(QJsonArray &jsonCommands);

    //narrative
    //load functions
    void readNodeList(QJsonArray &jsonNodeList);
    void readRequirements(QJsonObject &requirements, NarrativeNode &node);
    void readRequirementsChildren(QJsonObject &children, NarrativeRequirements &req);
    void readCommandBlock(QJsonArray &jsonCommandBlock, std::list<NarrativeCommand> &cmdList, int &numOfOnUnlockCmds);

    //save
    void WriteCommandBlock(std::list<NarrativeCommand> cmd, QJsonArray &block);
    void WriteRequirements(NarrativeRequirements &req, QJsonObject &node, QString objectName);

    std::list<NarrativeNode> narrativeNodes;
};

#endif // SAVEANDLOAD_H
