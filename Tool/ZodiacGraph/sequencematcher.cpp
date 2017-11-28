#include "sequencematcher.h"
#include "musaline/musaline.hpp"

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

QVector<AnaEvent> SequenceMatcher::read_events(
        QFile &file, std::shared_ptr<AnaIds> ids)
{
    AnaHandler handler(ids);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"Cannot open file "<<file.fileName()<<"\n";
        return QVector<AnaEvent>();
    }
    QString settings = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(settings.toUtf8());
    handler.set_events(jsonDoc.array());
    file.close();
    auto events = handler.get_events();
    return events;
}


SequenceMatcher::SequenceMatcher()
{
    //char const * filename1 = "D:\\GJ_Analytics\\Debug\\file1.txt";//argv[1];
    //char const * filename2 = "D:\\GJ_Analytics\\Debug\\file2.txt";//argv[2];

    QFile file1;
    file1.setFileName("D:\\GJ_Analytics\\Debug\\file1.txt");

    QFile file2;
    file2.setFileName("D:\\GJ_Analytics\\Debug\\file2.txt");

    // Read the two files.
    std::shared_ptr<AnaIds> ids = std::make_shared<AnaIds>();
    auto events1 = read_events(file1, ids);
    auto events2 = read_events(file2, ids);
    // for (auto el: events2) {
    //     std::cout<<el.action<<" "<<el.object<<"\n";
    // }

    // Construct an aligner to compute the similarity between the sequences.
    musaline::LinearAligner<AnaCost> aligner;

    // Compute the similarity between the ground_truth and itself.
    // This gives the maximal score.
    // fac is used to make percentage scores relative to this maximal score.
    double self_benefit = aligner.global_align(events1, events1).score;
    double fac = 100.0 / self_benefit;

    // Compute the score as a percentage of the maximal score after each step of the user.
    qDebug() << "In incremental steps:\n";
    for (auto cur = events2.begin(); true; ++cur) {
        boost::sub_range<QVector<AnaEvent> > sub(events2.begin(), cur);
        double benefit = aligner.cut_first_end_off_align(events1, sub).score;
        qDebug() << "Score is: " << fac*benefit << "%\n";
        if (cur == events2.end())
            break;
    }
}
