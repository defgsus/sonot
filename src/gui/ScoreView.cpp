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
#include <QAction>
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
#include "core/KeySignature.h"
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
        , curOctave             (3)
        , brushBackground       (QColor(155,155,155))
        , brushPageBackground   (QColor(255,255,240))
        , penLayoutFrame        (QColor(0,50,50,30))
        , penScoreRow           (QColor(0,0,0,50))
        , penScoreItem          (QColor(0,0,0))
        , penCursor             (QColor(0,60,100,50))
        , penPlayCursor         (QColor(0,160,0,60))
    {
        penScoreItem.setWidthF(.5);
        penLayoutFrame.setStyle(Qt::DotLine);
    }

    // --- gui / state ---

    void connectEditor(ScoreEditor*);
    void onMatrixChanged();
    void setAction(Action);
    void setCursor(const Score::Index& cursor, bool ensureVisible);
    void clearCursor();
    void updateStatus();

    // --- helper ---


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
    int curOctave;

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
    //grabKeyboard();
    setFocusPolicy(Qt::StrongFocus);

    p_->matrix.scale(2., 2.);
}

ScoreView::~ScoreView()
{
    delete p_;
}

const Score* ScoreView::score() const
    { return p_->document ? p_->document->score() : nullptr; }
ScoreDocument* ScoreView::scoreDocument() const { return p_->document; }
ScoreEditor* ScoreView::editor() const { return p_->document->editor(); }
const Score::Index& ScoreView::currentIndex() const { return p_->cursor; }

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

    // current view in doc-space
    QRectF view = mapToDocument(rect());
    // item in doc-space
    QRectF dst = item->boundingBox().adjusted(-5,-5,5,5);
    dst.moveTo(dst.topLeft()
               + p_->document->pagePosition(item->docIndex().pageIdx()));

    if (!view.contains(dst))
    {
        QPointF d = dst.topLeft();
        if (d.x() >= view.left() && d.x() <= view.right() - dst.width())
            d.setX(view.x());
        if (d.y() >= view.top() && d.y() <= view.bottom() - dst.height())
            d.setY(view.y());
        moveToPoint(d);
        return true;
    }
    return false;
}

void ScoreView::refreshIndex(const Score::Index& idx, double m)
{
    if (isAssigned())
    if (auto item = p_->document->getScoreItem(idx))
    {
        QRectF dst = item->boundingBox();
        dst.moveTo(dst.topLeft()
               + p_->document->pagePosition(item->docIndex().pageIdx()));
        update(mapFromDocument(dst.adjusted(-m, -m, m, m)));
    }
}

void ScoreView::setPlayingIndex(const Score::Index& cur)
{
    if (cur == p_->playCursor)
        return;
    if (p_->playCursor.isValid())
        refreshIndex(p_->playCursor);
    p_->playCursor = cur;
    if (p_->playCursor.isValid())
        refreshIndex(p_->playCursor);
}

QRect ScoreView::mapFromDocument(const QRectF& r)
{
    return p_->matrix.mapRect(r).toRect();
}

QRectF ScoreView::mapToDocument(const QRect& r)
{
    return p_->imatrix.mapRect(r);
}



// ##################### EDIT ACTIONS #############################

QList<QAction*> ScoreView::createEditActions()
{
    auto par = this;
    QAction* a;
    QList<QAction*> list;

    list << (a = new QAction(tr("insert new part"), par));
    //a->setShortcut(Qt::Key_Enter);
    connect(a, &QAction::triggered, [=](){ editInsertStream(false); });

    list << (a = new QAction(tr("insert new part (after bar)"), par));
    a->setShortcut(Qt::ALT + Qt::Key_Enter);
    connect(a, &QAction::triggered, [=](){ editInsertStream(true); });

    list << (a = new QAction(tr("insert bar"), par));
    a->setShortcut(Qt::ALT + Qt::Key_B);
    connect(a, &QAction::triggered, [=](){ editInsertBar(false); });

    list << (a = new QAction(tr("insert row"), par));
    a->setShortcut(Qt::ALT + Qt::Key_R);
    connect(a, &QAction::triggered, [=](){ editInsertRow(false); });

    list << (a = new QAction(tr("insert note"), par));
    a->setShortcut(Qt::ALT + Qt::Key_N);
    connect(a, &QAction::triggered, [=](){ editInsertNote(); });

    list << (a = new QAction(tr("duplicate bar"), par));
    a->setShortcut(Qt::ALT + Qt::Key_D);
    connect(a, &QAction::triggered, [=](){ editDuplicateBar(); });

    list << (a = new QAction(tr("split part"), par));
    a->setStatusTip(tr("Splits the current part into two, "
                       "after the current bar"));
    //a->setShortcut(Qt::);
    connect(a, &QAction::triggered, [=](){ editSplitStream(); });

    list << (a = new QAction(tr("delete bar"), par));
    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_B);
    connect(a, &QAction::triggered, [=](){ editDeleteBar(); });

    list << (a = new QAction(tr("delete row"), par));
    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_R);
    connect(a, &QAction::triggered, [=](){ editDeleteRow(); });

    list << (a = new QAction(tr("delete note"), par));
    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_N);
    connect(a, &QAction::triggered, [=](){ editDeleteNote(); });

    return list;
}

