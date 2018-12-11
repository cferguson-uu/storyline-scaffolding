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
    const QList<SettingItem> GetCharacters(){return m_characters;}
    const QList<SettingItem> GetLocations(){return m_locations;}
    const QList<SettingItem> GetTimes(){return m_times;}
    const QList<Episode> GetEpisodes(){return m_episodes;}
    const QList<EventGoal> GetEvents(){return m_events;}
    const QList<EventGoal> GetGoals(){return m_goals;}
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
    SimpleNode *addAttempt(QString id, QString description, Episode* parent);
    SimpleNode *addOutcome(QString id, QString description, Episode* parent);
    void addSubGoal(QString id, QString description, Episode* parent);

    EventGoal *addResolutionEvent(QString id, QString description, EventGoal* parent);
    void addResolutionState(QString id, QString description);

    void DeleteAllStoryItems();

    //commands
    void LoadNarrativeParamsAndCommands(QWidget *widget);

    //narrative
    bool LoadNarrativeFromFile(QWidget *widget);
    void SaveNarrativeToFile(QWidget *widget);

    //get functions
    const QList<Command> GetCommands(){return m_commands;}
    const QList<Parameter> GetParameters(){return m_parameters;}
    const QList<NarNode> GetNarrativeNodes(){return m_narrativeNodes;}
    QList<Command> *GetCommandListPointer(){return &m_commands;}

    //setter functions
    NarNode *addNarrativeNode(QString id, QString description, QString fileName);

    NarCommand *addOnUnlock(NarNode *node, QString cmdId, QString cmdDescription);
    NarCommand *addOnFail(NarNode *node, QString cmdId, QString cmdDescription);
    NarCommand *addOnUnlocked(NarNode *node, QString cmdId, QString cmdDescription);
    void addParameterToCommand(NarCommand *cmd, QString paramID, QString paramVal);

    void addStoryTagToNarrativeNode(NarNode *node, QString storyTag);

    NarRequirements *addRequirementToNarrativeNode(NarNode *node, QString type, QString id = "");
    NarRequirements *addChildRequirement(NarRequirements *req, QString type, QString id = "");

    void DeleteAllNarrativeItems();

    inline QVector<QString> getFileNames(){return m_fileNames;}
    void removeFileName(QString fileName);

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
    void WriteSettingItem(QJsonObject &jsonSetting, QList<SettingItem> &settingList, QString elementName, const QString &prefix);
    void WriteEpisodes(QJsonObject &jsonPlot, const QString &prefix);
    void WriteEpisode(QJsonArray &jsonEpisodes, const Episode &e, const QString &prefix);
    void WriteEventGoals(QJsonObject &jsonTheme, QList<EventGoal> &eVList, QString eventGoalId, QString subItemId, const QString &prefix);
    void WriteEventGoal(QJsonArray &jsonEvents, const EventGoal &e, QString subItemId, const QString &prefix);
    void WriteResolution(QJsonObject &jsonResolution);

    QString m_storyName;

    QList<SettingItem> m_characters;
    QList<SettingItem> m_locations;
    QList<SettingItem> m_times;

    QList<Episode> m_episodes;
    QList<EventGoal> m_events;
    QList<EventGoal> m_goals;

    Resolution m_resolution;

    //commands
    QList<Command> m_commands;
    QList<Parameter> m_parameters;

    void LoadParams(QJsonArray &jsonParams);
    void LoadCommands(QJsonArray &jsonCommands);

    //narrative
    //load functions
    void readNodeList(QJsonArray &jsonNodeList, QString fileName);
    void readRequirements(QJsonObject &requirements, NarNode &node);
    void readRequirementsChildren(QJsonObject &children, NarRequirements &req);
    void readCommandBlock(QJsonArray &jsonCommandBlock, QList<NarCommand> &cmdList);
    void readStoryTags(QJsonArray &jsonStoryTags, NarNode &node);

    //save
    void WriteCommandBlock(QList<NarCommand> cmd, QJsonArray &block);
    void WriteRequirements(NarRequirements &req, QJsonObject &node, QString objectName);
    void writeStoryTags(QList<QString> storyTags, QJsonArray &tagArray);

    QList<NarNode> m_narrativeNodes;

    QVector<QString> m_fileNames;
};

#endif // SAVEANDLOAD_H
