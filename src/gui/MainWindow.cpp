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

namespace Sonot {

struct MainWindow::Private
{
    Private(MainWindow*w)
        : p         (w)
        , player    (new SamplePlayer(p))
        , synth     (new SynthDevice(p))
    { }

    void createWidgets();

    void playSomething();

    MainWindow* p;

    ScoreView* scoreView;
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

    p_->playSomething();
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

    NoteStream stream;

    for (int i=0; i<8; ++i)
    {
        QList<Bar> rows;

        if (i % 3 < 2)
        {
            Bar bar(4);
            bar.setNote(0, Note(Note::C, 5));
            bar.setNote(1, Note(Note::Dx, 5));
            bar.setNote(2, Note(Note::G, 5));
            bar.setNote(3, Note(Note::A, 5));
            rows << bar;
        }
        if (i % 2 == 1)
        {
            Bar bar(4);
            bar.setNote(0, Note(Note::C, 6));
            bar.setNote(1, Note(Note::Dx, 6));
            bar.setNote(2, Note(Note::G, 6));
            bar.setNote(3, Note(Note::D, 5));
            rows << bar;
        }
        if (i > 1)
        {
            Bar bar(3);
            bar.setNote(0, Note(Note::C, 6));
            bar.setNote(1, Note(Note::F, 6));
            bar.setNote(2, Note(Note::D, 6));
            rows << bar;
        }
        if (i % 4 == 0)
        {
            Bar bar(4);
            bar.setNote(0, Note(Note::C, 6));
            bar.setNote(2, Note(Note::G, 6));
            bar.setNote(3, Note(Note::D, 7));
            rows << bar;
        }

        stream.appendBar(rows);
    }

    Score score;
    score.appendNoteStream(stream);

    synth->setScore(score);
    player->play(synth, 1, synth->sampleRate());
}

} // namespace Sonot
