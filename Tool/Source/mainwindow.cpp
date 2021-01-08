#include "mainwindow.h"

#include <QApplication>
#include <QAction>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>
#include <QStyleFactory>
#include <QToolBar>
#include <QMenuBar>

#include "nodectrl.h"
#include "mainctrl.h"
#include "propertyeditor.h"
#include "zodiacgraph/baseedge.h"
#include "zodiacgraph/edgearrow.h"
#include "zodiacgraph/edgelabel.h"
#include "zodiacgraph/node.h"
#include "zodiacgraph/nodelabel.h"
#include "zodiacgraph/perimeter.h"
#include "zodiacgraph/plug.h"
#include "zodiacgraph/pluglabel.h"
#include "zodiacgraph/scene.h"
#include "zodiacgraph/view.h"

#include "graphstructures.h"
#include "analyticshandler.h"

//void createZodiacLogo(MainCtrl* mainCtrl);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_pUndoStack(new QUndoStack())
{
    setWindowTitle("REVEAL VR - Storyline Scaffolding Tool");
    setWindowIcon(QIcon(":/icons/reveal_logo.png"));

    /*QFont f = QGuiApplication::font();
    int defaultFontSize = f.pointSize();
    f.setPointSize(defaultFontSize + 3);
    QApplication::setFont(f);*/

    //set up the undo functions
    m_pUndoStack->setUndoLimit(10); //don't allow too many undos
    m_pUndoAction = m_pUndoStack->createUndoAction(this, tr("&Undo"));
    m_pUndoAction->setShortcuts(QKeySequence::Undo);

    m_pRedoAction = m_pUndoStack->createRedoAction(this, tr("&Redo"));
    m_pRedoAction->setShortcuts(QKeySequence::Redo);

    //create menu for undo functions
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(m_pUndoAction);
    editMenu->addAction(m_pRedoAction);

    //create menu for story graph functions
    QMenu *storyMenu = menuBar()->addMenu(tr("&Story Graph"));
    QAction* saveStory = new QAction(tr("&Save Story"), this);
    QAction* loadStory = new QAction(tr("&Load Story"), this);
    QAction* showStoryLinks = new QAction(tr("&Show Story Links"), this);
    storyMenu->addAction(saveStory);
    storyMenu->addAction(loadStory);
    storyMenu->addAction(showStoryLinks);
    showStoryLinks->setCheckable(true);
    showStoryLinks->setChecked(true);    //all links will be shown by default
    connect(saveStory, &QAction::triggered, [=]{m_mainCtrl->saveStoryGraph();});
    connect(loadStory, &QAction::triggered, [=]{m_mainCtrl->loadStoryGraph();});
    connect(showStoryLinks, &QAction::toggled, [=](bool checked) {m_mainCtrl->changeStoryVisibility(checked, zodiac::NODE_STORY);});

    //create menu for narrative graph functions
    QMenu *narrativeMenu = menuBar()->addMenu(tr("&Gameplay Graph"));
    QAction* saveNarrative = new QAction(tr("&Save Gameplay Graph"), this);
    QAction* loadNarrative = new QAction(tr("&Load Gameplay Graph"), this);
    QAction* showRequirementsLinks = new QAction(tr("&Show Requirements Links"), this);
    QAction* showNarStoryLinks = new QAction(tr("&Show Story Links"), this);
    showRequirementsLinks->setCheckable(true);
    showNarStoryLinks->setCheckable(true);
    showRequirementsLinks->setChecked(true);    //all links will be shown by default
    showNarStoryLinks->setChecked(true);
    narrativeMenu->addAction(saveNarrative);
    narrativeMenu->addAction(loadNarrative);
    narrativeMenu->addAction(showRequirementsLinks);
    narrativeMenu->addAction(showNarStoryLinks);
    connect(saveNarrative, &QAction::triggered, [=]{m_mainCtrl->saveNarrativeGraph();});
    connect(loadNarrative, &QAction::triggered, [=]{m_mainCtrl->loadNarrativeGraph();});
    connect(showRequirementsLinks, &QAction::toggled, [=](bool checked) {m_mainCtrl->changeReqVisibility(checked);});
    connect(showNarStoryLinks, &QAction::toggled, [=](bool checked) {m_mainCtrl->changeStoryVisibility(checked, zodiac::NODE_NARRATIVE);});

    // create the main toolbar
    QToolBar* mainToolBar = new QToolBar(this);
    mainToolBar->setStyleSheet("QToolBar {border: 0px;}");
    mainToolBar->setIconSize(QSize(12,12));
    mainToolBar->setMovable(false);
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addToolBar(Qt::TopToolBarArea, mainToolBar);

    //create menu for analytics functions
    QMenu *analyticsMenu = menuBar()->addMenu(tr("&Analytics"));
    QAction *lostnessEdit = new QAction(tr("&Add/Edit Tasks"), this);
    QAction *analyticsConnect = new QAction(tr("&Connect"), this);
    QAction *analyticsDisconnect = new QAction(tr("&Disconnect"), this);
    QAction *analyticsLoad = new QAction(tr("&Load Log File"), this);
    QAction *analyticsClear = new QAction(tr("&Clear Analytical Data"), this);
    QAction *lostnessGraph = new QAction(tr("&View Lostness Graph"), this);
    analyticsMenu->addAction(lostnessEdit);
    analyticsMenu->addAction(analyticsConnect);
    analyticsMenu->addAction(analyticsDisconnect);
    analyticsMenu->addAction(analyticsLoad);
    analyticsMenu->addAction(analyticsClear);
    analyticsMenu->addAction(lostnessGraph);

    // create the status bar
    statusBar();

    // create the Zodiac graph
    zodiac::Scene* zodiacScene = new zodiac::Scene(this);
    zodiac::View* zodiacView = new zodiac::View(this);
    zodiacView->setScene(zodiacScene);

    // create the Property Editor
    PropertyEditor* propertyEditor = new PropertyEditor(this);

    //create the analytics systems
    AnalyticsLogWindow *analyticsLog = new AnalyticsLogWindow(this);
    AnalyticsHandler *analyticsHandler = new AnalyticsHandler(analyticsLog, analyticsConnect, analyticsDisconnect, lostnessEdit, analyticsLoad, analyticsClear, lostnessGraph, this);

    // create the Main Controller
    //create action early to send to mainctrl so it can be disabled (only 1 story graph can exist)
    QAction* newStoryNodeAction = new QAction(QIcon(":/icons/plus.svg"), tr("&Add Story Graph"), this);
    m_mainCtrl = new MainCtrl(this, zodiacScene, propertyEditor, analyticsHandler, newStoryNodeAction, m_pUndoStack);

    // setup the main splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->addWidget(propertyEditor);
    m_mainSplitter->addWidget(zodiacView);
    m_mainSplitter->setSizes({100, 900});

    //setup the second splitter
    m_secondSplitter = new QSplitter(Qt::Vertical, this);
    m_secondSplitter->addWidget(m_mainSplitter);
    m_secondSplitter->addWidget(analyticsLog);
    m_secondSplitter->setSizes({900, 100});

    // create global actions
    QAction* newNarrativeNodeAction = new QAction(QIcon(":/icons/plus.svg"), tr("&Add Gameplay Node"), this);
    newNarrativeNodeAction->setShortcuts(QKeySequence::New);
    newNarrativeNodeAction->setStatusTip(tr("Create a new Narrative Node"));
    mainToolBar->addAction(newNarrativeNodeAction);
    connect(newNarrativeNodeAction, &QAction::triggered, [=]{m_mainCtrl->createDefaultNode(); });

    newStoryNodeAction->setShortcuts(QKeySequence::New);
    newStoryNodeAction->setStatusTip(tr("Create a new Story Node"));
    mainToolBar->addAction(newStoryNodeAction);
    connect(newStoryNodeAction, &QAction::triggered, [=]{m_mainCtrl->createStoryGraph();});

    //define actions for switching between both modes
    QAction* designModeAction = new QAction(tr("&Enable Design Mode"), this);
    QAction* analyticsModeAction = new QAction(tr("&Enable Analytics Mode"), this);


    designModeAction->setStatusTip(tr("Add, edit and remove nodes"));
    mainToolBar->addAction(designModeAction);
    connect(designModeAction, &QAction::triggered, [=]{analyticsModeAction->setEnabled(true); designModeAction->setEnabled(false); analyticsHandler->stopAnalyticsMode();
                                                    newStoryNodeAction->setEnabled(true); newNarrativeNodeAction->setEnabled((true)); saveNarrative->setEnabled(true);
                                                    loadNarrative->setEnabled(true); saveStory->setEnabled(true); loadStory->setEnabled(true); m_pUndoAction->setEnabled(true);
                                                    m_pRedoAction->setEnabled(true); lostnessEdit->setEnabled(true); analyticsConnect->setEnabled(false); analyticsLoad->setEnabled(false);
                                                    analyticsClear->setEnabled(false);});
    designModeAction->setEnabled(false);    //set to false as this will be enabled from the start

    analyticsModeAction->setStatusTip(tr("Analyse player data"));
    mainToolBar->addAction(analyticsModeAction);
    connect(analyticsModeAction, &QAction::triggered, [=]{designModeAction->setEnabled(true); analyticsModeAction->setEnabled(false);  analyticsHandler->startAnalyticsMode();
                                                        newStoryNodeAction->setEnabled(false); newNarrativeNodeAction->setEnabled((false)); saveNarrative->setEnabled(false);
                                                        loadNarrative->setEnabled(false); saveStory->setEnabled(false); loadStory->setEnabled(false); m_pUndoAction->setEnabled(false);
                                                        m_pRedoAction->setEnabled(false); lostnessEdit->setEnabled(false);  analyticsConnect->setEnabled(true); analyticsLoad->setEnabled(true);
                                                        analyticsClear->setEnabled(true);});

    QWidget* emptySpacer = new QWidget();
    emptySpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    mainToolBar->addWidget(emptySpacer);

    QAction* aboutAction = new QAction(QIcon(":/icons/questionmark.svg"), tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the about box"));
    mainToolBar->addAction(aboutAction);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(displayAbout()));

    // initialize the GUI
    //setCentralWidget(m_mainSplitter);
    setCentralWidget(m_secondSplitter);
    readSettings();
    zodiacScene->updateStyle();
    zodiacView->updateStyle();

    //createZodiacLogo(m_mainCtrl);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_mainCtrl->shutdown()) {
        // if the user accepted, close the application
        writeSettings();
        event->accept();

    } else {
        // if the user objected, do nothing
        event->ignore();
    }
}

