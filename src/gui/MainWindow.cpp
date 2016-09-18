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
#include "QProps/error.h"

#include "MainWindow.h"
#include "ScoreView.h"
#include "ScoreDocument.h"
#include "ScoreLayout.h"
#include "PageLayout.h"
#include "PageLayout.h"
#include "TextItem.h"
#include "audio/SamplePlayer.h"
#include "audio/SynthDevice.h"
#include "core/NoteStream.h"
#include "core/Score.h"
#include "core/ScoreEditor.h"

namespace Sonot {

struct MainWindow::Private
{
    Private(MainWindow*w)
        : p         (w)
        , document  (nullptr)
        , isChanged (false)
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
    bool setChanged(bool c);
    void updateWindowTitle();
    void updateActions();

    bool loadScore(const QString& fn);
    bool saveScore(const QString& fn);
    Score createNewScore();

    void setEditProperties(const QString& s);
    void applyProperties();

    // debugging
    void playSomething();
    Score getSomeScore();
    NoteStream getNotes1();
    NoteStream getNotes2();
    NoteStream getNotes3();


    MainWindow* p;

    ScoreView* scoreView;
    ScoreDocument* document;
    QProps::PropertiesView* propsView;

    QString curPropId, curFilename;
    bool isChanged;

    SamplePlayer* player;
    SynthDevice* synthStream;

