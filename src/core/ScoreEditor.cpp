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

#include "QProps/error.h"

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
    Notes* getBar(const Score::Index& i);

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
        if (Notes* bar = p_->getBar(idx))
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
            if (Notes* bar = p_->getBar(x))
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

bool ScoreEditor::insertBars(
        const Score::Index& idx, const QList<Notes>& rows, bool after)
{
    SONOT__CHECK_INDEX(idx, false);
    if (NoteStream* stream = p_->getStream(idx))
    {
        //size_t rows = stream->numRows();
        stream->insertBar(idx.bar() + (after ? 1 : 0), rows);
        //if (stream->numRows() != rows)
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
    if (Notes* bar = p_->getBar(idx))
    {
        bar->setNote(idx.column(), n);
        emit noteValuesChanged(IndexList() << idx);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::changeBar(const Score::Index& idx, const Notes& b)
{
    SONOT__CHECK_INDEX(idx, false);
    if (Notes* bar = p_->getBar(idx))
    {
        *bar = b;
        emit barsChanged(IndexList() << idx);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::deleteNote(const Score::Index& idx, bool allRows)
{
    SONOT__CHECK_INDEX(idx, false);
    if (Notes* bar = p_->getBar(idx))
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
                if (p_->getBar(x))
                    list << x;
            }
            emit notesAboutToBeDeleted(list);
            for (auto& x : list)
            if (Notes* b = p_->getBar(x))
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
    if (idx.isStreamEnd())
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

Notes* ScoreEditor::Private::getBar(const Score::Index& idx)
{
    SONOT__CHECK_INDEX(idx, nullptr);
    return const_cast<Notes*>(&score_->noteStreams()[idx.stream()]
                            .notes(idx.bar(), idx.row()));
}


} // namespace Sonot
