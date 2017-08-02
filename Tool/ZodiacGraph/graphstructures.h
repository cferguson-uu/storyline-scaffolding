#ifndef GRAPHSTRUCTURES_H
#define GRAPHSTRUCTURES_H

#include <QDebug>

//Story graph info begins

static const QString kName_StoryName = "storyName";

static const QString kName_Setting = "setting";
static const QString kName_Characters = "characters";
static const QString kName_Locations = "locations";
static const QString kName_Times = "times";
static const QString kName_Details = "details";

static const QString kName_Theme = "theme";
static const QString kName_Events = "events";
static const QString kName_Goals = "goals";
static const QString kName_SubEvents = "subEvents";
static const QString kName_SubGoals = "subGoals";
static const QString kName_SubEvent = "subEvent";
static const QString kName_SubGoal = "subGoal";

static const QString kName_Plot = "plot";
static const QString kName_Resolution = "resolution";

static const QString kName_States = "states";
static const QString kName_State = "state";

static const QString kName_Episodes = "episodes";
static const QString kName_Attempts = "attempts";
static const QString kName_Outcomes = "outcomes";

static const QString kName_Id = "id";
static const QString kName_Description = "description";

//prefixes
//setting
static const QString kPrefix_Characters = "CHAR";
static const QString kPrefix_Locations = "LOC";
static const QString kPrefix_Times = "TIM";
static const QString kPrefix_Detail = "DET";
static const QString kPrefix_State = "STATE";

//theme
static const QString kPrefix_ThemeEvent = "TEVENT";
static const QString kPrefix_ThemeGoal = "TGOAL";

//plot
static const QString kPrefix_Episode = "EP";
static const QString kPrefix_SubEpisode = "SUBEP";
static const QString kPrefix_SubGoal = "SUBG";
static const QString kPrefix_Attempt = "ATT";
static const QString kPrefix_Outcome = "OUT";

//resolution
static const QString kPrefix_ResolutionEvent = "REVENT";
static const QString kPrefix_ResolutionState = "RSTATE";

struct SimpleNode
{
    QString id;
    QString description;
};

struct SimpleNodeWithState
{
    QString id;
    QString description;
    QString stateID;
    QString stateDescription;
};

struct SettingItem : public SimpleNode  //simple node with state child
{
    std::list<SimpleNodeWithState> details;
};

struct EventGoal : public SimpleNode  //simple node with nested children
{
    std::list<EventGoal> subItems;
};

struct Episode : public SimpleNodeWithState //id and subgoal
{
    std::list<SimpleNodeWithState> attempts;
    std::list<SimpleNodeWithState> outcomes;

    std::list<Episode> attemptSubEpisodes;
    std::list<Episode> outcomeSubEpisodes;
};

struct Resolution
{
    std::list<EventGoal> events;
    std::list<SimpleNode> states;
};

//Story graph info ends

//command info begins

enum ValueType
{
    VAL_FLOAT,
    VAL_INT,
    VAL_STRING
};

struct Parameter
{
    Parameter(QString l, QString i, QString t)
    {
        label = l;
        id = i;

        if( t == "float")
            type = VAL_FLOAT;
        else
            if( t == "int")
                type = VAL_INT;
            else
                if( t == "string")
                    type = VAL_STRING;
                else
                    qDebug() << "Incorrect Value";
    }

    Parameter(QString l, QString i, ValueType t)
    {
        label = l;
        id = i;
        type = t;
    }

    QString label;
    QString id;
    ValueType type;
};

struct Command
{
    Command(QString l, QString i, QString t, std::list<Parameter> cP = std::list<Parameter>())
    {
        label = l;
        id = i;

        if( t == "float")
            type = VAL_FLOAT;
        else
            if( t == "int")
                type = VAL_INT;
            else
                if( t == "string")
                    type = VAL_STRING;
                else
                    qDebug() << "Incorrect Value";

        commandParams = cP;
    }

    Command(QString l, QString i, ValueType t, std::list<Parameter> cP = std::list<Parameter>())
    {
        label = l;
        id = i;
        type = t;
        commandParams = cP;
    }

    QString label;
    QString id;
    ValueType type;
    std::list<Parameter> commandParams;
};

//command info ends

//narrative graph info begins

struct NarrativeCommand
{
    QString command;
    std::list<QString> params;    //list of parameters associated with command
};

struct NarrativeRequirements
{
    QString type;
    QString id;
    std::list<NarrativeRequirements> children;
};

struct NarrativeNode
{
    QString comments;
    QString id;

    NarrativeRequirements requirements;

    std::list<NarrativeCommand> onUnlockCommands;
    std::list<NarrativeCommand> onFailCommands;
    std::list<NarrativeCommand> onUnlockedCommands;
};

//narrative graph info ends

#endif // GRAPHSTRUCTURES_H
