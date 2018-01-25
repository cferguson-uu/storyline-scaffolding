#include "narrativefilesorter.h"

NarrativeFileSorter::NarrativeFileSorter(QWidget *parent)
    : m_mainLayout(new QGridLayout),
      QDialog(parent)
{
    QVBoxLayout *parentLayout = new QVBoxLayout;
    QScrollArea *scrollArea = new QScrollArea;
    QWidget *widg = new QWidget();

    m_mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    m_mainLayout->setAlignment(Qt::AlignTop);
    widg->setLayout(m_mainLayout);
    scrollArea->setWidget(widg);

    parentLayout->addWidget(scrollArea);
    setLayout(parentLayout);

    m_listWidget = new QListWidget();
    m_listWidget->setDragEnabled(true);
    m_listWidget->setDropIndicatorShown(true);
    m_listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    m_listWidget->show();

    m_okBtn = new QPushButton("OK");
    m_cancelBtn = new QPushButton("Cancel");
    m_okBtn->setMaximumSize(100, 25);
    m_cancelBtn->setMaximumSize(100, 25);
    m_okBtn->setMinimumSize(100, 25);
    m_cancelBtn->setMinimumSize(100, 25);
    connect(m_okBtn, &QPushButton::released, [=]{exportSortedList();});
    connect(m_cancelBtn, &QPushButton::released, [=]{this->hide();});

    m_mainLayout->addWidget(m_listWidget, 0, 0, 1, 5, Qt::AlignCenter);
    m_mainLayout->addWidget(m_okBtn, 1, 0, 1, 1, Qt::AlignCenter);
    m_mainLayout->addWidget(m_cancelBtn, 1, 1, Qt::AlignCenter);

    setWindowTitle(tr("Sort Narrative Files"));
    resize(QSize(310,270));

    hide(); //should start hidden until called
}

void NarrativeFileSorter::showWindow(QVector<QString> fileNames)
{
    m_listWidget->clear();   //clear any existing data

    foreach (QString fileName, fileNames)   //add fileNames to the list for sorting
    {
        m_listWidget->addItem(fileName);
    }

    m_mainLayout->setAlignment(Qt::AlignCenter);

    show();
}

void NarrativeFileSorter::exportSortedList()
{
    m_orderedList.clear();

    for(int row = 0; row < m_listWidget->count(); row++)
    {
        QListWidgetItem *item = m_listWidget->item(row);
        m_orderedList.push_back(item->text());
    }

    loadOrderedNarrative(m_orderedList);

    hide();
}
