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
#include <QClipboard>
#include <QApplication>
#include <QMenu>

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
#include "core/SelectionMimeData.h"

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
        , brushSelection        (QColor(0,100,240, 40))
        , penLayoutFrame        (QColor(0,50,50,30))
        , penScoreRow           (QColor(0,0,0,50))
        , penScoreItem          (QColor(0,0,0))
        , penCursor             (QColor(0,60,100,100))
        , penPlayCursor         (QColor(0,160,0,100))
    {
        penScoreItem.setWidthF(.5);
        penLayoutFrame.setStyle(Qt::DotLine);
    }

    // --- gui / state ---

    void connectEditor(ScoreEditor*);
    void onMatrixChanged();
    void setAction(Action);
    void setCursor(const Score::Index& cursor,
                   bool ensureVisible, bool extendSelection);
    void setSelection(const Score::Selection&);
    void clearCursor();
    void updateStatus();

    void setCurOctave(int row, int oct);
    int  getCurOctave(int row);

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

    Score::Index cursor, playCursor, selStart;
    Score::Selection curSelection;
    std::vector<int> curOctave;

    // -- config --

    QBrush  brushBackground,
            brushPageBackground,
            brushSelection;
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

bool ScoreView::isSelection() const { return p_->curSelection.isValid(); }
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

void ScoreView::setCurrentIndex(const Score::Index& idx)
{
    p_->setCursor(idx, true, false);
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

void ScoreView::createEditActions(QMenu* menu)
{
    QAction* a;
    a = menu->addAction(tr("insert new part"));
    //a->setShortcut(Qt::Key_Enter);
    connect(a, &QAction::triggered, [=](){ editInsertStream(false); });

    a = menu->addAction(tr("insert new part (after bar)"));
    a->setShortcut(Qt::ALT + Qt::Key_Enter);
    connect(a, &QAction::triggered, [=](){ editInsertStream(true); });

    a = menu->addAction(tr("insert bar"));
    a->setShortcut(Qt::ALT + Qt::Key_B);
    connect(a, &QAction::triggered, [=](){ editInsertBar(false); });

    a = menu->addAction(tr("insert row"));
    a->setShortcut(Qt::ALT + Qt::Key_R);
    connect(a, &QAction::triggered, [=](){ editInsertRow(false); });

    a = menu->addAction(tr("insert note"));
    a->setShortcut(Qt::ALT + Qt::Key_N);
    connect(a, &QAction::triggered, [=](){ editInsertNote(); });

    a = menu->addAction(tr("duplicate bar"));
    a->setShortcut(Qt::ALT + Qt::Key_D);
    connect(a, &QAction::triggered, [=](){ editDuplicateBar(); });

    a = menu->addAction(tr("split part"));
    a->setStatusTip(tr("Splits the current part into two, "
                       "after the current bar"));
    //a->setShortcut(Qt::);
    connect(a, &QAction::triggered, [=](){ editSplitStream(); });

    menu->addSeparator();

    a = menu->addAction(tr("select row/column/bar/stream"));
    a->setShortcut(Qt::ALT + Qt::Key_S);
    connect(a, &QAction::triggered, [=](){ editSelectNext(); });

    a = menu->addAction(tr("unselect"));
    a->setShortcut(Qt::ALT + Qt::Key_U);
    connect(a, &QAction::triggered, [=](){ editSelectNone(); });

    a = menu->addAction(tr("copy selection"));
    a->setShortcut(Qt::CTRL + Qt::Key_C);
    connect(a, &QAction::triggered, [=](){ editCopySel(); });

    a = menu->addAction(tr("paste selection"));
    a->setShortcut(Qt::CTRL + Qt::Key_V);
    connect(a, &QAction::triggered, [=](){ editPaste(); });

    menu->addSeparator();

    a = menu->addAction(tr("delete part"));
    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_P);
    connect(a, &QAction::triggered, [=](){ editDeleteStream(); });

    a = menu->addAction(tr("delete bar"));
    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_B);
    connect(a, &QAction::triggered, [=](){ editDeleteBar(); });

    a = menu->addAction(tr("delete row"));
    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_R);
    connect(a, &QAction::triggered, [=](){ editDeleteRow(); });

    a = menu->addAction(tr("delete note"));
    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_N);
    connect(a, &QAction::triggered, [=](){ editDeleteNote(); });

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
                p_->setCursor(c, true, false);
        }
    }

}

