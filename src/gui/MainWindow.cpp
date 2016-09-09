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

#include "MainWindow.h"
#include "ScoreView.h"
#include "ScoreDocument.h"
#include "ScoreLayout.h"
#include "PageLayout.h"
#include "TextItem.h"
#include "PropertiesView.h"

namespace Sonot {

struct MainWindow::Private
{
    Private(MainWindow*w)
        : p         (w)
    { }

    void createWidgets();

    MainWindow* p;

    ScoreView* scoreView;
    PropertiesView* propsView;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow   (parent)
    , p_            (new Private(this))
{
    setObjectName("MainWindow");
    setMinimumSize(320, 320);
    setGeometry(0,0,720,600);
    p_->createWidgets();

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

        propsView = new PropertiesView(p);
        lh->addWidget(propsView);

        propsView->setProperties(
            //scoreView->scoreDocument().scoreLayout(0).props()
            //scoreView->scoreDocument().pageLayout(0).marginsEven()
            //scoreView->scoreDocument().pageAnnotation(0).
            scoreView->scoreDocument().pageAnnotation(0).textItems()[0].props()
            //TextItem().props()
            );
        connect(propsView, &PropertiesView::propertyChanged, [=]()
        {
            PageAnnotation anno = scoreView->scoreDocument().pageAnnotation(0);
            anno.textItems()[0].setProperties(propsView->properties());
            scoreView->scoreDocument().setPageAnnotation(0, anno);
            scoreView->update();
        });
}

void MainWindow::showEvent(QShowEvent*)
{
    p_->scoreView->showPage(0);
}

} // namespace Sonot
