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

#include "QProps/error.h"

#include "ScoreView.h"
#include "ScoreDocument.h"
#include "ScoreLayout.h"
#include "TextItem.h"
#include "PageLayout.h"
#include "ScoreItem.h"
#include "core/NoteStream.h"
#include "core/Score.h"
#include "core/ScoreEditor.h"

namespace Sonot {

struct ScoreView::Private
{
    enum Action
    {
        A_NOTHING,
        A_DRAG_TRANSLATE,
        A_DRAG_ZOOM,
        A_ENTER_NOTE
    };

    Private(ScoreView* p)
        : p                     (p)
        , document              (nullptr)
        , matrix                ()
        , action                (A_NOTHING)
        , brushBackground       (QColor(155,155,155))
        , brushPageBackground   (QColor(255,255,240))
        , penLayoutFrame        (QColor(0,50,50,30))
        , penScoreRow           (QColor(0,0,0,50))
        , penScoreItem          (QColor(0,0,0))
        , penCursor             (QColor(0,60,100,50))
        , penPlayCursor         (QColor(0,200,0,40))
    {
        penLayoutFrame.setStyle(Qt::DotLine);
    }

    // --- gui / state ---

    void connectEditor(ScoreEditor*);
    void onMatrixChanged();
    void setAction(Action);
    void setCursor(const Score::Index& cursor, bool ensureVisible);
    void clearCursor();

    // --- drawing ---

    bool doShowLayoutBoxes() const { return true; }

    void paintBackground(QPainter* p, const QRect& updateRect) const;
    void paintPage(QPainter* p, const QRectF& updateRect, int pageIndex) const;
    void paintPageAnnotation
            (QPainter* p, const QRectF& updateRect, int pageIndex) const;
    void paintScore(QPainter* p, const QRectF& updateRect, int pageIndex) const;

    ScoreView* p;

    ScoreDocument* document;

    QTransform matrix, imatrix;

    // -- gui --

    Action action;
    QPointF lastMouseDown,
        // in document space
        lastMouseDownDoc;
    QTransform lastMouseDownMatrix;

    Score::Index cursor, playCursor;
    QString inputString;

    // -- config --

