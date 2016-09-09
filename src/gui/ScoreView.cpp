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
#include "ScoreDocument.h"
#include "ScoreLayout.h"
#include "TextItem.h"
#include "PageLayout.h"
#include "core/Score.h"

namespace Sonot {

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
        , penScoreRow           (QColor(180,180,180))
    {

    }

    // --- gui / state ---

    void onMatrixChanged();
    void setAction(Action);


    // --- drawing ---

    bool doShowLayoutBoxes() const { return true; }

    void paintBackground(QPainter* p, const QRect& updateRect) const;
    void paintPage(QPainter* p, const QRect& updateRect, int pageIndex) const;
    void paintPageAnnotation
            (QPainter* p, const QRect& updateRect, int pageIndex) const;
    void paintScore(QPainter* p, const QRect& updateRect, int pageIndex) const;

    ScoreView* p;

    ScoreDocument document;

    QTransform matrix, imatrix;

    // -- gui --

    Action action;
    QPointF lastMouseDown;
    QTransform lastMouseDownMatrix;

    // -- config --

    QBrush  brushBackground,
            brushPageBackground;
    QPen    penScoreRow;
};



ScoreView::ScoreView(QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(this))
{
    //grabKeyboard();

    p_->matrix.scale(2., 2.);

    goToPage(0);
}

ScoreView::~ScoreView()
{
    delete p_;
}

const ScoreDocument& ScoreView::scoreDocument() const { return p_->document; }
ScoreDocument& ScoreView::scoreDocument() { return p_->document; }


void ScoreView::goToPage(int pageIndex, double margin)
{
    auto p = p_->document.pagePosition(pageIndex)
                            - QPointF(margin, margin);

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
    auto r = p_->document.pageRect();
    r.moveTo(p_->document.pagePosition(pageIndex));
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
    //qDebug() << matrix.map(QPointF(0,0));
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
    //qDebug() << p_->pageIndexForDocumentPosition(QPointF(
    //                p_->imatrix.map(e->pos())));

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

void ScoreView::mouseReleaseEvent(QMouseEvent* )
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
        QPointF delta =   imatrix.map(p_->lastMouseDown)
                        - imatrix.map(QPointF(e->pos()));
        // zoom strength
        const double f =  imatrix.map(QPointF(20.,20.)).x()
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

    p_->paintBackground(&p, e->rect());
    for (int i=0; i<500; ++i)
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
    auto pageLayout = document.pageLayout(pageIndex);
    auto pagePos = document.pagePosition(pageIndex);

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
        p->drawRect(pageLayout.contentRect());
        p->drawRect(pageLayout.scoreRect());
    }

    paintPageAnnotation(p, updateRect, pageIndex);
    paintScore(p, updateRect, pageIndex);

    p->restore();
}

void ScoreView::Private::paintPageAnnotation(
        QPainter* p, const QRect& /*updateRect*/, int pageIndex) const
{
    PageAnnotation anno = document.pageAnnotation(pageIndex);

    auto pageLayout = document.pageLayout(pageIndex);
    auto pageRect = pageLayout.pageRect();
    auto contentRect = pageLayout.contentRect();
    auto pageNum = document.pageNumberForIndex(pageIndex);

    for (const TextItem& item : anno.textItems())
    {
        auto box = item.alignedBoundingBox(contentRect);

        // show text box
        if (doShowLayoutBoxes())
        {
            p->setBrush(Qt::NoBrush);
            p->setPen(Qt::DashDotDotLine);
            p->drawRect(box);
        }

        auto text = item.text();
        if (text.isEmpty())
            continue;

        // fill in tags
        text.replace("#page", QString::number(pageNum));
        auto props = document.score().props();
        for (auto i = props.begin(); i!=props.end(); ++i)
            text.replace("#" + i.key(), i.value().toString());

        if (text.isEmpty())
            continue;

        p->setPen(item.color());
        p->setFont(item.font());
        p->drawText(box, item.textAlignment() | item.textFlags(),
                    text, &pageRect);
    }
}

void ScoreView::Private::paintScore(
        QPainter* p, const QRect& /*updateRect*/, int pageIndex) const
{
    auto pageLayout = document.pageLayout(pageIndex);
    auto scoreLayout = document.scoreLayout(pageIndex);
    auto srect = pageLayout.scoreRect();

    double y = 0.;
    while (y < srect.height())
    {
        int numRows = 3;
        if (y + scoreLayout.lineHeight(numRows) >= srect.height())
            break;

        p->setPen(penScoreRow);
        for (int i=0; i<numRows; ++i, y += scoreLayout.rowSpacing())
            p->drawLine(srect.left(), srect.top() + y,
                        srect.right(), srect.top() + y);

        y += scoreLayout.lineSpacing();
    }
}


} // namespace Sonot
