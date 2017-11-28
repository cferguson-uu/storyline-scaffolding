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
#include <QFile>
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
        qDebug() << "original size: " << events.size();
        int i = 0;
        int currentAction, currentObject;
        foreach (QJsonValue val, events)
        {
            if(val.isObject())
            {
                QJsonObject obj = val.toObject();

                if(!(obj.contains("verb") && obj["verb"].isString() && obj.contains("object") && obj["object"].isString()))
                    continue; //action doesn't have required fields

                currentAction = m_ids->get_action_no(obj["verb"].toString());
                currentObject = m_ids->get_object_no(obj["object"].toString());

                if(currentAction == -1)
                    continue; //ignored action
                else
                    ++i;

                m_events.push_back(AnaEvent{ currentAction, currentObject });
            }
            else
                qDebug () << "ff";
        }
        qDebug() << "new size: " << i;
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
    double gap_bft() const { return -0.1;}
    double match(AnaEvent const &ev1, AnaEvent const &ev2) const {
        if (ev1.action==ev2.action && ev1.object==ev2.object) {
            return 1.0;
        } else {
            return -0.1;
        }
    }
};

class SequenceMatcher
{
public:
    SequenceMatcher();
    QVector<AnaEvent> read_events(QFile &file, std::shared_ptr<AnaIds> ids);
};

#endif // SEQUENCEMATCHER_H
