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

#include <QList>
#include <QMap>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>

#include "Score.h"
#include "NoteStream.h"
#include "Bar.h"
#include "QProps/JsonInterfaceHelper.h"
#include "QProps/error.h"

namespace Sonot {

struct Score::Private
{
    Private(Score* p)
        : p         (p)
        , props     ("score")
    {
        props.set("title", tr("title"),
                  tr("Title of the collection"), QString());
        props.set("author", tr("author"),
                  tr("Author of the collection"), QString());
        props.set("poet", tr("poet"),
                  tr("Author of the lyrics"), QString());
        props.set("copyright", tr("copyright"),
                  tr("Copyleft/right information"), QString());
        props.set("transcriber", tr("transcriber"),
                  tr("The one who did the typing"),
                  QString());
        props.set("source", tr("source"),
                  tr("Description of the source"),
                  QString());
    }

    Score* p;

    QList<NoteStream> streams;
    QProps::Properties props;
};

Score::Score()
    : p_        (new Private(this))
{

}

Score::Score(const Score &o)
    : Score()
{
    *this = o;
}

Score::~Score()
{
    delete p_;
}

Score& Score::operator = (const Score& o)
{
    p_->streams = o.p_->streams;
    p_->props = o.p_->props;
    return *this;
}

bool Score::operator == (const Score& rhs) const
{
    return p_->streams == rhs.p_->streams
        && p_->props == rhs.p_->props;
}

const QProps::Properties& Score::props() const { return p_->props;}
QProps::Properties& Score::propsw() { return p_->props;}

QString Score::toInfoString() const
{
    QString s = "Score(";
    for (int i=0; i<p_->streams.size(); ++i)
    {
        if (i != 0)
            s += ", ";
        s += QString("%1x%2")
                .arg(p_->streams[i].numBars())
                .arg(p_->streams[i].numRows());
    }
    return s + ")";
}

void Score::setProperties(const QProps::Properties& p)
{
    p_->props = p;
}

QJsonObject Score::toJson() const
{
    QJsonArray jstreams;
    for (const NoteStream& s : p_->streams)
        jstreams.append(QJsonValue(s.toJson()));

    QJsonObject o;
    if (!jstreams.isEmpty())
        o.insert("streams", jstreams);

    o.insert("props", p_->props.toJson());

    return o;
}

void Score::fromJson(const QJsonObject& o)
{
    QList<NoteStream> streams;
    QProps::Properties props(p_->props);

    QProps::JsonInterfaceHelper json("Score");

    if (o.contains("streams"))
    {
        auto jstreams = json.expectArray(json.expectChildValue(o, "streams"));
        for (int i=0; i<jstreams.size(); ++i)
        {
            NoteStream s;
            s.fromJson(json.expectObject(jstreams.at(i)));
            streams.append( s );
        }
    }

    if (o.contains("props"))
    {
        props.fromJson(json.expectChildObject(o, "props"));
    }

    p_->streams.swap(streams);
    p_->props.swap(props);
}


size_t Score::numNoteStreams() const
{
    return p_->streams.size();
}

const NoteStream& Score::noteStream(size_t idx) const
{
    QPROPS_ASSERT_LT(idx, size_t(p_->streams.size()), "in Score::noteStream()");
    return p_->streams.at(idx);
}

NoteStream& Score::noteStream(size_t idx)
{
    QPROPS_ASSERT_LT(idx, size_t(p_->streams.size()), "in Score::noteStream()");
    return p_->streams[idx];
}

const QList<NoteStream>& Score::noteStreams() const
{
    return p_->streams;
}


void Score::clear()
{
    clearProperties();
    clearScore();
}

void Score::clearProperties()
{
    p_->props.clear();
}

void Score::clearScore()
{
    p_->streams.clear();
}

void Score::setNoteStream(size_t idx, const NoteStream& s)
{
    QPROPS_ASSERT_LT(idx, numNoteStreams(), "in Score::setNoteStream()");
    p_->streams[idx] = s;
}

void Score::appendNoteStream(const NoteStream& s)
{
    p_->streams.append(s);
}

void Score::insertNoteStream(size_t idx, const NoteStream& s)
{
    if (idx >= size_t(p_->streams.size()))
        p_->streams.append(s);
    else
        p_->streams.insert(idx, s);
}

void Score::removeNoteStream(size_t idx)
{
    QPROPS_ASSERT_LT(idx, size_t(p_->streams.size()),
                    "in Score::removeNoteStream()");
    p_->streams.removeAt(idx);
}

void Score::removeNoteStreams(size_t idx, int64_t count)
{
    QPROPS_ASSERT_LT(idx, size_t(p_->streams.size()),
                    "in Score::removeNoteStreams("
                     << idx << "," << count << ")");
    if (count < 0)
        count = p_->streams.size();
    while (count && (size_t)p_->streams.size() >= idx)
    {
        p_->streams.removeAt(idx);
        --count;
    }
}




// ####################### index ##############################

Score::Index Score::index(
        size_t stream, size_t bar, size_t row, size_t column) const
{
    Index idx;
    idx.p_score = const_cast<Score*>(this);
    idx.p_stream = stream;
    idx.p_bar = bar;
    idx.p_row = row;
    idx.p_column = column;
    return idx;
}

QString Score::Index::toString() const
{
    if (isValid())
        return QString("(%1,%2,%3,%4)")
                    .arg(p_stream).arg(p_bar).arg(p_row).arg(p_column);

    QString s = "invalid(";
    if (!score())
        return s + "NULL)";

    if (stream() >= score()->numNoteStreams())
        return s + QString("%1>=%2)")
                    .arg(stream()).arg(score()->numNoteStreams());
    s += QString("%1,").arg(stream());
    const NoteStream& stream_ = score()->noteStream(stream());

    if (bar() >= stream_.numBars())
        return s + QString("%1>=%2)")
                    .arg(bar()).arg(stream_.numBars());
    s += QString("%1,").arg(bar());

    if (row() >= stream_.numRows())
        return s + QString("%1>=%2)")
                    .arg(row()).arg(stream_.numRows());
    s += QString("%1,").arg(row());

    const Bar& bar_ = stream_.bar(bar());
    if (column() >= bar_.notes(row()).length())
        return s + QString("%1>=%2)")
                    .arg(column()).arg(bar_.notes(row()).length());
    return s + QString("%1)").arg(column());
}


bool Score::Index::operator == (const Index& rhs) const
{
    return p_score == rhs.p_score
        && p_stream == rhs.p_stream
        && p_bar == rhs.p_bar
        && p_row == rhs.p_row
        && p_column == rhs.p_column;
}

bool Score::Index::operator < (const Index& o) const
{
    if (!o.isValid())
        return true;
    QPROPS_ASSERT(score() == o.score(), "comparing Score::Index from "
                                        "different scores");
    return p_stream == o.p_stream
             ? p_bar == o.p_bar
               ? p_row == o.p_row
                 ? p_column < o.p_column
                 : p_row < o.p_row
               : p_bar < o.p_bar
             : p_stream < o.p_stream;
}

bool Score::Index::isValid() const
{
    return p_score != nullptr
        && stream() < (size_t)score()->noteStreams().size()
        && bar() < score()->noteStream(stream()).numBars()
        && row() < score()->noteStream(stream()).numRows()
        && column() < score()->noteStream(stream()).notes(bar(), row()).length();
}

bool Score::Index::isRight() const
{
    if (!isValid())
        return false;
    return column() + 1 == getNotes().length();
}

bool Score::Index::isBottom() const
{
    if (!isValid())
        return false;
    return row() + 1 == getBar().numRows();
}

bool Score::Index::isInserter() const
{
    if (p_score == nullptr)
        return false;
    if (stream() >= size_t(score()->noteStreams().size()))
        return false;
    if (score()->noteStream(stream()).isEmpty())
        return true;
    if (bar() >= score()->noteStream(stream()).numBars())
        return false;
    if (row() >= score()->noteStream(stream()).numRows())
        return false;
    if (score()->noteStream(stream()).notes(bar(), row()).isEmpty())
        return true;
    return false;
}

bool Score::Index::isValid(int s, int b, int r, int c) const
{
    return p_score != nullptr
        && s >= 0 && s < score()->noteStreams().size()
        && b >= 0 && (size_t)b < score()->noteStream(stream()).numBars()
        && r >= 0 && (size_t)r < score()->noteStream(stream()).numRows()
        && c >= 0 && (size_t)c < score()->noteStream(
                                stream()).notes(bar(), row()).length();
}

bool Score::Index::isFirstBar() const
{
    return bar() == 0;
}

bool Score::Index::isLastBar() const
{
    if (!isValid())
        return false;
    auto& s = getStream();
    return s.isEmpty() || (bar() + 1 == s.numBars());
}

bool Score::Index::isTempoChange() const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::isTempoChange()");
    auto i = *this;
    if (!i.prevBar())
        return false;
    return getBeatsPerMinute() != i.getBeatsPerMinute();
}

