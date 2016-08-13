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

#include "MainWindow.h"
#include "ScoreView.h"

namespace Sonote {

struct MainWindow::Private
{
    Private(MainWindow*w)
        : p         (w)
    { }

    void createWidgets();

    MainWindow* p;

    ScoreView* scoreView;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow   (parent)
    , p_            (new Private(this))
{
    setObjectName("MainWindow");
    setMinimumSize(320, 320);

    p_->createWidgets();
}

MainWindow::~MainWindow()
{
    delete p_;
}

void MainWindow::Private::createWidgets()
{
    scoreView = new ScoreView(p);
    scoreView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    p->setCentralWidget(scoreView);
}

} // namespace Sonote
