/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#include <QLayout>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>

#include "QProps/PropertiesView.h"
#include "QProps/FileTypes.h"
#include "QProps/error.h"

#include "MainWindow.h"
#include "ScoreView.h"
#include "ScoreDocument.h"
#include "ScoreLayout.h"
#include "PageLayout.h"
#include "PageLayout.h"
#include "TextItem.h"
#include "AllPropertiesView.h"
#include "audio/SamplePlayer.h"
#include "audio/SynthDevice.h"
#include "core/NoteStream.h"
#include "core/Score.h"
#include "core/ScoreEditor.h"
#include "core/ExportMusicXML.h"

namespace Sonot {

struct MainWindow::Private
{
    Private(MainWindow*w)
        : p         (w)
        , document  (nullptr)
        , isChanged (false)
        , isSynthChanged (false)
        , player    (nullptr)
        , synthStream(nullptr)
    { }

    ~Private()
    {
        delete document;
    }

    void createWidgets();
    void createObjects();
    void createMenu();

    bool isSaveToDiscard();
    bool isSaveToDiscardSynth();
    bool setChanged(bool c);
    void updateWindowTitle();
    void updateActions();

    bool loadScore(const QString& fn);
    bool saveScore(const QString& fn);
    Score createNewScore();
    bool exportMusicXML();

    bool loadSynth(const QString& fn);
    bool saveSynth(const QString& fn);

    //void setEditProperties(const QString& s);
    //void applyProperties();


    MainWindow* p;

    ScoreView* scoreView;
    ScoreDocument* document;
    AllPropertiesView* propsView;

    QString curPropId, curFilename, curSynthFilename;
    bool isChanged, isSynthChanged;

    SamplePlayer* player;
    SynthDevice* synthStream;

    QMenu* menuEdit;
    QAction *actSaveScore,
            *actFollowPlay,
            *actUndo, *actRedo;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow   (parent)
    , p_            (new Private(this))
{
    setObjectName("MainWindow");
    setMinimumSize(320, 320);
    setGeometry(0,0,720,600);

    p_->createObjects();
    p_->createWidgets();
    p_->createMenu();

    // put synthStream into play
    p_->player->play(p_->synthStream, 1, p_->synthStream->sampleRate());

    //setScore(p_->getSomeScore());
    setScore(p_->createNewScore());
    p_->document->editor()->clearUndo();

    // install file types
    QProps::FileTypes::addFileType(QProps::FileType(
        "document", "Sonot document", "../sonot/score",
        QStringList() << ".sonot.json",
        QStringList() << "Sonot document (*.sonot.json)"));
    QProps::FileTypes::addFileType(QProps::FileType(
        "synth", "Synthesizer", "../sonot/synth",
        QStringList() << ".synth.json",
        QStringList() << "Sonot synth (*.synth.json)"));
    QProps::FileTypes::addFileType(QProps::FileType(
        "musicxml", "MusicXML", "../sonot/export",
        QStringList() << ".music.xml",
        QStringList() << "MusicXML (*.music.xml *.xml)"));
}

MainWindow::~MainWindow()
{
    delete p_;
}

void MainWindow::Private::createWidgets()
{
    p->setStatusBar(new QStatusBar(p));

    p->setCentralWidget(new QWidget());
    auto lh = new QHBoxLayout(p->centralWidget());

        scoreView = new ScoreView(p);
        scoreView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        lh->addWidget(scoreView);
        scoreView->setDocument(document);
        connect(scoreView, &ScoreView::noteEntered, [=](const Note& n)
        {
            if (n.isNote())
                synthStream->playNote(n.value(), .4);
        });
        connect(scoreView, &ScoreView::statusChanged, [=](const QString& s)
        {
            p->statusBar()->showMessage(s);
        });
        connect(scoreView, &ScoreView::currentIndexChanged,
                [=](const Score::Index& newIdx, const Score::Index& oldIdx)
        {
            // update stream property view on stream change
            if (newIdx.stream() != oldIdx.stream())
                propsView->setScoreIndex(newIdx);
        });

        connect(synthStream, &SynthDevice::indexChanged,
                [=](const Score::Index& idx)
        {
            scoreView->setPlayingIndex(idx);
            if (actFollowPlay->isChecked())
                scoreView->ensureIndexVisible(idx);
        });

        propsView = new AllPropertiesView(p);
        propsView->setSizePolicy(
                    QSizePolicy::Minimum, QSizePolicy::Expanding);
        propsView->setSynthStream(synthStream);
        propsView->setDocument(document);
        propsView->setVisible(false);
        lh->addWidget(propsView);

        connect(propsView, &AllPropertiesView::synthChanged,
                [=](){ isSynthChanged = true; });
}

void MainWindow::Private::createObjects()
{
    player = new SamplePlayer(p);

    synthStream = new SynthDevice(p);

    document = new ScoreDocument();
    connect(document->editor(), &ScoreEditor::documentChanged,
            [=]()
    {
        setChanged(true);
    });
    connect(document->editor(), &ScoreEditor::scoreReset,
            [=](Score* s)
    {
        synthStream->setScore(s);
        synthStream->setIndex(s->index(0,0,0,0));
    });
    connect(document->editor(), &ScoreEditor::undoAvailable,
            [=](bool avail, const QString& desc)
    {
        actUndo->setText(tr("undo %1").arg(desc));
        actUndo->setEnabled(avail);
    });
    connect(document->editor(), &ScoreEditor::redoAvailable,
            [=](bool avail, const QString& desc)
    {
        actRedo->setText(tr("redo %1").arg(desc));
        actRedo->setEnabled(avail);
    });
    connect(document->editor(), &ScoreEditor::cursorChanged,
            [=](const Score::Index& idx)
    {
        scoreView->setCurrentIndex(idx);
    });
}

void MainWindow::Private::createMenu()
{
    p->setMenuBar(new QMenuBar(p) );
    QMenu* menu;
    QAction* a;

    menu = p->menuBar()->addMenu(tr("File"));

    a = menu->addAction(tr("New"));
    a->setShortcut(Qt::CTRL + Qt::Key_N);
    a->connect(a, &QAction::triggered, [=]()
    {
        if (!isSaveToDiscard())
            return;
        curFilename.clear();
        setChanged(false);
        p->setScore(createNewScore());
    });

    a = menu->addAction(tr("Load Score"));
    a->setShortcut(Qt::CTRL + Qt::Key_L);
    a->connect(a, &QAction::triggered, [=]()
    {
        if (!isSaveToDiscard())
            return;
        QString fn = QProps::FileTypes::getOpenFilename("document", p);
        if (!fn.isEmpty())
            loadScore(fn);
    });

    a = actSaveScore = menu->addAction(tr("Save Score"));
    a->setShortcut(Qt::CTRL + Qt::Key_S);
    a->setEnabled(false);
    a->connect(a, &QAction::triggered, [=]()
    {
        if (!curFilename.isEmpty())
            saveScore(curFilename);
    });

    a = menu->addAction(tr("Save Score as"));
    a->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);
    a->connect(a, &QAction::triggered, [=]()
    {
        QString fn = QProps::FileTypes::getSaveFilename("document", p);
        if (!fn.isEmpty())
            saveScore(fn);
    });