size_t Score::Index::numRows() const
{
    return isValid() ? getStream().numRows() : 0;
}

const NoteStream& Score::Index::getStream() const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::getNoteStream()");
    return score()->noteStream(stream());
}

const Bar& Score::Index::getBar() const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::getBar()");
    return score()->noteStream(stream()).bar(bar());
}

const Notes& Score::Index::getNotes(int row_) const
{
    row_ += row();
    QPROPS_ASSERT(isValid(stream(), bar(), row_, 0),
                  "in Score::Index::getBar(" << row_ << "), this="
                  << toString());
    return score()->noteStream(stream()).notes(bar(), row_);
}

/*
QList<Notes> Score::Index::getNotes(int startRow, int numRows) const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::getBars("
                             << startRow << ", " << numRows << ")");
    if (startRow < 0)
        startRow = 0;
    if (numRows < 0)
        numRows = getStream().numRows();
    numRows = std::min((size_t)numRows, getStream().numRows());

    QList<Notes> bars;
    for (int i=startRow; i<numRows; ++i)
        bars << getStream().bar(bar(), i);
    return bars;
}
*/

const Note& Score::Index::getNote() const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::getNote()");
    return score()->noteStream(stream()).notes(bar(), row()).note(column());
}

const Note& Score::Index::getNote(int r, int c) const
{
    r += row();
    c += column();
    QPROPS_ASSERT(isValid(stream(), bar(), r, c),
                  "in Score::Index::getNote(" << r << ", " << c << ")");
    return score()->noteStream(stream()).notes(bar(), r).note(c);
}

