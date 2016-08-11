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

#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QBrush>

#include "ScoreView.h"
#include "TextItem.h"

namespace Sonote {

struct ScoreView::Private
{
    Private(ScoreView* p)
        : p                 (p)
        , matrix            ()
        , brushBackground   (QColor(255,255,240))
    { }

    void paintPage(QPainter* p, int pageNum) const;

    ScoreView* p;

    QTransform matrix;

    // -- config --

    QBrush brushBackground;
};

ScoreView::ScoreView(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{

}

ScoreView::~ScoreView()
{
    delete p_;
}

void ScoreView::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    paintPage(&p, 0);
}

void ScoreView::paintPage(QPainter* p, int pageNum) const
{
    p_->paintPage(p, pageNum);
}

void ScoreView::Private::paintPage(QPainter *p, int pageNum) const
{
    p->save();
    p->setTransform(matrix);



    p->restore();
}



} // namespace Sonote
