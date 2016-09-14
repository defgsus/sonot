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
    {
        penLayoutFrame.setStyle(Qt::DotLine);
    }

    // --- gui / state ---

    void connectEditor(ScoreEditor*);
    void onMatrixChanged();
    void setAction(Action);
    void setCursor(const Score::Index& cursor);
    void clearCursor();

    // --- drawing ---

    bool doShowLayoutBoxes() const { return true; }

    void paintBackground(QPainter* p, const QRect& updateRect) const;
    void paintPage(QPainter* p, const QRect& updateRect, int pageIndex) const;
    void paintPageAnnotation
            (QPainter* p, const QRect& updateRect, int pageIndex) const;
    void paintScore(QPainter* p, const QRect& updateRect, int pageIndex) const;

    ScoreView* p;

    ScoreDocument* document;

    QTransform matrix, imatrix;

    // -- gui --

    Action action;
    QPointF lastMouseDown,
        // in document space
        lastMouseDownDoc;
    QTransform lastMouseDownMatrix;

    Score::Index cursor;
    QString inputString;

    // -- config --

    QBrush  brushBackground,
            brushPageBackground;
    QPen    penLayoutFrame,
            penScoreRow,
            penScoreItem,
            penCursor;
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
    if (!isAssigned())
        return;

    auto r = p_->document->pageRect();
    r.moveTo(p_->document->pagePosition(pageIndex));
    showRect(r, margin_mm);
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

void ScoreView::Private::setCursor(const Score::Index& cur)
{
    if (cur == cursor)
        return;
    if (cursor.isValid())
        p->updateCursor(cursor);
    cursor = cur;
    if (cursor.isValid())
        p->updateCursor(cursor);
}

void ScoreView::Private::clearCursor()
{
    setCursor(Score::Index());
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
        qDebug() << "old cursor" << p_->cursor.toString();
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
                p_->setCursor(cursor);
                p_->inputString.clear();
                qDebug() << "new cursor" << p_->cursor.toString();
            }
            return;
        }

        // ENTER NOTES
        if ((e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9)
          || (e->key() >= Qt::Key_A && e->key() <= Qt::Key_H)
          || (e->key() == Qt::Key_Backspace)
          || (e->key() == Qt::Key_X)
          || (e->key() == '#')
          )
        {
            if (e->key() == Qt::Key_Backspace)
                p_->inputString.chop(1);
            else
                p_->inputString += QChar(e->key());
            //qDebug() << p_->inputString;

            auto newVal = p_->inputString.isEmpty()
                    ? (int8_t)Note::Space
                    : Note::valueFromString(p_->inputString);
            if (newVal != Note::Invalid)
            {
                Note n = p_->cursor.getNote();
                n.setValue(newVal);
                editor()->changeNote(p_->cursor, n);
                emit noteEntered(n);
            }
            else
                // remove illegal character
                p_->inputString.chop(1);
        }

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
            p_->setCursor(idx);
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
    auto pageLayout = document->pageLayout(pageIndex);
    auto pagePos = document->pagePosition(pageIndex);

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
        p->setBrush(Qt::NoBrush);
        p->setPen(penLayoutFrame);
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
        QPainter* p, const QRect& /*updateRect*/, int pageIndex) const
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

    p->setPen(penScoreItem);
    document->paintScoreItems(*p, pageIndex);

    // cursor
    if (action == A_ENTER_NOTE && cursor.isValid())
    if (auto item = document->getScoreItem(cursor))
    {
        p->setPen(penCursor);
        p->setBrush(Qt::NoBrush);
        p->drawRect(item->boundingBox());
    }
}


} // namespace Sonot