void MainWindow::displayAbout()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle("About the REVEAL Storyline Scaffolding Tool");
    aboutBox.setText(
        "<h3>About this Application</h3>"

        "This storyline scaffolding tool is designed for developing Educational Environmental Narrative games. It enables a game narrative (game logic) and a game story "
        "(according to story grammar theory) to be created simulataneously. The node graph interface allows both graphs to be visualised and linked together to show where story "
        "information is contained in the narrative graph, ensuring that the full story is included."

        "<br><br>The tool is also effective as an analytical tool, showing the performance of the player, such as how lost they are and how close they are to the perfect sequence, depending "
        "on set parameters, as well as highlighting the parts of the narrative and story that a player has accessed, showing the topics that have been learned."

        "<br><br>Created with funding from the European Union's Horizon 2020 Research and Innovation Programme."

        "<h3>License</h3>"
        "<b>Storyline Scaffolding Tool</b> is developed by <a href=\"http://www.revealvr.eu\" title=\"revealvr.eu\">The REVEAL Consortium</a> "
        "and licensed under <a href=\"http://creativecommons.org/licenses/by/3.0/\" title=\"Creative Commons BY 4.0 International\">CC BY 4.0</a>.<br><br>"
        "<b>ZodiacGraph</b> is developed by <a href=\"http://www.clemens-sielaff.com\" title=\"clemens-sielaff.com\">Clemens Sielaff</a> "
        "and released under the terms of the <a href=\"https://opensource.org/licenses/MIT\" title=\"MIT License\">MIT License</a>."

        "<h3>Icons</h3>"
        "Icons from <a href=\"http://www.flaticon.com\" title=\"Flaticon\">www.flaticon.com</a>, licensed under <a href=\"http://creativecommons.org/licenses/by/3.0/\" title=\"Creative Commons BY 3.0\">CC BY 3.0</a><br>"
        "&#8594; Arrows, Plus, Minus & Bucket Icons made by <a href=\"http://www.freepik.com\" title=\"Freepik\">Freepik</a><br>"
        "&#8594; Door Icons made by <a href=\"http://www.icomoon.io\" title=\"Icomoon\">Icomoon</a><br>"
        "&#8594; Questionmark Icon made by <a href=\"http://www.danielbruce.se\" title=\"Daniel Bruce\">Daniel Bruce</a><br>"
        "&#8594; Play Icon made by <a href=\"http://yanlu.de\" title=\"Yannick\">Yannick</a><br>"
         );
    aboutBox.exec();
}

