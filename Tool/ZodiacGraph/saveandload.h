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
    void LoadStoryFromFile(QWidget *widget);
    void SaveStoryToFile(QWidget *widget);

private:
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
};

#endif // SAVEANDLOAD_H
