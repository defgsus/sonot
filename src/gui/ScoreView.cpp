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

#include <QDebug>

#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QBrush>

#include "ScoreView.h"
#include "TextItem.h"
#include "PageLayout.h"

namespace Sonote {

struct ScoreView::Private
{
    enum Action
    {
        A_NOTHING,
        A_DRAG_TRANSLATE,
        A_DRAG_ZOOM
    };

    Private(ScoreView* p)
        : p                     (p)
        , matrix                ()
        , action                (A_NOTHING)
        , brushBackground       (QColor(155,155,155))
        , brushPageBackground   (QColor(255,255,240))
    { }

    bool showLayoutBoxes() const { return true; }
    void paintPage(QPainter* p, const QRect& updateRect, int pageNum) const;
    void paintTextItems(QPainter* p, const QRect& updateRect, int pageNum,
                        const QList<TextItem*>& items) const;
    void onMatrixChanged();
    void setAction(Action);

    ScoreView* p;

    PageLayout pageLayout;
    QTransform matrix, imatrix;

    QList<TextItem*> textItems;

    // -- gui --

    Action action;
    QPointF lastMouseDown;
    QTransform lastMouseDownMatrix;

    // -- config --

    QBrush  brushBackground,
            brushPageBackground;
};

ScoreView::ScoreView(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{
    auto i = new TextItem;
    i->setBoundingBox(QRectF(0,0,100,10));
    i->setBoxAlignment(Qt::AlignHCenter | Qt::AlignTop);
    i->setText("HCenter|Top");
    p_->textItems << i;

    i = new TextItem;
    i->setBoundingBox(QRectF(0,0,30,10));
    i->setBoxAlignment(Qt::AlignLeft | Qt::AlignTop);
    i->setText("TL");
    p_->textItems << i;

    i = new TextItem;
    i->setBoundingBox(QRectF(0,0,20,10));
    i->setBoxAlignment(Qt::AlignRight | Qt::AlignBottom);
    i->setText("BR");
    i->setFontSize(5.);
    p_->textItems << i;

    p_->matrix.scale(2., 2.);
    p_->onMatrixChanged();
}

ScoreView::~ScoreView()
{
    delete p_;
}

void ScoreView::Private::setAction(Action a)
{
    action = a;
    switch (action)
    {
        case A_NOTHING: p->setCursor(Qt::ArrowCursor); break;
        case A_DRAG_TRANSLATE: p->setCursor(Qt::ClosedHandCursor); break;
        case A_DRAG_ZOOM: p->setCursor(Qt::SizeVerCursor); break;
    }
}

void ScoreView::Private::onMatrixChanged()
{
    imatrix = matrix.inverted();
}

void ScoreView::mousePressEvent(QMouseEvent* e)
{
    p_->lastMouseDown = QPointF(e->pos());
    p_->lastMouseDownMatrix = p_->matrix;

    if (e->button() == Qt::LeftButton)
    {
        p_->setAction(Private::A_DRAG_TRANSLATE);
    }
    else if (e->button() == Qt::RightButton)
    {
        p_->setAction(Private::A_DRAG_ZOOM);
    }
}

void ScoreView::mouseReleaseEvent(QMouseEvent* e)
{
    p_->setAction(Private::A_NOTHING);
}

void ScoreView::mouseMoveEvent(QMouseEvent* e)
{
    QPointF delta =   p_->imatrix.map(QPointF(e->pos()))
                    - p_->imatrix.map(p_->lastMouseDown);

    if (p_->action == Private::A_DRAG_TRANSLATE)
    {

        p_->matrix = p_->lastMouseDownMatrix;
        p_->matrix.translate(delta.x(), delta.y());
        p_->onMatrixChanged();

        update();
        return;
    }

    if (p_->action == Private::A_DRAG_ZOOM)
    {
        auto imatrix = p_->lastMouseDownMatrix.inverted();
        QPointF delta =   imatrix.map(QPointF(e->pos()))
                        - imatrix.map(p_->lastMouseDown);
        // zoom strength
        const double f =  imatrix.map(QPointF(10.,10.)).x()
                        - imatrix.map(QPointF(0.,0.)).x();
        // scale factor
        const double sf = delta.y() <= 0.
                ? 1.-delta.y()/f
                : 1./(1.+delta.y()/f);
        const QPointF anchor = imatrix.map(p_->lastMouseDown);

        p_->matrix = p_->lastMouseDownMatrix;
        p_->matrix.translate(anchor.x(), anchor.y());
        p_->matrix.scale(sf, sf);
        p_->matrix.translate(-anchor.x(), -anchor.y());
        p_->onMatrixChanged();

        update();
    }
}



// ########################### DRAWING ##############################


void ScoreView::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    p_->paintPage(&p, e->rect(), 0);
}

void ScoreView::paintPage(QPainter* p, int pageNum) const
{
    p_->paintPage(p, rect(), pageNum);
}

void ScoreView::Private::paintPage(
        QPainter *p, const QRect& updateRect, int pageNum) const
{
    p->save();
    p->setTransform(matrix);

    // widget background
    auto r = imatrix.mapRect(this->p->rect());
    p->fillRect(r.adjusted(-1,-1,1,1), brushBackground);

    // page packground
    p->fillRect(pageLayout.pageRect(), brushPageBackground);

    // content borders / margins
    if (showLayoutBoxes())
    {
        p->setPen(Qt::DotLine);
        p->setBrush(Qt::NoBrush);
        p->drawRect(pageLayout.contentRect(0));
    }

    paintTextItems(p, updateRect, pageNum, textItems);

    p->restore();
}

void ScoreView::Private::paintTextItems(
        QPainter* p, const QRect& updateRect, int pageNum,
        const QList<TextItem*>& items) const
{
    auto pageRect = pageLayout.pageRect();

    for (auto item : items)
    {
        auto box = item->alignedBoundingBox(
                            pageLayout.contentRect(pageNum));

        // show text box
        if (showLayoutBoxes())
        {
            p->setBrush(Qt::NoBrush);
            p->setPen(Qt::DashDotDotLine);
            p->drawRect(box);
        }

        p->setPen(Qt::black);
        p->setFont(item->font());
        p->drawText(box, item->textAlignment() | item->textFlags(),
                    item->text(), &pageRect);
    }
}

} // namespace Sonote