double Score::Index::getBeatsPerMinute() const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::getBeatsPerMinute()");
    return getStream().beatsPerMinute(bar());
}

double Score::Index::getBarLengthSeconds() const
{
    return getStream().barLengthSeconds(bar());
}

namespace {
    size_t get_limit(size_t x, size_t end)
    {
        return x < end ? x : end > 0 ? end - 1 : 0;
    }
}

Score::Index Score::Index::right() const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::right()");
    auto& b = getBar();
    int len = b[row()].length(),
        col = get_limit(len, len);
    return score()->index(stream(), bar(), row(), col);
}

Score::Index Score::Index::bottom() const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::bottom()");
    auto& b = getBar();
    int row = get_limit(b.numRows(), b.numRows());
    return score()->index(stream(), bar(), row, column());
}

Score::Index Score::Index::bottomRight() const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::bottomRight()");
    auto& b = getBar();
    int row = get_limit(b.numRows(), b.numRows()),
        len = b[row].length(),
        col = get_limit(len, len);
    return score()->index(stream(), bar(), row, col);
}

Score::Index Score::Index::streamBottomRight() const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::streamBottomRight()");
    auto& s = getStream();
    int bar = get_limit(s.numBars(), s.numBars());
    auto& b = s.bar(bar);
    int row = get_limit(b.numRows(), b.numRows()),
        len = b[row].length(),
        col = get_limit(len, len);
    return score()->index(stream(), bar, row, col);
}