void ScoreView::editInsertStream(bool after)
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;

    if (editor()->insertStream(
                p_->cursor, p_->cursor.getStream().createDefaultStream(),
                after))
    {
        if (after)
        {
            auto c = score()->index(p_->cursor.stream() + 1, 0,0,0);
            if (c.isValid())
                p_->setCursor(c, true);
        }
    }

}

void ScoreView::editInsertBar(bool after)
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;

    size_t len = p_->cursor.getStream().numNotes(p_->cursor.bar());
    if (editor()->insertBars(p_->cursor,
                             p_->cursor.getStream().createDefaultBarRows(len),
                             after))
    {
        auto c = p_->cursor.left();
        if (after)
            c.nextBar();
        if (c.isValid())
            p_->setCursor(c, true);
    }
}

void ScoreView::editDeleteBar()
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;

    if (p_->cursor.getStream().numBars() <= 1)
        return;

    auto c = p_->cursor;
    if (!c.nextBar())
        c.prevBar();

    if (editor()->deleteBar(p_->cursor)
            && !p_->cursor.isValid())
        p_->setCursor(c, true);
}

void ScoreView::editInsertRow(bool after)
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;

    editor()->insertRow(p_->cursor, after);
}

void ScoreView::editDuplicateBar()
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;

    auto rows = p_->cursor.getStream().getRows(p_->cursor.bar());
    if (editor()->insertBars(p_->cursor, rows, true))
    {
        auto c = p_->cursor.left();
        c.nextBar();
        if (c.isValid())
            p_->setCursor(c, true);
    }
}

void ScoreView::editDeleteRow()
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;

    if (p_->cursor.getStream().numRows() <= 1)
        return;

    auto c = p_->cursor;
    if (!c.nextRow())
        c.prevRow();
    if (editor()->deleteRow(p_->cursor)
            && !p_->cursor.isValid())
        p_->setCursor(c, true);
}

void ScoreView::editInsertNote(const Note& n)
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;

    editor()->insertNote(p_->cursor, n, true);
}

void ScoreView::editDeleteNote()
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;

    if (p_->cursor.getNotes().length() <= 1)
        return;

    auto c = p_->cursor;
    if (!c.nextNote())
        c.prevNote();

    if (editor()->deleteNote(p_->cursor, true)
            && !p_->cursor.isValid())
        p_->setCursor(c, true);
}

void ScoreView::editSplitStream()
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;
    editor()->splitStream(p_->cursor);
}

void ScoreView::editTransposeUp(int steps)
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;
    Note n = p_->cursor.getNote();
    if (n.isNote())
    {
        n.transpose(steps);
        editor()->changeNote(p_->cursor, n);
    }
}

void ScoreView::editTransposeDown(int steps)
{
    editTransposeUp(-steps);
}

void ScoreView::editAccidentialUp(int steps)
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;
    Note n = p_->cursor.getNote();
    if (n.isNote())
    {
        n.setAccidental(n.accidental() + steps);
        editor()->changeNote(p_->cursor, n);
    }
}

void ScoreView::editAccidentialDown(int steps)
{
    editAccidentialUp(-steps);
}

// ######################### EVENTS #########################