void MainWindow::readSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

    // apply custom palette and style
    QPalette palette;
    settings.beginGroup("palette");

    palette.setColor(QPalette::Window, settings.value("window", "#353535").toString());
    palette.setColor(QPalette::WindowText, settings.value("windowText", "#E0E2E4").toString());
    palette.setColor(QPalette::Base, settings.value("base", "#191919").toString());
    palette.setColor(QPalette::AlternateBase, settings.value("alternateBase", "#353535").toString());
    palette.setColor(QPalette::ToolTipBase, settings.value("toolTipBase", "#000000").toString());
    palette.setColor(QPalette::ToolTipText, settings.value("toolTipText", "#808080").toString());
    palette.setColor(QPalette::Text, settings.value("text", "#E0E2E4").toString());
    palette.setColor(QPalette::Button, settings.value("button", "#353535").toString());
    palette.setColor(QPalette::ButtonText, settings.value("buttonText", "#E0E2E4").toString());
    palette.setColor(QPalette::BrightText, settings.value("brightText", "#ffffff").toString());
    palette.setColor(QPalette::Link, settings.value("link", "#2a82da").toString());
    palette.setColor(QPalette::Highlight, settings.value("highlight", "#2a82da").toString());
    palette.setColor(QPalette::HighlightedText, settings.value("highlightedText", "#000000").toString());
    settings.endGroup();
    qApp->setPalette(palette);
    qApp->setStyle(QStyleFactory::create(settings.value("style", QString("Fusion")).toString()));

    // apply window position and size
    settings.beginGroup("mainWindow");
    resize(settings.value("size", QSize(1080, 600)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    m_mainSplitter->restoreState(settings.value("propertySplitter").toByteArray());
    settings.endGroup();

    // apply ZodiacGraph settings
    settings.beginGroup("zodiac");

    /*settings.beginGroup("baseEdge");
    zodiac::BaseEdge::setBaseWidth(settings.value("width", 2.5).toReal());
    zodiac::BaseEdge::setBaseColor(settings.value("color", "#cc5d4e").toString());
    settings.endGroup();*/

    /*settings.beginGroup("edgeArrow");
    zodiac::EdgeArrow::defineArrow(settings.value("length", 12).toReal(), settings.value("width", 15).toReal());
    zodiac::EdgeArrow::setArrowColor(settings.value("color", "#cc5d4e").toString());
    settings.endGroup();*/

    settings.beginGroup("edgeLabel");
    zodiac::EdgeLabel::setFontFamily(settings.value("fontFamily", "DejaVu Sans Mono").toString());
    zodiac::EdgeLabel::setPointSize(settings.value("pointSize", 10).toReal());
    zodiac::EdgeLabel::setWeight(QFont::Weight(settings.value("weight", 75).toInt()));
    zodiac::EdgeLabel::setColor(settings.value("color", "#c8c8c8").toString());
    zodiac::EdgeLabel::setTransparency(settings.value("transparency", 0.7).toReal());
    zodiac::EdgeLabel::setVerticalOffset(settings.value("verticalOffset", 0.5).toReal());
    settings.endGroup();

    settings.beginGroup("node");
    zodiac::Node::setCoreRadius(settings.value("coreRadius", 25).toReal());
    /*zodiac::Node::setIdleColor(settings.value("idleColor", "#4b77a7").toString());
    zodiac::Node::setSelectedColor(settings.value("selectedColor", "#62abfa").toString());
    zodiac::Node::setOutlineColor(settings.value("outlineColor", "#cdcdcd").toString());
    zodiac::Node::setOutlineWidth(settings.value("outlineWidth", 3).toReal());*/
    zodiac::Node::setPlugSweep(settings.value("plugSweep", 32.5).toReal());
    zodiac::Node::setGapSweep(settings.value("gapSweep", 8.125).toReal());
    settings.endGroup();

    settings.beginGroup("nodeLabel");
    //zodiac::NodeLabel::setTextColor(settings.value("textColor", "#ffffff").toString());
    //zodiac::NodeLabel::setBackgroundColor(settings.value("backgroundColor", "#426998").toString());
    //zodiac::NodeLabel::setLineColor(settings.value("lineColor", "#cdcdcd").toString());
    //zodiac::NodeLabel::setLineWidth(settings.value("lineWidth", 1.5).toReal());
    zodiac::NodeLabel::setCornerRadius(settings.value("cornerRadius", 8).toReal());
    zodiac::NodeLabel::setVerticalMargin(settings.value("verticalMargin", 2).toReal());
    zodiac::NodeLabel::setHorizontalMargin(settings.value("horizontalMargin", 4).toReal());
    zodiac::NodeLabel::setFontFamily(settings.value("fontFamily", "DejaVu Sans Mono").toString());
    zodiac::NodeLabel::setPointSize(settings.value("pointSize", 9).toReal());
    zodiac::NodeLabel::setWeight(QFont::Weight(settings.value("weight", 63).toInt()));
    settings.endGroup();

    settings.beginGroup("perimeter");
    zodiac::Perimeter::setColor(settings.value("color", "#2b517d").toString());
    settings.endGroup();

    settings.beginGroup("plug");
    zodiac::Plug::setWidth(settings.value("width", 12).toReal());
    zodiac::Plug::setInColor(settings.value("inColor", "#728872").toString());
    zodiac::Plug::setOutColor(settings.value("outColor", "#887272").toString());
    zodiac::Plug::setHighlightColor(settings.value("highlightColor", "#d1d7db").toString());
    settings.endGroup();

    settings.beginGroup("plugLabel");
    zodiac::PlugLabel::setFontFamily(settings.value("fontFamily", "DejaVu Sans Mono").toString());
    zodiac::PlugLabel::setPointSize(settings.value("pointSize", 10).toReal());
    zodiac::PlugLabel::setWeight(QFont::Weight(settings.value("weight", 75).toInt()));
    zodiac::PlugLabel::setColor(settings.value("color", "#828688").toString());
    zodiac::PlugLabel::setLabelDistance(settings.value("distance", 15).toReal());
    settings.endGroup();

    settings.beginGroup("view");
    zodiac::View::setBackgroundColor(QColor("#565656"));
    //zodiac::View::setBackgroundColor(settings.value("backgroundColor", "#565656").toString());
    settings.endGroup();

    settings.endGroup(); // zodiac
}

void MainWindow::writeSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

    // write out current palette
    QPalette palette = qApp->palette();
    settings.beginGroup("palette");
    settings.setValue("window", palette.color(QPalette::Window).name());
    settings.setValue("windowText", palette.color(QPalette::WindowText).name());
    settings.setValue("base", palette.color(QPalette::Base).name());
    settings.setValue("alternateBase", palette.color(QPalette::AlternateBase).name());
    settings.setValue("toolTipBase", palette.color(QPalette::ToolTipBase).name());
    settings.setValue("toolTipText", palette.color(QPalette::ToolTipText).name());
    settings.setValue("text", palette.color(QPalette::Text).name());
    settings.setValue("button", palette.color(QPalette::Button).name());
    settings.setValue("buttonText", palette.color(QPalette::ButtonText).name());
    settings.setValue("brightText", palette.color(QPalette::BrightText).name());
    settings.setValue("link", palette.color(QPalette::Link).name());
    settings.setValue("highlight", palette.color(QPalette::Highlight).name());
    settings.setValue("highlightedText", palette.color(QPalette::HighlightedText).name());
    settings.endGroup();

    // write out MainWindow position and size
    settings.beginGroup("mainWindow");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("propertySplitter", m_mainSplitter->saveState());
    settings.endGroup();

    // write out ZodiacGraph settings
    settings.beginGroup("zodiac");

    /*settings.beginGroup("baseEdge");
    settings.setValue("width", zodiac::BaseEdge::getBaseWidth());
    settings.setValue("color", zodiac::BaseEdge::getBaseColor().name());
    settings.endGroup();*/

    /*settings.beginGroup("edgeArrow");
    settings.setValue("width", zodiac::EdgeArrow::getArrowWidth());
    settings.setValue("length", zodiac::EdgeArrow::getArrowLength());
    settings.setValue("color", zodiac::EdgeArrow::getArrowColor().name());
    settings.endGroup();*/

    settings.beginGroup("edgeLabel");
    settings.setValue("fontFamily", zodiac::EdgeLabel::getFontFamily());
    settings.setValue("pointSize", zodiac::EdgeLabel::getPointSize());
    settings.setValue("weight", zodiac::EdgeLabel::getWeight());
    settings.setValue("color", zodiac::EdgeLabel::getColor().name());
    settings.setValue("transparency", zodiac::EdgeLabel::getTransparency());
    settings.setValue("verticalOffset", zodiac::EdgeLabel::getVerticalOffset());
    settings.endGroup();

    settings.beginGroup("node");
    settings.setValue("coreRadius", zodiac::Node::getCoreRadius());
    /*settings.setValue("idleColor", zodiac::Node::getIdleColor().name());
    settings.setValue("selectedColor", zodiac::Node::getSelectedColor().name());
    settings.setValue("outlineColor", zodiac::Node::getOutlineColor().name());
    settings.setValue("outlineWidth", zodiac::Node::getOutlineWidth());*/
    settings.setValue("plugSweep", zodiac::Node::getPlugSweep());
    settings.setValue("gapSweep", zodiac::Node::getGapSweep());
    settings.endGroup();

    settings.beginGroup("nodeLabel");
    //settings.setValue("textColor", zodiac::NodeLabel::getTextColor().name());
    //settings.setValue("backgroundColor", zodiac::NodeLabel::getBackgroundColor().name());
    //settings.setValue("lineColor", zodiac::NodeLabel::getLineColor().name());
    settings.setValue("lineWidth", zodiac::NodeLabel::getLineWidth());
    settings.setValue("cornerRadius", zodiac::NodeLabel::getCornerRadius());
    settings.setValue("verticalMargin", zodiac::NodeLabel::getVerticalMargin());
    settings.setValue("horizontalMargin", zodiac::NodeLabel::getHorizontalMargin());
    settings.setValue("fontFamily", zodiac::NodeLabel::getFontFamily());
    settings.setValue("pointSize", zodiac::NodeLabel::getPointSize());
    settings.setValue("weight", zodiac::NodeLabel::getWeight());
    settings.endGroup();

    settings.beginGroup("perimeter");
    settings.setValue("color", zodiac::Perimeter::getColor().name());
    settings.endGroup();

    settings.beginGroup("plug");
    settings.setValue("width", zodiac::Plug::getWidth());
    settings.setValue("inColor", zodiac::Plug::getInColor().name());
    settings.setValue("outColor", zodiac::Plug::getOutColor().name());
    settings.setValue("highlightColor", zodiac::Plug::getHighlightColor().name());
    settings.endGroup();

    settings.beginGroup("plugLabel");
    settings.setValue("fontFamily", zodiac::PlugLabel::getFontFamily());
    settings.setValue("pointSize", zodiac::PlugLabel::getPointSize());
    settings.setValue("weight", zodiac::PlugLabel::getWeight());
    settings.setValue("color", zodiac::PlugLabel::getColor().name());
    settings.setValue("distance", zodiac::PlugLabel::getLabelDistance());
    settings.endGroup();

    settings.beginGroup("view");
    settings.setValue("backgroundColor", zodiac::View::getBackgroundColor().name());
    settings.endGroup();

    settings.endGroup(); // zodiac
}