    menu->addSeparator();

    auto sub = menu->addMenu(tr("Export"));

        a = sub->addAction(tr("MusicXML"));
        connect(a, &QAction::triggered, [=](){ exportMusicXML(); });

    menu->addSeparator();

    a = menu->addAction(tr("Load Synth settings"));
    a->connect(a, &QAction::triggered, [=]()
    {
        if (!isSaveToDiscardSynth())
            return;
        QString fn = QProps::FileTypes::getOpenFilename("synth", p);
        if (!fn.isEmpty())
            loadSynth(fn);
    });

    a = menu->addAction(tr("Save Synth settings as"));
    a->connect(a, &QAction::triggered, [=]()
    {
        QString fn = QProps::FileTypes::getSaveFilename("synth", p);
        if (!fn.isEmpty())
            saveSynth(fn);
    });

    // ######## EDIT ########

    menu = menuEdit = p->menuBar()->addMenu(tr("Edit"));

    a = actUndo = menu->addAction(tr("Undo"));
    a->setShortcut(Qt::CTRL + Qt::Key_Z);
    a->setEnabled(false);
    connect(a, &QAction::triggered, [=](){ document->editor()->undo(); });

    a = actRedo = menu->addAction(tr("Redo"));
    a->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
    a->setEnabled(false);
    connect(a, &QAction::triggered, [=](){ document->editor()->redo(); });

    menu->addSeparator();

    scoreView->createEditActions(menu);



    // ###### PLAYBACK ######

    menu = p->menuBar()->addMenu(tr("Playback"));

    a = actFollowPlay = menu->addAction(tr("Follow play cursor"));
    a->setCheckable(true);
    a->setChecked(true);

    a = menu->addAction(tr("Play whole"));
    a->setShortcut(Qt::Key_F5);
    a->connect(a, &QAction::triggered, [=]()
    {
        synthStream->setIndex(synthStream->score()->index(0,0,0,0));
        synthStream->setPlaying(true);
    });

    a = menu->addAction(tr("Play current part"));
    a->setShortcut(Qt::Key_F6);
    a->connect(a, &QAction::triggered, [=]()
    {
        auto idx = scoreView->currentIndex();
        synthStream->setIndex(synthStream->score()->index(
                                            idx.stream(),0,0,0));
        synthStream->setPlaying(true);
    });

    a = menu->addAction(tr("Play from cursor"));
    a->setShortcut(Qt::Key_F7);
    a->connect(a, &QAction::triggered, [=]()
    {
        auto idx = scoreView->currentIndex();
        if (!idx.isValid())
            idx = synthStream->score()->index(0,0,0,0);
        synthStream->setIndex(idx);
        synthStream->setPlaying(true);
    });

    a = menu->addAction(tr("Stop"));
    a->setShortcut(Qt::Key_F8);
    a->connect(a, &QAction::triggered, [=]()
    {
        synthStream->setPlaying(false);
    });


    menu = p->menuBar()->addMenu(tr("Settings"));

