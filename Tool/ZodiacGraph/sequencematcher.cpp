#include "sequencematcher.h"
#include "musaline/musaline.hpp"

// Construct an aligner to compute the similarity between the sequences.
musaline::LinearAligner<AnaCost> m_aligner;

SequenceMatcher::SequenceMatcher(QWidget *parent)
    :QWidget(parent)
{

}

void SequenceMatcher::compareSequencesFromFiles()
{
    // Read the two files.
    auto events1 = readEvents(readSequenceFromFile(), m_ids);
    auto events2 = readEvents(readSequenceFromFile(), m_ids);

    // Compute the similarity between the ground_truth and itself.
    // This gives the maximal score.
    // fac is used to make percentage scores relative to this maximal score.
    double self_benefit = m_aligner.global_align(events1, events1).score;
    double fac = 100.0 / self_benefit;

    // Compute the score as a percentage of the maximal score after each step of the user.
    qDebug() << "In incremental steps:\n";
    for (auto cur = events2.begin(); true; ++cur) {
        boost::sub_range<QVector<AnaEvent> > sub(events2.begin(), cur);
        double benefit = m_aligner.cut_first_end_off_align(events1, sub).score;
        qDebug() << "Score is: " << fac*benefit << "%\n";
        if (cur == events2.end())
            break;
    }
}

void SequenceMatcher::loadPerfectSequence(QJsonArray seqArray)
{
    // Read the two files.
    m_perfectSequence = readEvents(seqArray, m_ids);

    m_self_benefit = m_aligner.global_align(m_perfectSequence, m_perfectSequence).score;
    m_fac = 100.0 / m_self_benefit;
}

float SequenceMatcher::compareLatestUserSequence(QJsonObject &latestEventInUserSequence)
{
    AnaEvent event = readEvent(latestEventInUserSequence, m_ids);

    if(event.action == -1 && event.object == -1)
        return -1;  //event was not valid, ignore it

    m_userSequence.push_back(event);

    for (auto cur = m_userSequence.begin(); true; ++cur)
    {
        boost::sub_range<QVector<AnaEvent> > sub(m_userSequence.begin(), cur);
        double benefit = m_aligner.cut_first_end_off_align(m_perfectSequence, sub).score;
        //qDebug() << "Score is: " << m_fac*benefit << "%\n";
        if (cur == m_userSequence.end())
            return m_fac*benefit;
    }
}

QJsonArray SequenceMatcher::readSequenceFromFile()
{
    QFile file(QFileDialog::getOpenFileName(this,
                                                     QObject::tr("Load Sequence"), "",
                                                     QObject::tr("JSON File (*.json);;All Files (*)")));

    if(!file.fileName().isEmpty()&& !file.fileName().isNull())
    {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug()<<"Cannot open file "<<file.fileName()<<"\n";
            return QJsonArray();
        }
        QString settings = file.readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(settings.toUtf8());

        if(jsonDoc.isArray())
            return jsonDoc.array();
        else
        {
            qDebug() << "JSON file" << file.fileName() << " is invalid";
            return QJsonArray();
        }
    }
    else
    {
        qDebug() << "Load aborted by user";
        return QJsonArray();
    }
}

QVector<AnaEvent> SequenceMatcher::readEvents(
        QJsonArray &array, std::shared_ptr<AnaIds> ids)
{
    AnaHandler handler(ids);
    handler.set_events(array);
    QVector<AnaEvent> events = handler.get_events();
    return events;
}

AnaEvent SequenceMatcher::readEvent(QJsonObject &object, std::shared_ptr<AnaIds> ids)
{
    AnaHandler handler(ids);
    handler.add_event(object);
    QVector<AnaEvent> events = handler.get_events();

    if(events.size() == 1)
        return events.front();
    else
        return AnaEvent{-1, -1};
}

QSet<QString> SequenceMatcher::getIgnoredActions()
{
    return m_ids->getIgnoredActions();
}

QJsonArray SequenceMatcher::getPerfectSequence()
{
    AnaHandler handler(m_ids);
    handler.set_events(m_perfectSequence);
    return handler.eventsToJson();
}

AnaIds::AnaIds()
    : m_next_action_no(1), m_next_object_no(1)
{
    m_ignored_actions.insert("looked at");
    m_ignored_actions.insert("stopped looking at");
    m_ignored_actions.insert("dropped");
    m_ignored_actions.insert("attempted");
    add_action("jumped to");
    add_action("picked up");
    add_action("examined");
}

void AnaIds::add_action(QString const &name)
{
    m_known_actions.insert(name, m_next_action_no++);
}

void AnaIds::add_object(QString const &name)
{
    m_known_objects.insert(name, m_next_object_no++);
}

int AnaIds::get_action_no(QString const &name)
{
    auto it = m_known_actions.find(name);
    if (it != m_known_actions.end()) {
        return it.value();
    }
    if (m_ignored_actions.find(name) != m_ignored_actions.end()) {
        return -1;
    }
    int result = m_next_action_no;
    qDebug() <<"Adding action "<<name<<"\n";
    add_action(name);
    return result;
}

QString AnaIds::get_action_string(int num)
{
    for(QMap<QString,int>::iterator actionIt = m_known_actions.begin(); actionIt != m_known_actions.end(); ++actionIt)
    {
        if(actionIt.value() == num)
            return actionIt.key();
    }

    return -1; //error
}

QString AnaIds::get_object_string(int num)
{
    for(QMap<QString,int>::iterator objectIt = m_known_actions.begin(); objectIt != m_known_actions.end(); ++objectIt)
    {
        if(objectIt.value() == num)
            return objectIt.key();
    }

    return -1; //error
}

int AnaIds::get_object_no(QString const &name)
{
    auto it = m_known_objects.find(name);
    if (it != m_known_objects.end()) {
        return it.value();
    }
    int result = m_next_object_no;
    add_object(name);
    return result;
}
