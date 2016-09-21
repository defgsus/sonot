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

#include <QMimeData>

#include "QProps/error.h"
#include "QProps/JsonInterfaceHelper.h"

#include "ScoreEditor.h"
#include "Notes.h"
#include "NoteStream.h"

namespace Sonot {

struct ScoreEditor::Private
{
    Private(ScoreEditor* p)
        : p         (p)
        , score_    (nullptr)
    { }

    Score* score() const { return score_; }

    /** Returns writeable NoteStream */
    NoteStream* getStream(const Score::Index& i);
    /** Returns writeable Bar */
    Bar* getBar(const Score::Index& i);
    /** Returns writeable Notes */
    Notes* getNotes(const Score::Index& i);

    ScoreEditor* p;

    Score* score_;
};

ScoreEditor::ScoreEditor(QObject *parent)
    : QObject   (parent)
    , p_        (new Private(this))
{

}

ScoreEditor::~ScoreEditor()
{
    delete p_;
}

Score* ScoreEditor::score() const { return p_->score_; }




// ##################### editing ###########################

#define SONOT__CHECK_INDEX(i__, ret__) \
    QPROPS_ASSERT(i__.score() == score(), \
     "non-matching index" << i__.toString() << "used with ScoreEditor"); \
    QPROPS_ASSERT(i__.isValid(), \
     "invalid index" << i__.toString() << "used with ScoreEditor"); \
    if (!i__.isValid() || i__.score() != score()) return ret__;

#define SONOT__CHECK_SELECTION(i__, ret__) \
    SONOT__CHECK_INDEX(i__.from(), ret__); \
    SONOT__CHECK_INDEX(i__.to(), ret__);


void ScoreEditor::setScore(const Score& s)
{
    delete p_->score_;
    p_->score_ = new Score(s);
    emit scoreReset(p_->score_);
}

void ScoreEditor::setStreamProperties(
        size_t streamIdx, const QProps::Properties& p)
{
    if (!score())
        return;
    auto idx = score()->index(streamIdx, 0,0,0);
    if (NoteStream* stream = p_->getStream(idx))
    {
        stream->setProperties(p);
        emit streamsChanged(IndexList() << idx);
        emit documentChanged();
    }
}

bool ScoreEditor::insertNote(
        const Score::Index& idx, const Note& n, bool allRows)
{
    SONOT__CHECK_INDEX(idx, false);
    if (!allRows)
    {
        if (Notes* bar = p_->getNotes(idx))
        {
            bar->insertNote(idx.column(), n);
            emit barsChanged(IndexList() << idx);
            emit documentChanged();
            return true;
        }
    }
    else
    {
        bool changed = false;
        NoteStream* s = p_->getStream(idx);
        for (size_t i=0; i<s->numRows(); ++i)
        {
            auto x = score()->index(idx.stream(),
                                    idx.bar(),
                                    i,
                                    idx.column());
            x = x.limitRight();
            if (Notes* bar = p_->getNotes(x))
            {
                bar->insertNote(idx.column(), n);
                changed = true;
            }
        }
        if (changed)
        {
            emit barsChanged(IndexList() << idx);
            emit documentChanged();
            return true;
        }
    }
    return false;
}

bool ScoreEditor::insertBar(
        const Score::Index& idx, const Bar& bar, bool after)
{
    SONOT__CHECK_INDEX(idx, false);
    if (NoteStream* stream = p_->getStream(idx))
    {
        stream->insertBar(idx.bar() + (after ? 1 : 0), bar);
        emit streamsChanged(IndexList() << idx);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::insertBars(
        const Score::Index& idx, const NoteStream& stream, bool after)
{
    SONOT__CHECK_INDEX(idx, false);
    if (NoteStream* dst = p_->getStream(idx))
    {
        size_t iidx = idx.bar() + (after ? 1 : 0);
        for (size_t b = 0; b < stream.numBars(); ++b)
        {
            dst->insertBar(iidx, stream.bar(b));
            iidx++;
        }
        emit streamsChanged(IndexList() << idx);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::insertRow(const Score::Index& idx, bool after)
{
    SONOT__CHECK_INDEX(idx, false);
    if (NoteStream* stream = p_->getStream(idx))
    {
        stream->insertRow(idx.row() + (after ? 1 : 0));
        emit streamsChanged(IndexList() << idx);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::insertStream(
        const Score::Index& idx, const NoteStream& s, bool after)
{
    SONOT__CHECK_INDEX(idx, false);
    if (idx.stream() < score()->numNoteStreams())
    {
        size_t iidx = idx.stream() + (after ? 1 : 0);
        score()->insertNoteStream(iidx, s);

        IndexList list;
        for (size_t i = iidx; i < score()->numNoteStreams(); ++i)
            list << score()->index(i, 0,0,0);
        emit streamsChanged(list);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::insertScore(
        const Score::Index& idx, const Score& s, bool after)
{
    SONOT__CHECK_INDEX(idx, false);
    if (idx.stream() < score()->numNoteStreams())
    {
        size_t iidx = idx.stream() + (after ? 1 : 0);
        for (size_t i = 0; i<s.numNoteStreams(); ++i)
        {
            score()->insertNoteStream(iidx + i, s.noteStream(i));
        }

        IndexList list;
        for (size_t i = iidx; i < score()->numNoteStreams(); ++i)
            list << score()->index(i, 0,0,0);
        emit streamsChanged(list);
        emit documentChanged();
        return true;
    }
    return false;
}


bool ScoreEditor::deleteRow(const Score::Index& idx)
{
    SONOT__CHECK_INDEX(idx, false);
    if (NoteStream* stream = p_->getStream(idx))
    {
        stream->removeRow(idx.row());
        emit streamsChanged(IndexList() << idx);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::changeNote(const Score::Index& idx, const Note& n)
{
    SONOT__CHECK_INDEX(idx, false);
    if (Notes* notes = p_->getNotes(idx))
    {
        notes->setNote(idx.column(), n);
        emit noteValuesChanged(IndexList() << idx);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::changeBar(const Score::Index& idx, const Notes& b)
{
    SONOT__CHECK_INDEX(idx, false);
    if (Notes* bar = p_->getNotes(idx))
    {
        *bar = b;
        emit barsChanged(IndexList() << idx);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::transpose(const Score::Selection& sel, int steps)
{
    SONOT__CHECK_SELECTION(sel, false);
    if (steps == 0)
        return false;
    auto indices = sel.containedIndices();
    if (indices.isEmpty())
        return false;
    for (Score::Index& idx : indices)
    {
        idx.setNote( idx.getNote().transposed(steps) );
    }
    emit noteValuesChanged(indices);
    emit documentChanged();
    return true;
}



bool ScoreEditor::deleteNote(const Score::Index& idx, bool allRows)
{
    SONOT__CHECK_INDEX(idx, false);
    if (Notes* bar = p_->getNotes(idx))
    {
        IndexList list;
        if (!allRows)
        {
            list << idx;
            emit notesAboutToBeDeleted(list);
            emit documentChanged();
            bar->removeNote(idx.column());
        }
        else
        {
            for (size_t i=0; i<idx.getStream().numRows(); ++i)
            {
                auto x = score()->index(idx.stream(), idx.bar(),
                                        i, idx.column());
                if (p_->getNotes(x))
                    list << x;
            }
            emit notesAboutToBeDeleted(list);
            for (auto& x : list)
            if (Notes* b = p_->getNotes(x))
            {
                b->removeNote(x.column());
            }
        }

        emit notesDeleted(list);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::deleteBar(const Score::Index& idx)
{
    SONOT__CHECK_INDEX(idx, false);
    if (NoteStream* stream = p_->getStream(idx))
    {
        IndexList list; list << idx;
        emit barsAboutToBeDeleted(list);
        stream->removeBar(idx.bar());
        emit barsDeleted(list);
        emit streamsChanged(list);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::deleteStream(const Score::Index& idx)
{
    SONOT__CHECK_INDEX(idx, false);
    if (idx.stream() < score()->numNoteStreams())
    {
        IndexList list; list << idx;
        emit streamsAboutToBeDeleted(list);
        score()->removeNoteStream(idx.stream());
        emit streamsDeleted(list);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::splitStream(const Score::Index& idx)
{
    SONOT__CHECK_INDEX(idx, false);
    if (idx.isStreamRight())
        return false;
    NoteStream* org = p_->getStream(idx);
    if (!org)
        return false;
    NoteStream cpy = *org;
    cpy.removeBars(0, idx.bar() + 1);
    org->removeBars(idx.bar() + 1);
    score()->insertNoteStream(idx.stream() + 1, cpy);
    // get propper indices after change
    auto i1 = score()->index(idx.stream(), 0,0,0),
         i2 = score()->index(idx.stream()+1, 0,0,0);
    QPROPS_ASSERT(i1.isValid(), "");
    QPROPS_ASSERT(i2.isValid(), "");
    emit streamsChanged(IndexList() << i1 << i2);
    emit documentChanged();
    return true;
}

NoteStream* ScoreEditor::Private::getStream(const Score::Index& idx)
{
    SONOT__CHECK_INDEX(idx, nullptr);
    return const_cast<NoteStream*>(&score_->noteStreams()[idx.stream()]);
}

Bar* ScoreEditor::Private::getBar(const Score::Index& idx)
{
    SONOT__CHECK_INDEX(idx, nullptr);
    return const_cast<Bar*>(&score_->noteStreams()[idx.stream()]
                            .bar(idx.bar()));
}

Notes* ScoreEditor::Private::getNotes(const Score::Index& idx)
{
    SONOT__CHECK_INDEX(idx, nullptr);
    return const_cast<Notes*>(&score_->noteStreams()[idx.stream()]
                            .notes(idx.bar(), idx.row()));
}

bool ScoreEditor::pasteMimeData(const Score::Index& idx,
                                const QMimeData* data, bool after)
{
    if (!data->hasText())
        return false;
    auto doc = QJsonDocument::fromJson(data->data("text/plain"));
    if (doc.isNull())
        return false;

    QProps::JsonInterfaceHelper json("ScoreEditor");

    QJsonObject o = doc.object();

    if (o.contains("bar"))
    {
        Bar bar;
        bar.fromJson( json.expectChildObject(o, "bar") );

        int startRow = o.value("row-start").toInt(-1),
            endRow = o.value("row-end").toInt(-1),
            startCol = o.value("col-start").toInt(-1),
            endCol = o.value("col-end").toInt(-1);
        QPROPS_ASSERT(startRow < 0 || endRow >= 0, "range not fully set");
        QPROPS_ASSERT(startCol < 0 || endCol >= 0, "range not fully set");
        qDebug() << startRow << endRow << startCol << endCol;
        if (startRow >= 0 && endRow >= 0
         && startCol >= 0 && endCol >= 0)
        {
            QPROPS_ASSERT_LT((size_t)startRow, bar.numRows(), "");
            QPROPS_ASSERT_LT((size_t)endRow, bar.numRows(), "");
            Bar* dst = p_->getBar(idx);
            if (!dst)
                return false;
            int maRow = -1, maCol = -1;
            for (int r=startRow; r<=endRow; ++r)
            {
                int row = idx.row() + r - startRow;
                if (row >= 0 && (size_t)row < dst->numRows())
                {
                    QPROPS_ASSERT_LT((size_t)startCol, bar[r].length(), "");
                    QPROPS_ASSERT_LT((size_t)endCol, bar[r].length(), "");
                    maRow = std::max(maRow, row);
                    Notes n = (*dst)[row];
                    for (int c=startCol; c<=endCol; ++c)
                    {
                        int col = idx.column() + c - startCol;
                        if (col >= 0 && (size_t)col < n.length())
                        {
                            maCol = std::max(maCol, col);
                            n.setNote(col, bar[r].note(c));
                        }
                    }
                    dst->setNotes(row, n);
                }
            }
            if (maRow >= 0 && maCol >= 0)
            {
                emit barsChanged(IndexList() << idx);
                emit documentChanged();
                emit pasted(Score::Selection(
                                idx, score()->index(idx.stream(),
                                                    idx.bar(), maRow, maCol)));
                return true;
            }
            return false;
        }
        else
        {
            if (insertBar(idx, bar, after))
            {
                auto k = idx;
                if (after)
                    k.nextBar();
                if (k.isValid())
                {
                    auto i1 = Score::Selection::fromBar(k);
                    if (i1.isValid())
                        emit pasted(i1);
                }
                return true;
            }
            return false;
        }

    }
    else if (o.contains("bars"))
    {
        NoteStream s;
        s.fromJson( json.expectChildObject(o, "bars") );
        if (insertBars(idx, s, after))
        {
            auto i1 = idx.streamTopLeft();
            if (after)
                i1.nextStream();
            auto s1 = Score::Selection::fromStream(i1);
            if (s1.isValid())
                emit pasted(s1);
            return true;
        }
        return false;
    }
    else if (o.contains("stream"))
    {
        NoteStream s;
        s.fromJson( json.expectChildObject(o, "stream") );
        return insertStream(idx, s, after);
    }
    else if (o.contains("score"))
    {
        Score s;
        s.fromJson( json.expectChildObject(o, "score") );
        return insertScore(idx, s, after);
    }

    return false;
}


} // namespace Sonot