void ScoreView::Private::connectEditor(ScoreEditor* editor)
{
    connect(editor, &ScoreEditor::scoreReset, [=]()
    {
        setCursor(document->score()->index(0,0,0,0), false);
        setAction(cursor.isValid()
                      ? Private::A_ENTER_NOTE
                      : Private::A_NOTHING);
        p->goToPage(0);
    });
    connect(editor, &ScoreEditor::refresh, [=]()
    {
        p->update();
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
        p->refreshIndex(cursor);
    auto oldIdx = cursor;
    cursor = cur;
    if (ensureVisible)
    {
        if (cursor.isValid())
            if (!p->ensureIndexVisible(cursor))
                p->refreshIndex(cursor);
        updateStatus();
        emit p->currentIndexChanged(cursor, oldIdx);
        return;
    }
    if (cursor.isValid())
        p->refreshIndex(cursor);
    updateStatus();
    emit p->currentIndexChanged(cursor, oldIdx);
}

void ScoreView::Private::clearCursor()
{
    setCursor(Score::Index(), false);
}

void ScoreView::Private::updateStatus()
{
    QString s;
    if (!cursor.isValid())
        s = "[]";
    else
        s = QString("Part: %1, Bar(%2): %3, Row: %4, Note: %5 (%6)")
                .arg(cursor.stream())
                .arg(cursor.getNotes().length())
                .arg(cursor.bar())
                .arg(cursor.row())
                .arg(cursor.column())
                .arg(cursor.getNote().to3String());
    s += QString(" | octave %1")
            .arg(curOctave);
    emit p->statusChanged(s);
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
            isShift = e->modifiers() & Qt::SHIFT,
            isAlt = e->modifiers() & Qt::ALT;

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
                cursor = p_->document->goToPrevRow(cursor); break;
            case Qt::Key_Down:
                cursor = p_->document->goToNextRow(cursor); break;
            case Qt::Key_Home:
                cursor = p_->document->goToStart(cursor); break;
            case Qt::Key_End:
                cursor = p_->document->goToEnd(cursor); break;
            default: handled = false;
        }
        if (handled)
        {
            if (cursor.isValid() && cursor != p_->cursor)
            {
                p_->setCursor(cursor, true);
            }
            return;
        }

        // INSERT/DELETE / CHANGE
        handled = true;
        switch (e->key())
        {
            case Qt::Key_Insert:
            case '.':
                editInsertNote();
            break;
            case Qt::Key_Delete:
                editDeleteNote();
            break;
            /** @todo there is some duplicate functionality
                in keyEvent() and createEditActions() */
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (isAlt)
                    editInsertStream(true);
                else
                    editInsertBar(true);
            break;

            case '+':
            case '-':
                if (e->key() == '+')
                    editAccidentialUp();
                else
                    editAccidentialDown();
                if (p_->cursor.isValid() && p_->cursor.getNote().isNote())
                {
                    emit noteEntered(p_->cursor.getNote());
                    p_->curOctave = p_->cursor.getNote().octaveSpanish();
                    p_->updateStatus();
                }
            break;

            case '>':
                p_->curOctave++;
                p_->updateStatus();
            break;
            case '<':
                p_->curOctave--;
                p_->updateStatus();
            break;

            default: handled = false;
        }
        if (handled)
        {
            return;
        }


        // ENTER NOTES
        handled = true;
        QString noteStr;
        KeySignature keySig = p_->cursor.getStream().keySignature();
        switch (e->key())
        {
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_A:
            case Qt::Key_B:
            case Qt::Key_C:
            case Qt::Key_D:
            case Qt::Key_E:
            case Qt::Key_F:
            case Qt::Key_G:
            case Qt::Key_H:
                noteStr = QChar(e->key());
                noteStr += QString("'").repeated(p_->curOctave-3);
                noteStr += QString(",").repeated(3-p_->curOctave);
            break;
            case Qt::Key_P:
                noteStr = QChar(e->key());
            break;

            case Qt::Key_Space:
                noteStr = " ";
            break;

            default: handled = false;
        }

        if (!handled)
        {
            e->ignore();
            return;
        }

        Note note = Note::fromString(noteStr);
        if (!note.isValid())
            return;

        Note old = p_->cursor.getNote();
        if (note != old)
        {
            editor()->changeNote(p_->cursor, note);
            p_->updateStatus();
        }

        note = keySig.transform(note);
        emit noteEntered(note);

        return;
    }

    /** @todo go-to-page-keys are ambigious now */
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

    p_->setAction( p_->cursor.isValid()
            ? Private::A_ENTER_NOTE
            : Private::A_NOTHING );
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

    for (size_t page=0; page < p_->document->numPages(); ++page)
        p_->paintPage(&p, docUpdate, page);

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
    // dont paint this page
    {
        QRectF pr = document->pageRect(pageIndex);
        if (!pr.intersects(updateRect))
            return;
    }

    auto pageLayout = document->pageLayout(pageIndex);
    auto pagePos = document->pagePosition(pageIndex);

    // page-local matrix
    auto mat = matrix;
    mat.translate(pagePos.x(), pagePos.y());

    p->save();
    p->setTransform(mat);

    // page-local update rect
    QRectF uRect = updateRect;
    uRect.moveTo(updateRect.x()-pagePos.x(),
                 updateRect.y()-pagePos.y());

    // page packground
    //QBrush b(QColor(rand()&0xff,rand()&0xff,rand()&0xff));
    p->fillRect((pageLayout.pageRect() & uRect.adjusted(-1,-1,1,1)),
                brushPageBackground);

    // content borders / margins
    if (doShowLayoutBoxes())
    {
        p->setBrush(Qt::NoBrush);
        p->setPen(penLayoutFrame);
        p->drawRect(pageLayout.contentRect());
        p->drawRect(pageLayout.scoreRect());
    }

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
    auto srect = pageLayout.scoreRect();
    if (!srect.intersects(updateRect))
        return;

    //p->setBrush(QBrush(QColor(rand()&0xff,rand()&0xff,rand()&0xff)));
    //p->drawRect(updateRect);

    p->setPen(penScoreItem);
    document->paintScoreItems(*p, pageIndex, updateRect);

    // play cursor
    if (playCursor.isValid())
    if (auto item = document->getScoreItem(playCursor))
    if (item->docIndex().pageIdx() == pageIndex)
    {
        p->setPen(penPlayCursor);
        p->setBrush(Qt::NoBrush);
        p->drawRect(item->boundingBox());
    }

    // cursor
    if (cursor.isValid())
    if (auto item = document->getScoreItem(cursor))
    if (item->docIndex().pageIdx() == pageIndex)
    {
        p->setPen(penCursor);
        p->setBrush(Qt::NoBrush);
        p->drawRect(item->boundingBox());
    }
}


} // namespace Sonot