void ScoreView::editInsertBar(bool after)
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;

    size_t len = p_->cursor.getStream().numNotes(p_->cursor.bar());
    if (editor()->insertBar(p_->cursor,
                            p_->cursor.getStream().createDefaultBar(len),
                            after))
    {
        auto c = p_->cursor.left();
        if (after)
            c.nextBar();
        if (c.isValid())
            p_->setCursor(c, true, false);
    }
}

void ScoreView::editDeleteStream()
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;

    if (score()->numNoteStreams() <= 1)
        return;

    auto c = p_->cursor;
    if (!c.nextStream())
        c.prevStream();

    if (editor()->deleteStream(p_->cursor)
            && !p_->cursor.isValid())
        p_->setCursor(c, true, false);
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
        p_->setCursor(c, true, false);
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

    auto& bar = p_->cursor.getBar();
    if (editor()->insertBar(p_->cursor, bar, true))
    {
        auto c = p_->cursor.left();
        c.nextBar();
        if (c.isValid())
            p_->setCursor(c, true, false);
    }
}

void ScoreView::editDeleteRow()
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;

    if (p_->cursor.getStream().numRows() <= 1)
        return;

    auto c = p_->cursor;
    if (!c.prevRow())
        c.nextRow();
    if (editor()->deleteRow(p_->cursor)
            && !p_->cursor.isValid())
        p_->setCursor(c, true, false);
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
    if (!c.prevNote())
        c.nextNote();

    if (editor()->deleteNote(p_->cursor, true)
            && !p_->cursor.isValid())
        p_->setCursor(c, true, false);
}

void ScoreView::editSplitStream()
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;
    editor()->splitStream(p_->cursor);
}

void ScoreView::editTransposeUp(int steps, bool whole)
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;
    if (!isSelection())
    {
        Note n = p_->cursor.getNote();
        if (n.isNote())
        {
            n.transpose(steps, whole);
            editor()->changeNote(p_->cursor, n);
        }
    }
    else
    {
        editor()->transpose(p_->curSelection, steps, whole);
    }
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

void ScoreView::editOctaveUp(int steps)
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;
    Note n = p_->cursor.getNote();
    if (n.isNote())
    {
        n.setOctave(n.octave() + steps);
        editor()->changeNote(p_->cursor, n);
        p_->setCurOctave(p_->cursor.row(), n.octave());
    }
    else
        p_->setCurOctave(p_->cursor.row(),
                         p_->getCurOctave(p_->cursor.row()) + steps);

}

void ScoreView::Private::setCurOctave(int row, int oct)
{
    if (row < 0)
        return;
    while ((size_t)row >= curOctave.size())
        curOctave.push_back(3);
    curOctave[row] = oct;
    updateStatus();
}

int ScoreView::Private::getCurOctave(int row)
{
    if (row < 0 || (size_t)row >= curOctave.size())
        return 3;
    return curOctave[row];
}

void ScoreView::editSelectNone()
{
    if (!isAssigned())
        return;
    p_->setSelection(Score::Selection());
}

void ScoreView::editSelectNext()
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;
    if (!p_->curSelection.contains(p_->cursor) )
    {
        p_->setSelection(Score::Selection::fromColumn(p_->cursor));
    }
    else
    {
        if (p_->curSelection.isSingleColumn())
        {
            p_->setSelection(Score::Selection::fromNotes(p_->cursor));
        }
        else
        {
            auto s = Score::Selection::fromBars(p_->curSelection);
            if (s != p_->curSelection)
            {
                p_->setSelection(s);
                return;
            }
            s = Score::Selection::fromStream(p_->cursor);
            if (s == p_->curSelection)
                s = Score::Selection();
            p_->setSelection(s);
        }
    }
}

void ScoreView::editCopySel()
{
    if (!isAssigned() || !p_->curSelection.isValid())
        return;

    auto data = SelectionMimeData::fromSelection(p_->curSelection);
    qApp->clipboard()->setMimeData(data);
}

void ScoreView::editPaste(bool after)
{
    if (!isAssigned() || !p_->cursor.isValid())
        return;
    editor()->pasteMimeData(p_->cursor,
                            qApp->clipboard()->mimeData(),
                            after);
}

// ######################### EVENTS #########################