    QBrush  brushBackground,
            brushPageBackground;
    QPen    penLayoutFrame,
            penScoreRow,
            penScoreItem,
            penCursor,
            penPlayCursor;
};



ScoreView::ScoreView(QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(this))
{
    grabKeyboard();

    p_->matrix.scale(2., 2.);
}

ScoreView::~ScoreView()
{
    delete p_;
}

ScoreDocument* ScoreView::scoreDocument() const { return p_->document; }
ScoreEditor* ScoreView::editor() const { return p_->document->editor(); }

void ScoreView::setDocument(ScoreDocument* doc)
{
    p_->document = doc;
    p_->cursor = Score::Index();
    if (p_->document)
        p_->connectEditor(p_->document->editor());

    goToPage(0);
}

void ScoreView::goToPage(int pageIndex, double margin)
{
    if (!isAssigned())
        return;

    auto p = p_->document->pagePosition(pageIndex)
                            - QPointF(margin, margin);
    moveToPoint(p);
}

void ScoreView::moveToPoint(const QPointF &p)
{
    // current matrix without translation
    auto m = p_->matrix;
    m.setMatrix(m.m11(), m.m12(), m.m13(),
                m.m21(), m.m22(), m.m23(),
                0, 0, m.m33());

    // transform requested position to current matrix
    auto p2 = m.map(p);

    // replace position
    p_->matrix.setMatrix(
                p_->matrix.m11(), p_->matrix.m12(), p_->matrix.m13(),
                p_->matrix.m21(), p_->matrix.m22(), p_->matrix.m23(),
                -p2.x(), -p2.y(), p_->matrix.m33());

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
    if (!isAssigned())
        return;

    auto r = p_->document->pageRect();
    r.moveTo(p_->document->pagePosition(pageIndex));
    showRect(r, margin_mm);
}

bool ScoreView::ensureIndexVisible(const Score::Index& idx)
{
    if (!isAssigned())
        return false;
    auto item = p_->document->getScoreItem(idx);
    if (!item)
        return false;

    QRectF view = p_->imatrix.map(QRectF(rect())).boundingRect();
    QRectF dst = item->boundingBox().adjusted(-5,-5,5,5);
    dst.moveTo(dst.topLeft()
               + p_->document->pagePosition(item->docIndex().pageIdx));
    if (!view.contains(dst))
    {
        moveToPoint(dst.topLeft());
        return true;
    }
    return false;
}

void ScoreView::updateIndex(const Score::Index& idx, double m)
{
    if (isAssigned())
    if (auto item = p_->document->getScoreItem(idx))
    {
        QRectF dst = item->boundingBox();
        dst.moveTo(dst.topLeft()
               + p_->document->pagePosition(item->docIndex().pageIdx));
        update(mapFromDocument(dst.adjusted(-m, -m, m, m)));
    }
}

void ScoreView::setPlayingIndex(const Score::Index& cur)
{
    if (cur == p_->playCursor)
        return;
    if (p_->playCursor.isValid())
        updateIndex(p_->playCursor);
    p_->playCursor = cur;
    if (p_->playCursor.isValid())
        updateIndex(p_->playCursor);
}

QRect ScoreView::mapFromDocument(const QRectF& r)
{
    return p_->matrix.mapRect(r).toRect();
}

QRectF ScoreView::mapToDocument(const QRect& r)
{
    return p_->imatrix.mapRect(r);
}



// ######################### EVENTS #########################

void ScoreView::Private::connectEditor(ScoreEditor* editor)
{
    connect(editor, &ScoreEditor::scoreReset, [=]()
    {
        cursor = document->score()->index(0,0,0,0);
        setAction(cursor.isValid()
                      ? Private::A_ENTER_NOTE
                      : Private::A_NOTHING);
        p->showPage(0);
    });
    connect(editor, &ScoreEditor::barsChanged, [=]()
    {
        p->update();
    });
    connect(editor, &ScoreEditor::barsDeleted, [=]()
    {
        p->update();
    });
    connect(editor, &ScoreEditor::noteValuesChanged, [=]()
    {
        p->update();
    });
    connect(editor, &ScoreEditor::notesDeleted, [=]()
    {
        p->update();
    });
    connect(editor, &ScoreEditor::streamsChanged, [=]()
    {
        p->update();
    });
    connect(editor, &ScoreEditor::streamsDeleted, [=]()
    {
        p->update();
    });

}

void ScoreView::Private::setAction(Action a)
{
    action = a;
    switch (action)
    {
        case A_ENTER_NOTE:
        case A_NOTHING: p->setCursor(Qt::ArrowCursor); break;
        case A_DRAG_TRANSLATE: p->setCursor(Qt::ClosedHandCursor); break;
        case A_DRAG_ZOOM: p->setCursor(Qt::SizeVerCursor); break;
    }
    //qDebug() << "ACTION" << action;
}

void ScoreView::Private::setCursor(const Score::Index& cur, bool ensureVisible)
{
    if (cur == cursor)
        return;
    if (cursor.isValid())
        p->updateIndex(cursor);
    cursor = cur;
    if (ensureVisible)
    {
        if (cursor.isValid())
            if (!p->ensureIndexVisible(cursor))
                p->updateIndex(cursor);
        return;
    }
    if (cursor.isValid())
        p->updateIndex(cursor);
}

void ScoreView::Private::clearCursor()
{
    setCursor(Score::Index(), false);
}


void ScoreView::Private::onMatrixChanged()
{
    //qDebug() << matrix.map(QPointF(0,0));
    imatrix = matrix.inverted();
}

void ScoreView::keyPressEvent(QKeyEvent* e)
{
    if (!isAssigned())
    {
        e->ignore();
        return;
    }

    const bool
            isShift = e->modifiers() & Qt::SHIFT;

    //qDebug() << p_->action << p_->cursor.toString();

    if (p_->action == Private::A_ENTER_NOTE && p_->cursor.isValid())
    {
        // NAVIGATION
        bool handled = true;
        auto cursor = p_->cursor;
        switch (e->key())
        {
            case Qt::Key_Left:
                if (isShift)
                    cursor.prevBar();
                else
                    cursor.prevNote();
            break;
            case Qt::Key_Right:
                if (isShift)
                    cursor.nextBar();
                else
                    cursor.nextNote();
            break;
            case Qt::Key_Tab: cursor.nextBar(); break;
            case Qt::Key_Up:
                if (!cursor.prevRow())
                {
                    const ScoreLayout& l = p_->document->scoreLayout(
                            p_->document->pageIndexForScoreIndex(cursor));
                    if (l.isFixedBarWidth())
                    {
                        if (cursor.bar() >= l.barsPerLine())
                        {
                            if (cursor.prevBar(l.barsPerLine()))
                                // go to bottom row
                                while (cursor.nextRow());
                        }
                    }
                    else QPROPS_PROG_ERROR("Non-fixed layout no implemented");
                }
            break;
            case Qt::Key_Down:
                if (!cursor.nextRow())
                {
                    const ScoreLayout& l = p_->document->scoreLayout(
                            p_->document->pageIndexForScoreIndex(cursor));
                    if (l.isFixedBarWidth())
                    {
                        if (cursor.nextBar(l.barsPerLine()))
                            // go to top row
                            while (cursor.prevRow());
                    }
                    else QPROPS_PROG_ERROR("Non-fixed layout no implemented");
                }
            break;
            default: handled = false;
        }
        if (handled)
        {
            if (cursor != p_->cursor)
            {
                p_->setCursor(cursor, true);
                p_->inputString.clear();
            }
            return;
        }

        // ENTER NOTES
        handled = true;
        switch (e->key())
        {
            case Qt::Key_0:
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
                p_->inputString += QChar(e->key());
            break;
            case Qt::Key_A:
            case Qt::Key_B:
            case Qt::Key_C:
            case Qt::Key_D:
            case Qt::Key_E:
            case Qt::Key_F:
            case Qt::Key_G:
            case Qt::Key_H:
            case Qt::Key_I:
            case Qt::Key_S:
            case Qt::Key_X:
            case Qt::Key_NumberSign: // #
            case Qt::Key_Comma:
            case Qt::Key_QuoteLeft: // '
                p_->inputString += QChar(e->key());
            break;

            case Qt::Key_Space:
                if (p_->inputString.size() < 2)
                    p_->inputString.clear();
            case Qt::Key_Backspace:
                p_->inputString.chop(1);
            break;
            case Qt::Key_Delete:
                p_->inputString.clear();
            break;

            default: handled = false;
        }

        if (!handled)
        {
            e->ignore();
            return;
        }

        Note n = p_->cursor.getNote();
        auto newVal = p_->inputString.isEmpty()
                        ? (int8_t)Note::Space
                        : Note::valueFromString(p_->inputString);
        if (newVal != Note::Invalid && newVal != n.value())
        {
            n.setValue(newVal);
            editor()->changeNote(p_->cursor, n);
            emit noteEntered(n);
        }
        else
            // remove illegal or unused character
            p_->inputString.chop(1);

        return;
    }

    if (e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9)
    {
        if (e->modifiers() != 0)
            showPage(e->key() - Qt::Key_0);
        else
            goToPage(e->key() - Qt::Key_0);
        return;
    }

    e->ignore();
}

void ScoreView::mousePressEvent(QMouseEvent* e)
{
    if (!isAssigned())
        return;

    //qDebug() << p_->pageIndexForDocumentPosition(QPointF(
    //                p_->imatrix.map(e->pos())));

    p_->lastMouseDown = QPointF(e->pos());
    p_->lastMouseDownDoc = p_->imatrix.map(p_->lastMouseDown);
    p_->lastMouseDownMatrix = p_->matrix;

    Score::Index idx = p_->document->getScoreIndex(p_->lastMouseDownDoc);
    //qDebug() << idx.toString();

    if (e->button() == Qt::LeftButton)
    {
        if (idx.isValid())
        {
            p_->setCursor(idx, false);
            p_->setAction(Private::A_ENTER_NOTE);
            p_->inputString.clear();
            return;
        }
        else
        {
            p_->clearCursor();
            p_->setAction(Private::A_DRAG_TRANSLATE);
            return;
        }
    }
    else if (e->button() == Qt::RightButton)
    {
        p_->setAction(Private::A_DRAG_ZOOM);
        return;
    }

    e->ignore();
}

void ScoreView::mouseReleaseEvent(QMouseEvent* )
{
    if (!isAssigned())
        return;

    if (p_->action != Private::A_ENTER_NOTE)
    {
        p_->setAction(Private::A_NOTHING);
    }
}

void ScoreView::mouseMoveEvent(QMouseEvent* e)
{
    if (!isAssigned())
        return;

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

    if (!isAssigned())
        return;

    QRectF docUpdate = mapToDocument(e->rect());

    for (int i=0; i<100; ++i)
        p_->paintPage(&p, docUpdate, i);

    /*
    p.setBrush(QBrush(Qt::red));
    QRectF r(0,0,5,5);
    r.moveTo(p_->document->pagePosition(0));
    p.drawRect(mapFromDocument(r));
    */
}


void ScoreView::Private::paintBackground(
        QPainter* p, const QRect& /*updateRect*/) const
{
    // widget background
    auto r = this->p->rect();
    p->fillRect(r.adjusted(-1,-1,1,1), brushBackground);

}

void ScoreView::Private::paintPage(
        QPainter *p, const QRectF& updateRect, int pageIndex) const
{
    {
        QRectF pr = document->pageRect(pageIndex);
        if (!pr.intersects(updateRect))
            return;
    }

    auto pageLayout = document->pageLayout(pageIndex);
    auto pagePos = document->pagePosition(pageIndex);

    auto mat = matrix;
    mat.translate(pagePos.x(), pagePos.y());

    p->save();
    p->setTransform(mat);

    // page packground
    //QBrush b(QColor(rand()&0xff,rand()&0xff,rand()&0xff));
    p->fillRect(pageLayout.pageRect(), brushPageBackground);

    // content borders / margins
    if (doShowLayoutBoxes())
    {
        p->setBrush(Qt::NoBrush);
        p->setPen(penLayoutFrame);
        p->drawRect(pageLayout.contentRect());
        p->drawRect(pageLayout.scoreRect());
    }

    // page-local update rect
    QRectF uRect = updateRect;
    uRect.moveTo(updateRect.x()-pagePos.x(),
                 updateRect.y()-pagePos.y());

    paintPageAnnotation(p, uRect, pageIndex);
    paintScore(p, uRect, pageIndex);

    p->restore();
}

void ScoreView::Private::paintPageAnnotation(
        QPainter* p, const QRectF& /*updateRect*/,
        int pageIndex) const
{
    PageAnnotation anno = document->pageAnnotation(pageIndex);

    auto pageLayout = document->pageLayout(pageIndex);
    auto pageRect = pageLayout.pageRect();
    auto contentRect = pageLayout.contentRect();
    auto pageNum = document->pageNumberForIndex(pageIndex);

    for (const TextItem& item : anno.textItems())
    {
        auto box = item.alignedBoundingBox(contentRect);

        // show text box
        if (doShowLayoutBoxes())
        {
            p->setBrush(Qt::NoBrush);
            p->setPen(penLayoutFrame);
            p->drawRect(box);
        }

        auto text = item.text();
        if (text.isEmpty())
            continue;

        // fill in tags
        text.replace("#page", QString::number(pageNum));
        auto props = document->score()->props();
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
        QPainter* p, const QRectF& updateRect, int pageIndex) const
{
    auto pageLayout = document->pageLayout(pageIndex);
    auto scoreLayout = document->scoreLayout(pageIndex);
    //auto srect = pageLayout.scoreRect();

    /*
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
    */

    //p->setBrush(QBrush(QColor(rand()&0xff,rand()&0xff,rand()&0xff)));
    //p->drawRect(updateRect);

    p->setPen(penScoreItem);
    document->paintScoreItems(*p, pageIndex, updateRect);

    // play cursor
    if (playCursor.isValid())
    if (auto item = document->getScoreItem(playCursor))
    if (item->docIndex().pageIdx == pageIndex)
    {
        p->setPen(penPlayCursor);
        p->setBrush(Qt::NoBrush);
        p->drawRect(item->boundingBox());
    }

    // cursor
    if (action == A_ENTER_NOTE && cursor.isValid())
    if (auto item = document->getScoreItem(cursor))
    if (item->docIndex().pageIdx == pageIndex)
    {
        p->setPen(penCursor);
        p->setBrush(Qt::NoBrush);
        p->drawRect(item->boundingBox());
    }
}


} // namespace Sonot