Score::Index Score::Index::closest(const Index &i1,
                                   const Index &i2) const
{
    int64_t
    d1 = std::abs((int64_t)stream() - (int64_t)i1.stream()),
    d2 = std::abs((int64_t)stream() - (int64_t)i2.stream());
    if (d1 < d2) return i1;
    if (d2 < d1) return i2;
    d1 = std::abs((int64_t)bar() - (int64_t)i1.bar());
    d2 = std::abs((int64_t)bar() - (int64_t)i2.bar());
    if (d1 < d2) return i1;
    if (d2 < d1) return i2;
    d1 = std::abs((int64_t)row() - (int64_t)i1.row());
    d2 = std::abs((int64_t)row() - (int64_t)i2.row());
    if (d1 < d2) return i1;
    if (d2 < d1) return i2;
    d1 = std::abs((int64_t)column() - (int64_t)i1.column());
    d2 = std::abs((int64_t)column() - (int64_t)i2.column());
    return d1 < d2 ? i1 : i2;
}

Score::Index Score::Index::farthestManhatten(const Index &i1,
                                             const Index &i2) const
{
    return closestManhatten(i1, i2) == i1 ? i2 : i1;
}

Score::Index Score::Index::closestManhatten(const Index &i1,
                                            const Index &i2) const
{
    int64_t
    d1 = std::abs((int64_t)stream() - (int64_t)i1.stream()),
    d2 = std::abs((int64_t)stream() - (int64_t)i2.stream());
    if (d1 < d2) return i1;
    if (d2 < d1) return i2;
    d1 = std::abs((int64_t)bar() - (int64_t)i1.bar());
    d2 = std::abs((int64_t)bar() - (int64_t)i2.bar());
    if (d1 < d2) return i1;
    if (d2 < d1) return i2;
    d1 = std::abs((int64_t)row() - (int64_t)i1.row());
    d2 = std::abs((int64_t)row() - (int64_t)i2.row());
    d1 += std::abs((int64_t)column() - (int64_t)i1.column());
    d2 += std::abs((int64_t)column() - (int64_t)i2.column());
    return d1 < d2 ? i1 : i2;
}

Score::Index Score::Index::limitRight() const
{
    Index x = *this;
    if (!x.p_score)
        return x;

    x.p_stream = get_limit(x.p_stream, p_score->numNoteStreams());
    if (x.p_stream < p_score->numNoteStreams())
    {
        auto& s = p_score->noteStream(x.p_stream);
        x.p_row = get_limit(x.p_row, s.numRows());
        if (x.p_row < s.numRows())
        {
            x.p_bar = get_limit(x.p_bar, s.numBars());
            if (x.p_bar < s.numBars())
            {
                auto& b = s.notes(x.p_bar, x.p_row);
                x.p_column = get_limit(x.p_column, b.length());
            }
        }
    }
    return x;
}

bool Score::Index::nextStream()
{
    if (!isValid() || p_stream + 1 >= score()->numNoteStreams())
        return false;

    auto& nextStream = score()->noteStream(p_stream+1);
    if (nextStream.isEmpty())
        return false;

    size_t row = get_limit(p_row, nextStream.numRows());
    if (nextStream.notes(0, row).isEmpty())
        return false;

    p_stream = p_stream + 1;
    p_row = row;
    p_bar = 0;
    p_column = 0;

    return true;
}

bool Score::Index::prevStream()
{
    if (!isValid() || p_stream == 0)
        return false;

    auto& nextStream = score()->noteStream(p_stream-1);
    if (nextStream.isEmpty())
        return false;

    size_t row = get_limit(p_row, nextStream.numRows());
    if (nextStream.notes(nextStream.numBars()-1, row).isEmpty())
        return false;

    p_stream = p_stream - 1;
    p_row = row;
    p_bar = nextStream.numBars() - 1;
    p_column = getNotes().length() - 1;

    return true;
}

