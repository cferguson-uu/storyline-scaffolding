#ifndef SEQUENCEMATCHER_H
#define SEQUENCEMATCHER_H

// Author: Geert-Jan Giezeman <G.J.Giezeman@uu.nl>
// Copyright: Utrecht University, 2017

#include <memory>

#include <QObject>
#include <QSet>
#include <QMap>
#include <QPair>
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

struct AnaEvent {
    int action;
    int object;
};

struct AnaIds {
    int get_action_no(QString const &name) ;
    int get_object_no(QString const &name) ;
    QString get_action_string(int num) ;
    QString get_object_string(int num) ;
    QSet<QString> getIgnoredActions() {return m_ignored_actions;}
    AnaIds();
  private:
    QSet<QString> m_ignored_actions;
    QMap<QString,int> m_known_actions;
    int m_next_action_no;
    QMap<QString,int> m_known_objects;
    int m_next_object_no;
    void add_action(QString const &name);
    void add_object(QString const &name);
};

struct AnaHandler {
    QVector<AnaEvent> get_events()
    {
        return m_events;
    }

    void set_events(QJsonArray events)
    {
        foreach (QJsonValue val, events)
        {
            if(val.isObject())
                add_event(val.toObject());
        }
    }

    void add_event(QJsonObject event)
    {
        if(!(event.contains("verb") && event["verb"].isString() && event.contains("object") && event["object"].isString()))
            return; //action doesn't have required fields

        int currentAction = m_ids->get_action_no(event["verb"].toString());
        int currentObject = m_ids->get_object_no(event["object"].toString());

        if(currentAction == -1)
            return; //ignored action, don't add to the list

        m_events.push_back(AnaEvent{ currentAction, currentObject });
    }

    void set_events(QVector<AnaEvent> events)
    {
        m_events = events;
    }

    QJsonArray eventsToJson()
    {
        QJsonArray array;

        foreach (AnaEvent event, m_events)
        {
            QJsonObject obj;
            obj["verb"] = m_ids->get_action_string(event.action);
            obj["object"] = m_ids->get_object_string(event.object);
            array.append(obj);
        }

        return array;
    }

    AnaHandler(std::shared_ptr<AnaIds> ids)
        : m_ids(ids) {}
  private:
    std::shared_ptr<AnaIds> m_ids;
    QVector<AnaEvent> m_events;
};

class AnaCost {
  public:
    template <class Seq2>
    void preprocess(QVector<AnaEvent> const &, Seq2 const &) const
    {}
    double gap_bft() const { return -0.3;}
    double match(AnaEvent const &ev1, AnaEvent const &ev2) const {
        if (ev1.action==ev2.action && ev1.object==ev2.object) {
            return 1.0;
        } else {
            return -0.35;
        }
    }
};

struct PerfectSequence
{
    // Compute the similarity between the ground_truth and itself.
    // This gives the maximal score.
    // fac is used to make percentage scores relative to this maximal score.
    double fac;

    QVector<AnaEvent> sequence;
};

class SequenceMatcher : public QWidget
{
public:
    SequenceMatcher(QWidget *parent = 0);
    QVector<AnaEvent> readEvents(QJsonArray &array, std::shared_ptr<AnaIds> ids);
    AnaEvent readEvent(QJsonObject &object, std::shared_ptr<AnaIds> ids);
    void compareSequencesFromFiles();
    float compareSequences(QVector<AnaEvent> &seq1, QVector<AnaEvent> &seq2, double &fac);
    QJsonArray readSequenceFromFile();
    void updateUserSequence(QJsonObject &latestEventInUserSequence);
    float compareUserandPerfectSequences();
    bool addPerfectSequence(QJsonArray seqArray);
    QJsonArray getPerfectSequences();
    QSet<QString> getIgnoredActions();
    int getNumOfPerfectSequences() {return m_perfectSequences.size();}
    void resetUserSequence();

private:
    std::shared_ptr<AnaIds> m_ids = std::make_shared<AnaIds>();

    QVector<PerfectSequence> m_perfectSequences;
    QVector<AnaEvent> m_userSequence;
};

#endif // SEQUENCEMATCHER_H