    QMenu* menuEdit;
    QAction* actSaveScore;
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
                synthStream->playNote(n.value());
        });
        connect(scoreView, &ScoreView::statusChanged, [=](const QString& s)
        {
            p->statusBar()->showMessage(s);
        });

        connect(synthStream, SIGNAL(indexChanged(Score::Index)),
                scoreView, SLOT(setPlayingIndex(Score::Index)));

        propsView = new QProps::PropertiesView(p);
        propsView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        propsView->setVisible(false);
        lh->addWidget(propsView);

        connect(propsView, &QProps::PropertiesView::propertyChanged, [=]()
        {
            applyProperties();
            setChanged(true);
        });
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
        QString fn = p->getScoreFilename(false);
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
        QString fn = p->getScoreFilename(true);
        if (!fn.isEmpty())
            saveScore(fn);
    });

    menu = menuEdit = p->menuBar()->addMenu(tr("Edit"));
    menu->addActions( scoreView->createEditActions() );

    menu = p->menuBar()->addMenu(tr("Playback"));

    a = menu->addAction(tr("Play whole"));
    a->setShortcut(Qt::Key_F5);
    a->connect(a, &QAction::triggered, [=]()
    {
        synthStream->setIndex(synthStream->score()->index(0,0,0,0));
        synthStream->setPlaying(true);
    });

    a = menu->addAction(tr("Continue"));
    a->setShortcut(Qt::Key_F6);
    a->connect(a, &QAction::triggered, [=]()
    {
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


    menu = p->menuBar()->addMenu(tr("Options"));

    auto group = new QActionGroup(menu);
#define SONOT__CREATE_PROP(text__, id__) \
    a = menu->addAction(text__); \
    a->setCheckable(true); \
    a->connect(a, &QAction::triggered, [=](bool e) { \
        if (e) setEditProperties(id__); }); \
    group->addAction(a);

    SONOT__CREATE_PROP(tr("Hidden"), "hidden");
    a->setChecked(true);
    SONOT__CREATE_PROP(tr("Synth"), "synth");
    SONOT__CREATE_PROP(tr("Score"), "score");
    SONOT__CREATE_PROP(tr("Document"), "document");
    SONOT__CREATE_PROP(tr("Page layout (title)"), "page-layout-title");
    SONOT__CREATE_PROP(tr("Page layout (left)"), "page-layout-left");
    SONOT__CREATE_PROP(tr("Page layout (right)"), "page-layout-right");
    SONOT__CREATE_PROP(tr("Score layout (title)"), "score-layout-title");
    SONOT__CREATE_PROP(tr("Score layout (left)"), "score-layout-left");
    SONOT__CREATE_PROP(tr("Score layout (right)"), "score-layout-right");
#undef SONOT__CREATE_PROP

}

void MainWindow::Private::setEditProperties(const QString &s)
{
    curPropId = s;
    if (curPropId.startsWith("synth"))
    {
        propsView->setProperties(synthStream->synth().props());
    }
    else if (curPropId.startsWith("document"))
    {
        propsView->setProperties(document->props());
    }
    else if (curPropId.startsWith("page-layout-"))
    {
        QString sub = curPropId.mid(12);
        propsView->setProperties(document->pageLayout(sub).margins());
    }
    else if (curPropId.startsWith("score-layout-"))
    {
        QString sub = curPropId.mid(13);
        propsView->setProperties(document->scoreLayout(sub).props());
    }
    else if (curPropId.startsWith("score"))
    {
        propsView->setProperties(document->score()->props());
    }
    else
        propsView->clear();

    propsView->setVisible(!propsView->isEmpty());
}

void MainWindow::Private::applyProperties()
{
    if (curPropId.startsWith("synth"))
    {
        synthStream->setSynthProperties(propsView->properties());
    }
    else if (curPropId.startsWith("document"))
    {
        document->setProperties(propsView->properties());
    }
    else if (curPropId.startsWith("page-layout-"))
    {
        QString sub = curPropId.mid(12);
        auto l = document->pageLayout(sub);
        l.setMargins(propsView->properties());
        document->setPageLayout(sub, l);
    }
    else if (curPropId.startsWith("score-layout-"))
    {
        QString sub = curPropId.mid(13);
        auto l = document->scoreLayout(sub);
        l.setProperties(propsView->properties());
        document->setScoreLayout(sub, l);
    }
    else if (curPropId.startsWith("score"))
    {
        document->setScoreProperties(propsView->properties());
    }
    /*
    PageAnnotation anno = scoreView->scoreDocument().pageAnnotation(0);
    anno.textItems()[0].setProperties(propsView->properties());
    scoreView->scoreDocument().setPageAnnotation(0, anno);
    scoreView->update();
    */
}

void MainWindow::showEvent(QShowEvent*)
{
    p_->scoreView->showPage(0);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    if (!p_->isSaveToDiscard())
        e->ignore();
}

void MainWindow::setScore(const Score& s)
{
    p_->document->setScore(s);
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
    QString fn = p->getScoreFilename(true);
    if (fn.isEmpty())
        return false;
    return saveScore(fn);
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
        document->loadJsonFile(fn);
        curFilename = fn;
        if (!setChanged(false))
            updateWindowTitle();
        setEditProperties(curPropId);
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
    n.appendBar(n.defaultBarRows());

    Score s;
    s.appendNoteStream(n);
    return s;
}

QString MainWindow::getScoreFilename(bool forSave)
{
    QString dir = "../sonot/score",
            filter = "*.sonot.json";
    QString fn;
    if (forSave)
        fn = QFileDialog::getSaveFileName(this, tr("save score"),
                                 dir, filter, &filter);
    else
        fn = QFileDialog::getOpenFileName(this, tr("load score"),
                                 dir, filter, &filter);
    return fn;
}







void MainWindow::Private::playSomething()
{
#if 0
    std::vector<float> data;
#if 0
    for (int i=0; i<44100*3; ++i)
    {
        float t = (float)i / 44100.;
        data.push_back( std::sin(t * 3.14159265 * 2. * 437.) );
    }
#else
    Synth synth;
    for (int i=0; i<16; ++i)
        synth.noteOn(70 + rand()%19, .1, i*5000);

    data.resize(44100*3);
    synth.process(data.data(), data.size());
#endif

    player->play(data.data(), data.size(), 1, 22100);
    player->play(data.data(), data.size(), 1, 44100);
#endif


    Score score;
    NoteStream stream = getNotes3();
    //qDebug().noquote() << stream.toString();
    score.appendNoteStream(stream);

    //synth->setScore(score);
}

Score MainWindow::Private::getSomeScore()
{
    Score score;
    score.setTitle("Space Invaders");
    score.setCopyright("(c) 1963, Ingsoc");
    score.setAuthor("Dorian Gray");

    score.appendNoteStream( getNotes3() );
    return score;
}

NoteStream MainWindow::Private::getNotes1()
{
    NoteStream stream;

    for (int i=0; i<8; ++i)
    {
        QList<Bar> rows;

        if (i % 3 < 2)
        {
            Bar bar;
            bar << Note(Note::C, 5)
                << Note(Note::Dis, 5)
                << Note(Note::G, 5)
                << Note(Note::A, 5);
            rows << bar;
        }
        if (i % 2 == 1)
        {
            Bar bar;
            bar << Note(Note::C, 6)
                << Note(Note::Dis, 6)
                << Note(Note::G, 6)
                << Note(Note::D, 5);
            rows << bar;
        }
        if (i > 1)
        {
            Bar bar;
            bar << Note(Note::C, 6)
                << Note(Note::F, 6)
                << Note(Note::D, 6);
            rows << bar;
        }
        if (i % 4 == 0)
        {
            Bar bar;
            bar << Note(Note::C, 6)
                << Note(Note::Space)
                << Note(Note::G, 6)
                << Note(Note::D, 7);
            rows << bar;
        }

        stream.appendBar(rows);
    }

    return stream;
}

NoteStream MainWindow::Private::getNotes2()
{
    NoteStream stream;

    for (int i=0; i<8; ++i)
    {
        QList<Bar> rows;

        if (i % 2 == 0)
        {
            Bar bar;
            bar << Note(Note::C, 6)
                << Note(Note::A, 5)
                << Note(Note::Ais, 5)
                << Note(Note::G, 5);
            rows << bar;
        }
        else
        {
            Bar bar;
            bar << Note(Note::A, 5)
                << Note(Note::Dis, 5)
                << Note(Note::G, 5)
                << Note(Note::C, 5);
            rows << bar;
        }
        {
            Bar bar;
              bar << Note(72 + (i * 3) % 11);
            rows << bar;
        }

        stream.appendBar(rows);
    }

    return stream;
}

NoteStream MainWindow::Private::getNotes3()
{
    NoteStream stream;

#define SONOT__BAR(a1_, a2_) \
    if (i == barIdx++) \
    { Bar b1_; b1_ << a1_; b1_.transpose(0); rows << b1_; \
      Bar b2_; b2_ << a2_; b2_.transpose(0); rows << b2_; }

    for (int i=0; i<32; ++i)
    {
        QList<Bar> rows;
        int barIdx = 0;

        SONOT__BAR("p"<<" "<<" "<<" ",
                   "5"<<"6-4"<<"7"<<"1");

        SONOT__BAR("5"<<" "<<" "<<" ",
                   "7"<<"6"<<"5"<<"4");

        SONOT__BAR(" "<<" "<<" "<<" ",
                   "3"<<"5"<<"4"<<"3");

        SONOT__BAR("6"<<" "<<" "<<" ",
                   "2"<<"2"<<"7"<<"1");

        SONOT__BAR("7"<<" "<<" "<<" ",
                   "2"<<"1"<<"7"<<"6");

        SONOT__BAR(" "<<" "<<" "<<" ",
                   "5"<<"5"<<"2"<<"7");

        SONOT__BAR("7"<<" "<<" ",
                   "3"<<"2"<<"3");

        SONOT__BAR("7"<<" ",
                   "3"<<"4");

        SONOT__BAR(" "<<" "<<" "<<" ",
                   "5"<<"6"<<"7"<<"1");


        SONOT__BAR("5"<<" "<<" "<<" ",
                   "2"<<"3"<<"4"<<"5");

        SONOT__BAR("1-4"<<" ",
                   "6"  <<"3");

        SONOT__BAR(" "<<" "<<" ",
                   "4b"<<"3"<<"4");

        SONOT__BAR("1-4"<<" "<<" ",
                   "3"<<"2"<<"1");

        SONOT__BAR("7"<<" ",
                   "3"<<"5");

        SONOT__BAR(" "<<" "<<" ",
                   "4"<<"3"<<"2");

        SONOT__BAR("6"<<" ",
                   "1"<<"2");

        SONOT__BAR("7"<<" "<<" "<<" ",
                   " "<<"1"<<"7"<<"6");

        SONOT__BAR(" "<<" "<<" "<<" ",
                   "5"<<"4"<<"3"<<" ");

        stream.appendBar(rows);
    }
#undef SONOT__BAR

    Bar empty;
    empty << " " << " " << " " << " ";
    QList<Bar> rows;
    rows << empty << empty;
    for (int i=0; i<320; ++i)
        stream.appendBar(rows);

    return stream;
}


} // namespace Sonot