bool Score::Index::nextBar()
{
    if (!isValid())
        return false;

    if (p_bar + 1 >= getStream().numBars())
        return nextStream();

    auto& st = p_score->noteStream(p_stream);
    size_t row = get_limit(p_row, st.numRows());
    if (st.notes(p_bar+1, row).isEmpty())
        return false;

    p_row = row;
    p_bar = p_bar + 1;
    p_column = 0;
    return true;
}

bool Score::Index::prevBar()
{
    if (!isValid())
        return false;

    if (bar() == 0)
        return prevStream();

    auto& st = p_score->noteStream(p_stream);
    size_t row = get_limit(p_row, st.numRows());
    if (st.notes(p_bar-1, row).isEmpty())
        return false;

    p_row = row;
    p_bar = p_bar - 1;
    p_column = st.notes(p_bar, row).length() - 1;
    return true;
}


bool Score::Index::nextNote()
{
    if (!isValid())
        return false;
    if (column() + 1 >= getNotes().length())
    {
        if (bar() + 1 >= getStream().numBars())
            return nextStream();

        return nextBar();
    }

    ++p_column;
    return true;
}

bool Score::Index::prevNote()
{
    if (!isValid())
        return false;
    if (column() == 0)
    {
        if (bar() == 0)
            return prevStream();

        return prevBar();
    }

    --p_column;
    return true;
}


bool Score::Index::nextRow()
{
    if (!isValid() || row()+1 >= getStream().numRows())
        return false;

    auto& st = score()->noteStream(stream());
    if (st.notes(bar(), p_row+1).isEmpty())
        return false;

    ++p_row;
    p_column = get_limit(p_column, st.notes(bar(), row()).length());
    return true;
}

bool Score::Index::prevRow()
{
    if (!isValid() || row() < 1)
        return false;

    auto& st = score()->noteStream(stream());
    if (st.notes(bar(), p_row-1).isEmpty())
        return false;

    --p_row;
    p_column = get_limit(p_column, st.notes(bar(), row()).length());
    return true;
}

// ------- setter --------

void Score::Index::setNote(const Note& n)
{
    QPROPS_ASSERT(isValid(), "in Score::Index::setNote()");
    score()->noteStream(stream())
             .bar(bar()).notes(row()).setNote(column(), n);
}



// ####################### Selection ##############################

Score::Selection::Selection(const Index& i1, const Index& i2)
    : p_from    (i1)
    , p_to      (i1)
{
    unifyWith(i2);
}

QString Score::Selection::toString() const
{
    return QString("(%1 %2)").arg(p_from.toString()).arg(p_to.toString());
}

Score* Score::Selection::score() const { return p_from.score(); }

bool Score::Selection::isValid() const
    { return p_from.isValid() && p_to.isValid(); }

bool Score::Selection::isSingleNote() const
    { return isValid() && isSingleRow() && isSingleColumn(); }

bool Score::Selection::isSingleRow() const
    { return isValid() && p_from.row() == p_to.row(); }

bool Score::Selection::isSingleColumn() const
    { return isValid() && p_from.column() == p_to.column() && isSingleBar(); }

bool Score::Selection::isSingleBar() const
    { return isValid() && p_from.bar() == p_to.bar() && isSingleStream(); }

bool Score::Selection::isSingleStream() const
    { return isValid() && p_from.stream() == p_to.stream(); }

bool Score::Selection::isCompleteRow() const
    { return isValid() && p_from.isLeft() && p_to.isRight(); }

bool Score::Selection::isCompleteColumn() const
    { return isValid() && p_from.isTop() && p_to.isBottom(); }

bool Score::Selection::isCompleteBar() const
    { return isValid() && p_from.isTop() && p_from.isLeft()
          && p_to.isBottom() && p_to.isRight(); }

bool Score::Selection::isCompleteStream() const
    { return isValid() && p_from.isFirstBar() && p_to.isLastBar(); }

bool Score::Selection::isCompleteScore() const
    { return isValid() && p_from.stream() == 0
            && p_to.stream()+1 == p_from.score()->numNoteStreams(); }