    a = menu->addAction(tr("Visible"));
    a->setCheckable(true);
    a->connect(a, &QAction::triggered, [=](bool e)
    {
        propsView->setVisible(e);
    });
}


void MainWindow::showEvent(QShowEvent*)
{
    p_->scoreView->showPage(0);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    if (!p_->isSaveToDiscard() || !p_->isSaveToDiscardSynth())
        e->ignore();
}

void MainWindow::setScore(const Score& s)
{
    p_->document->editor()->setEnableUndo(false);
    p_->document->setScore(s);
    p_->document->editor()->setEnableUndo(true);
    p_->document->editor()->clearUndo();
    p_->propsView->setDocument(p_->document);
}


bool MainWindow::Private::isSaveToDiscard()
{
    if (!isChanged)
        return true;

    int ret = QMessageBox::question(p, tr("unsaved changes"),
                tr("The current changes will be lost in time,\n"
                   "like tears in the rain.\n"),
                tr("Save as"), tr("Throw away"), tr("Cancel"));
    if (ret == 1)
        return true;
    if (ret == 2)
        return false;
    QString fn = QProps::FileTypes::getSaveFilename("document", p);
    if (fn.isEmpty())
        return false;
    return saveScore(fn);
}


bool MainWindow::Private::isSaveToDiscardSynth()
{
    if (!isSynthChanged)
        return true;

    int ret = QMessageBox::question(p, tr("unsaved changes"),
                tr("The current synth settings will be lost in time,\n"
                   "like tears in the rain.\n"),
                tr("Save as"), tr("Throw away"), tr("Cancel"));
    if (ret == 1)
        return true;
    if (ret == 2)
        return false;
    QString fn = QProps::FileTypes::getSaveFilename("synth", p);
    if (fn.isEmpty())
        return false;
    return saveSynth(fn);
}

bool MainWindow::Private::setChanged(bool c)
{
    bool dif = c != isChanged;
    isChanged = c;
    if (dif)
    {
        updateWindowTitle();
        updateActions();
    }
    return dif;
}

void MainWindow::Private::updateWindowTitle()
{
    QString t = curFilename.isEmpty()
            ? QString("no-name")
            : QFileInfo(curFilename).fileName();
    if (isChanged)
        t.prepend("*");
    if (t != p->windowTitle())
        p->setWindowTitle(t);
}

void MainWindow::Private::updateActions()
{
    actSaveScore->setEnabled(
                !curFilename.isEmpty() && isChanged );
}


bool MainWindow::Private::loadScore(const QString& fn)
{
    try
    {
        document->editor()->setEnableUndo(false);
        document->loadJsonFile(fn);
        document->editor()->setEnableUndo(true);
        document->editor()->clearUndo();
        curFilename = fn;
        if (!setChanged(false))
            updateWindowTitle();
        propsView->setDocument(document);
        return true;
    }
    catch (QProps::Exception e)
    {
        QMessageBox::critical(p, tr("load score"),
                              tr("Could not load score from\n%1\n%2")
                              .arg(fn).arg(e.what()));
    }
    return false;
}

bool MainWindow::Private::saveScore(const QString& fn)
{
    try
    {
        document->saveJsonFile(fn);
        curFilename = fn;
        if (!setChanged(false))
            updateWindowTitle();
        return true;
    }
    catch (QProps::Exception e)
    {
        QMessageBox::critical(p, tr("save score"),
                              tr("Could not save score to\n%1\n%2")
                              .arg(fn).arg(e.what()));
    }
    return false;
}

Score MainWindow::Private::createNewScore()
{
    NoteStream n;
    n.appendBar(n.createDefaultBar());

    Score s;
    s.appendNoteStream(n);
    return s;
}

bool MainWindow::Private::exportMusicXML()
{
    ExportMusicXML exp(*document->score());
    QString filename = QProps::FileTypes::getSaveFilename("musicxml", p);
    if (filename.isEmpty())
        return false;

    try
    {
        exp.saveFile(filename);
        return true;
    }
    catch (const QProps::Exception& e)
    {
        QMessageBox::critical(p, tr("Export MusicXML"),
            tr("Exporting MusicXML file failed!\n%1")
                              .arg(e.text()));
    }
    return false;
}

bool MainWindow::Private::loadSynth(const QString& fn)
{
    try
    {
        synthStream->loadJsonFile(fn);
        curSynthFilename = fn;
        isSynthChanged = false;
        propsView->setSynthStream(synthStream);
        return true;
    }
    catch (QProps::Exception e)
    {
        QMessageBox::critical(p, tr("load synth"),
                              tr("Could not load synth from\n%1\n%2")
                              .arg(fn).arg(e.what()));
    }
    return false;
}

bool MainWindow::Private::saveSynth(const QString& fn)
{
    try
    {
        synthStream->saveJsonFile(fn);
        curSynthFilename = fn;
        isSynthChanged = false;
        return true;
    }
    catch (QProps::Exception e)
    {
        QMessageBox::critical(p, tr("save synth"),
                              tr("Could not save synth to\n%1\n%2")
                              .arg(fn).arg(e.what()));
    }
    return false;
}



} // namespace Sonot
