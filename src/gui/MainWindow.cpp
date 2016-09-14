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

#include "QProps/PropertiesView.h"

#include "MainWindow.h"
#include "ScoreView.h"
#include "ScoreDocument.h"
#include "ScoreLayout.h"
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
        , player    (new SamplePlayer(p))
        , synth     (new SynthDevice(p))
    { }

    ~Private()
    {
        delete document;
    }

    void createWidgets();

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

    SamplePlayer* player;
    SynthDevice* synth;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow   (parent)
    , p_            (new Private(this))
{
    setObjectName("MainWindow");
    setMinimumSize(320, 320);
    setGeometry(0,0,720,600);
    p_->createWidgets();

    p_->player->play(p_->synth, 1, p_->synth->sampleRate());

    //p_->playSomething();

    setScore(p_->getSomeScore());
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
        connect(scoreView, &ScoreView::noteEntered, [=](const Note& n)
        {
            if (n.isNote())
                synth->playNote(n.value());
        });

        propsView = new QProps::PropertiesView(p);
        lh->addWidget(propsView);

        propsView->setProperties(
            //scoreView->scoreDocument().scoreLayout(0).props()
            //scoreView->scoreDocument().pageLayout(0).marginsEven()
            //scoreView->scoreDocument().pageAnnotation(0).
            //scoreView->scoreDocument().pageAnnotation(0).textItems()[0].props()
            synth->synth().props()
            //TextItem().props()
            );
        connect(propsView, &QProps::PropertiesView::propertyChanged, [=]()
        {
            /*
            PageAnnotation anno = scoreView->scoreDocument().pageAnnotation(0);
            anno.textItems()[0].setProperties(propsView->properties());
            scoreView->scoreDocument().setPageAnnotation(0, anno);
            scoreView->update();
            */
            synth->setSynthProperties(propsView->properties());
        });
}

void MainWindow::showEvent(QShowEvent*)
{
    p_->scoreView->showPage(0);
}

void MainWindow::setScore(const Score& s)
{
    if (!p_->document)
    {
        p_->document = new ScoreDocument();
        p_->scoreView->setDocument(p_->document);
    }

    p_->document->setScore(s);
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
    qDebug().noquote() << stream.toString();
    score.appendNoteStream(stream);

    synth->setScore(score);
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

    return stream;
}


} // namespace Sonot