bool Score::Selection::contains(const Index &idx) const
{
    /*if (!isValid() || !idx.isValid())
        return false;

    return ( idx.stream() >= from().stream() && idx.stream() <= to().stream()
           && idx.row() >= from().row() && idx.row() <= to().row()
             );
    */

    if (!( isValid() && idx.isValid()
        && idx.stream() >= from().stream() && idx.stream() <= to().stream()
        && idx.row() >= from().row() && idx.row() <= to().row() ))
        return false;

    if ((idx.stream() == from().stream() && idx.bar() < from().bar())
      ||(idx.stream() == to().stream() && idx.bar() > to().bar()))
        return false;

    if ((idx.stream() == from().stream() && idx.bar() == from().bar()
         && idx.column() < from().column())
      ||(idx.stream() == to().stream() && idx.bar() == to().bar()
         && idx.column() > to().column()))
        return false;

    return true;
}

QList<Score::Index> Score::Selection::containedNoteIndices() const
{
    QList<Index> list;
    if (!isValid())
        return list;
    for (size_t rw=from().row(); rw<=to().row(); ++rw)
    {
        Index cursor = score()->index(from().stream(),
                                      from().bar(),
                                      rw,
                                      from().column());
        while (cursor.isValid())
        {
            list << cursor;
            if (cursor.stream() == to().stream()
             && cursor.bar() == to().bar()
             && cursor.column() == to().column())
                break;
            cursor.nextNote();
        }
    }
    return list;
}

void Score::Selection::set(const Index& idx)
{
    p_from = p_to = idx;
}

/** expand current selection */
Score::Selection& Score::Selection::unifyWith(const Index& idx)
{
    if (!isValid() || !idx.isValid())
        return *this;

    QPROPS_ASSERT(idx.score() == score(), "in Score::Selection::unifyWith("
                                          << idx.toString() << ")");
    p_from = score()->index(
                std::min(p_from.stream(), idx.stream()),
                p_from.stream() == idx.stream()
                    ? std::min(p_from.bar(), idx.bar())
                    : p_from.stream() < idx.stream()
                      ? p_from.bar() : idx.bar(),
                std::min(p_from.row(), idx.row()),
                p_from.bar() == idx.bar()
                    ? std::min(p_from.column(), idx.column())
                    : p_from.bar() < idx.bar()
                      ? p_from.column() : idx.column() );
    p_to = score()->index(
                std::max(p_to.stream(), idx.stream()),
                p_to.stream() == idx.stream()
                    ? std::max(p_to.bar(), idx.bar())
                    : p_to.stream() > idx.stream()
                      ? p_to.bar() : idx.bar(),
                std::max(p_to.row(), idx.row()),
                p_to.bar() == idx.bar()
                    ? std::max(p_to.column(), idx.column())
                    : p_to.bar() > idx.bar()
                      ? p_to.column() : idx.column() );
    return *this;
}

Score::Selection Score::Selection::unified(const Index& idx) const
{
    auto s = *this;
    return s.unifyWith(idx);
}

Score::Selection Score::Selection::fromNotes(const Index& i)
{
    if (!i.isValid())
        return Selection();
    return Selection(i.left(), i.right());
}

Score::Selection Score::Selection::fromColumn(const Index& i)
{
    if (!i.isValid())
        return Selection();
    return Selection(i.top(), i.bottom());
}

Score::Selection Score::Selection::fromBar(const Index& i)
{
    if (!i.isValid())
        return Selection();
    return Selection(i.topLeft(), i.bottomRight());
}

Score::Selection Score::Selection::fromStream(const Index& i)
{
    if (!i.isValid())
        return Selection();
    return Selection(i.streamTopLeft(), i.streamBottomRight());
}

Score::Selection Score::Selection::fromBars(const Selection& sel)
{
    if (!sel.isValid())
        return Selection();
    return Selection(sel.from().topLeft(), sel.to().bottomRight());
}

} // namespace Sonot
