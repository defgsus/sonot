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

#include <algorithm>

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
        , pageSpacing           (2., 10.)
    { }

    // --- gui / state ---

    void onMatrixChanged();
    void setAction(Action);

    // --- transformation ---

    int pageNumber(int pageIndex) const;
    QPointF pagePosition(int pageIndex) const;
    /** Returns the page index for a given point in document-space.
        Returns -1 if p is not on a page. */
    int pageIndexForDocumentPosition(const QPointF& p) const;

    // --- drawing ---

    bool doShowLayoutBoxes() const { return true; }

    void paintBackground(QPainter* p, const QRect& updateRect) const;
    void paintPage(QPainter* p, const QRect& updateRect, int pageIndex) const;
    void paintTextItems(QPainter* p, const QRect& updateRect, int pageIndex,
                        const QList<TextItem*>& items) const;

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
    QPointF pageSpacing;
};



ScoreView::ScoreView(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{
    grabKeyboard();

    auto i = new TextItem;
    i->setBoundingBox(QRectF(0,0,100,10));
    i->setBoxAlignment(Qt::AlignHCenter | Qt::AlignTop);
    i->setText("HCenter|Top");
    i->setFontFlags(TextItem::F_ITALIC);
    p_->textItems << i;

    i = new TextItem;
    i->setBoundingBox(QRectF(0,0,30,10));
    i->setBoxAlignment(Qt::AlignLeft | Qt::AlignTop);
    i->setText("TL");
    i->setFontFlags(TextItem::F_BOLD);
    p_->textItems << i;

    i = new TextItem;
    i->setBoundingBox(QRectF(0,0,20,10));
    i->setBoxAlignment(Qt::AlignRight | Qt::AlignBottom);
    i->setText("#page");
    i->setFontSize(5.);
    p_->textItems << i;

    qInfo().noquote() << i->toJsonString();
    p_->matrix.scale(2., 2.);

    goToPage(0);
}

ScoreView::~ScoreView()
{
    delete p_;
}




void ScoreView::goToPage(int pageIndex, double margin)
{
    auto p = p_->pagePosition(pageIndex) - QPointF(margin, margin);

    // current matrix without translation
    auto m = p_->matrix;
    m.setMatrix(m.m11(), m.m12(), m.m13(),
                m.m21(), m.m22(), m.m23(),
                0, 0, m.m33());

    // transform requested page position to current matrix
    p = m.map(p);

    // replace position
    p_->matrix.setMatrix(
                p_->matrix.m11(), p_->matrix.m12(), p_->matrix.m13(),
                p_->matrix.m21(), p_->matrix.m22(), p_->matrix.m23(),
                -p.x(), -p.y(), p_->matrix.m33());

    p_->onMatrixChanged();
    update();
}

void ScoreView::showRect(const QRectF& dst, double bo)
{
    QRectF src = QRectF(rect());
    double dx = src.width() / std::max(1., 2.*bo+dst.width()),
           dy = src.height() / std::max(1., 2.*bo+dst.height());
    dx = std::min(dx, dy);
    QTransform m;
    m.scale(dx, dx);
    m.translate(-dst.x() + bo, -dst.y() + bo);

    p_->matrix = m;
    p_->onMatrixChanged();
    update();
}

void ScoreView::showPage(int pageIndex, double margin_mm)
{
    auto r = p_->pageLayout.pageRect();
    r.moveTo(p_->pagePosition(pageIndex));
    showRect(r, margin_mm);
}





// ######################### EVENTS #########################

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
    qDebug() << matrix.map(QPointF(0,0));
    imatrix = matrix.inverted();
}

void ScoreView::keyPressEvent(QKeyEvent* e)
{
    if (e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9)
    {
        if (e->modifiers() != 0)
            showPage(e->key() - Qt::Key_0);
        else
            goToPage(e->key() - Qt::Key_0);
    }
}

void ScoreView::mousePressEvent(QMouseEvent* e)
{
    qDebug() << p_->pageIndexForDocumentPosition(QPointF(
                    p_->imatrix.map(e->pos())));

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



// ################# transformation #################################


int ScoreView::Private::pageNumber(int pageIndex) const
{
    return pageIndex + 1;
}

QPointF ScoreView::Private::pagePosition(int pageIndex) const
{
    auto r = pageLayout.pageRect();
    ++pageIndex;
    return QPointF(
                (pageIndex % 2) * (r.width() + pageSpacing.x()),
                (pageIndex / 2) * (r.height() + pageSpacing.y()));
}

int ScoreView::Private::pageIndexForDocumentPosition(const QPointF& p0) const
{
    // TODO: not finished

    if (p0.x() < 0.)
        return -1;
    auto r = pageLayout.pageRect();
    QPointF p(p0);
    p -= r.topLeft();
    p.rx() /= (r.width() + pageSpacing.x());
    p.ry() /= (r.height() + pageSpacing.y());
    if (p.rx() >= 2. || p.ry() < 0.)
        return -1;

    return int(p.ry()) * 2 + int(p.rx()) + 1;
}




// ########################### DRAWING ##############################


void ScoreView::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    p_->paintBackground(&p, e->rect());
    for (int i=0; i<5000; ++i)
        p_->paintPage(&p, e->rect(), i);
}


void ScoreView::Private::paintBackground(
        QPainter *p, const QRect &updateRect) const
{
    // widget background
    auto r = this->p->rect();
    p->fillRect(r.adjusted(-1,-1,1,1), brushBackground);

}

void ScoreView::Private::paintPage(
        QPainter *p, const QRect& updateRect, int pageIndex) const
{
    auto mat = matrix;
    auto pagePos = pagePosition(pageIndex);
    mat.translate(pagePos.x(), pagePos.y());

    {
        auto r = mat.mapRect( pageLayout.pageRect() );
        if (!QRectF(this->p->rect()).intersects(r))
            return;
    }

    p->save();
    p->setTransform(mat);

    // page packground
    p->fillRect(pageLayout.pageRect(), brushPageBackground);

    // content borders / margins
    if (doShowLayoutBoxes())
    {
        p->setPen(Qt::DotLine);
        p->setBrush(Qt::NoBrush);
        p->drawRect(pageLayout.contentRect(pageIndex));
    }

    paintTextItems(p, updateRect, pageIndex, textItems);

    p->restore();
}

void ScoreView::Private::paintTextItems(
        QPainter* p, const QRect& updateRect, int pageIndex,
        const QList<TextItem*>& items) const
{
    auto pageRect = pageLayout.pageRect();
    auto contentRect = pageLayout.contentRect(pageIndex);
    auto pageNum = pageNumber(pageIndex);

    for (auto item : items)
    {
        auto box = item->alignedBoundingBox(contentRect);

        // show text box
        if (doShowLayoutBoxes())
        {
            p->setBrush(Qt::NoBrush);
            p->setPen(Qt::DashDotDotLine);
            p->drawRect(box);
        }

        auto text = item->text();
        text.replace("#page", QString::number(pageNum));

        if (text.isEmpty())
            continue;

        p->setPen(item->color());
        p->setFont(item->font());
        p->drawText(box, item->textAlignment() | item->textFlags(),
                    text, &pageRect);
    }
}

} // namespace Sonote