void ScoreView::Private::connectEditor(ScoreEditor* editor)
{
    connect(editor, &ScoreEditor::scoreReset, [=]()
    {
        curOctave.clear();
        setCursor(document->score()->index(0,0,0,0), false, false);
        setSelection(Score::Selection());
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
    connect(editor, &ScoreEditor::pasted, [=](const Score::Selection& sel)
    {
        setCursor(sel.from(), true, false);
        setSelection(sel);
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

void ScoreView::Private::setCursor(
        const Score::Index& cur, bool ensureVisible, bool extendSelection)
{
    if (cur == cursor)
        return;
    if (cursor.isValid())
        p->refreshIndex(cursor);
    auto oldCursor = cursor;
    cursor = cur;
    if (ensureVisible)
    {
        if (cursor.isValid())
            if (!p->ensureIndexVisible(cursor))
                p->refreshIndex(cursor);
    }
    else
        if (cursor.isValid())
            p->refreshIndex(cursor);

    updateStatus();
    emit p->currentIndexChanged(cursor, oldCursor);

    if (cursor.isValid() && extendSelection)
    {
        if (!selStart.isValid())
        {
            // adjust selection after release
            if (curSelection.contains(oldCursor))
            {
                selStart = cursor.farthestManhatten(
                            curSelection.from(),
                            curSelection.to());
                setSelection(Score::Selection(selStart, cursor));
            }
            else
            {
                // start new selection
                setSelection(Score::Selection(oldCursor, cursor));
                selStart = curSelection.from();
            }
        }
        else
        {
            // adjust selection without release
            setSelection(Score::Selection(selStart, cursor));
        }
    }
}

void ScoreView::Private::setSelection(const Score::Selection& s)
{
    curSelection = s;
    if (!s.isValid())
        selStart = Score::Index();
    /** @todo refine refresh window for Score::Selection */
    p->update();
    /*
    QString st;
    for (int r = 0; r < 4; ++r)
    {
        for (int b = 0; b < 4; ++b)
        {
            st += "|";
            for (int c = 0; c < 8; ++c)
                st += curSelection.contains(p->score()->index(0,b,r,c))
                    ? "*" : ".";
        }
        st += "\n";
    }
    qDebug().noquote() << st;
    */
}

void ScoreView::Private::clearCursor()
{
    setCursor(Score::Index(), false, false);
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
    s += QString(" | current octave %1")
            .arg(getCurOctave(cursor.row()));
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
                cursor.prevNote();
            break;
            case Qt::Key_Right:
                cursor.nextNote();
            break;
            //case Qt::Key_Tab: cursor.nextBar(); break;
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
                if (!isShift)
                    p_->selStart = Score::Index();
                p_->setCursor(cursor, true, isShift);
            }
            return;
        }

        // INSERT/DELETE / CHANGE

        KeySignature keySig = p_->cursor.getStream().keySignature();

        handled = true;
        bool doSendNote = false;
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

            case '-':
            case '+':
                if (e->key() == '+')
                    editTransposeUp();
                else
                    editTransposeDown();
                doSendNote = true;
            break;

            case '8':
            case '9':
                if (e->key() == '9')
                    editAccidentialUp();
                else
                    editAccidentialDown();
                doSendNote = true;
            break;

            case '>':
                editOctaveUp();
                doSendNote = true;
            break;
            case '<':
                editOctaveDown();
                doSendNote = true;
            break;

            default: handled = false;
        }
        if (handled)
        {
            if (doSendNote)
            {
                if (!isSelection() && p_->cursor.isValid()
                        && p_->cursor.getNote().isNote())
                {
                    emit noteEntered(keySig.transform(p_->cursor.getNote()));
                    //p_->curOctave = p_->cursor.getNote().octaveSpanish();
                    p_->updateStatus();
                }
            }
            return;
        }


        // ENTER NOTES
        handled = true;
        QString noteStr;
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
            {
                int oct = p_->getCurOctave(p_->cursor.row());
                noteStr = QChar(e->key());
                noteStr += QString("'").repeated(oct-3);
                noteStr += QString(",").repeated(3-oct);
            }
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
    /*if (e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9)
    {
        if (e->modifiers() != 0)
            showPage(e->key() - Qt::Key_0);
        else
            goToPage(e->key() - Qt::Key_0);
        return;
    }*/

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
            p_->setCursor(idx, false, e->modifiers() & Qt::SHIFT);
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

    if (curSelection.isValid())
    {
        QList<QRectF> rects = document->getSelectionRects(
                    pageIndex, curSelection);
        for (const auto& r : rects)
            p->fillRect(r, brushSelection);
        /*
        auto i1 = document->getScoreItem(curSelection.from()),
             i2 = document->getScoreItem(curSelection.to());
        if (i1 && i2)
        {
            QRectF r = i1->boundingBox() | i2->boundingBox();
            p->fillRect(r, brushSelection);
        }
        */
    }
}


} // namespace Sonot
