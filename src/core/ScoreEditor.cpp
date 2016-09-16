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
#include "Bar.h"
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

bool ScoreEditor::insertNote(const Score::Index& idx, const Note& n)
{
    SONOT__CHECK_INDEX(idx, false);
    if (Bar* bar = p_->getBar(idx))
    {
        bar->insertNote(idx.column(), n);
        emit barsChanged(IndexList() << idx);
        return true;
    }
    return false;
}

bool ScoreEditor::insertBars(
        const Score::Index& idx, const QList<Bar>& rows, bool after)
{
    SONOT__CHECK_INDEX(idx, false);
    if (NoteStream* stream = p_->getStream(idx))
    {
        //size_t rows = stream->numRows();
        stream->insertBar(idx.bar() + (after ? 1 : 0), rows);
        //if (stream->numRows() != rows)
        emit streamsChanged(IndexList() << idx);
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
        return true;
    }
    return false;
}

bool ScoreEditor::changeNote(const Score::Index& idx, const Note& n)
{
    SONOT__CHECK_INDEX(idx, false);
    if (Bar* bar = p_->getBar(idx))
    {
        bar->setNote(idx.column(), n);
        emit noteValuesChanged(IndexList() << idx);
        return true;
    }
    return false;
}

bool ScoreEditor::changeBar(const Score::Index& idx, const Bar& b)
{
    SONOT__CHECK_INDEX(idx, false);
    if (Bar* bar = p_->getBar(idx))
    {
        *bar = b;
        emit barsChanged(IndexList() << idx);
        return true;
    }
    return false;
}

bool ScoreEditor::deleteNote(const Score::Index& idx)
{
    SONOT__CHECK_INDEX(idx, false);
    if (Bar* bar = p_->getBar(idx))
    {
        IndexList list; list << idx;
        emit notesAboutToBeDeleted(list);
        bar->removeNote(idx.column());
        emit notesDeleted(list);
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
        return true;
    }
    return false;
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
                            .bar(idx.bar(), idx.row()));
}


} // namespace Sonot
