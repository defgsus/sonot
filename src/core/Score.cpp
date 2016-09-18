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
        props.set("copyright", tr("copyright"),
                tr("Copyleft/right information"), QString());
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


// ----------------------- index ------------------------------

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
    auto s = QString("(%1,%2,%3,%4)")
                    .arg(p_stream).arg(p_bar).arg(p_row).arg(p_column);
    if (!isValid())
        s.prepend("invalid");
    return s;
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
        && column() < score()->noteStream(stream()).bar(bar(), row()).length();
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
    if (score()->noteStream(stream()).bar(bar(), row()).isEmpty())
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
                                stream()).bar(bar(), row()).length();
}

bool Score::Index::isStreamStart() const
{
    return bar() == 0;
}

bool Score::Index::isStreamEnd() const
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

const Bar& Score::Index::getBar(int row_) const
{
    row_ += row();
    QPROPS_ASSERT(isValid(stream(), bar(), row_, 0),
                  "in Score::Index::getBar(" << row_ << "), this="
                  << toString());
    return score()->noteStream(stream()).bar(bar(), row_);
}

QList<Bar> Score::Index::getBars(int startRow, int numRows) const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::getBars("
                             << startRow << ", " << numRows << ")");
    if (startRow < 0)
        startRow = 0;
    if (numRows < 0)
        numRows = getStream().numRows();
    numRows = std::min((size_t)numRows, getStream().numRows());

    QList<Bar> bars;
    for (int i=startRow; i<numRows; ++i)
        bars << getStream().bar(bar(), i);
    return bars;
}

const Note& Score::Index::getNote() const
{
    QPROPS_ASSERT(isValid(), "in Score::Index::getNote()");
    return score()->noteStream(stream()).bar(bar(), row()).note(column());
}

const Note& Score::Index::getNote(int r, int c) const
{
    r += row();
    c += column();
    QPROPS_ASSERT(isValid(stream(), bar(), r, c),
                  "in Score::Index::getNote(" << r << ", " << c << ")");
    return score()->noteStream(stream()).bar(bar(), r).note(c);
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
                auto& b = s.bar(x.p_bar, x.p_row);
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
    if (nextStream.bar(0, row).isEmpty())
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
    if (nextStream.bar(nextStream.numBars()-1, row).isEmpty())
        return false;

    p_stream = p_stream - 1;
    p_row = row;
    p_bar = nextStream.numBars() - 1;
    p_column = getBar().length() - 1;

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
    if (st.bar(p_bar+1, row).isEmpty())
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
    if (st.bar(p_bar-1, row).isEmpty())
        return false;

    p_row = row;
    p_bar = p_bar - 1;
    p_column = st.bar(p_bar, row).length() - 1;
    return true;
}


bool Score::Index::nextNote()
{
    if (!isValid())
        return false;
    if (column() + 1 >= getBar().length())
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
    if (st.bar(bar(), p_row+1).isEmpty())
        return false;

    ++p_row;
    p_column = get_limit(p_column, st.bar(bar(), row()).length());
    return true;
}

bool Score::Index::prevRow()
{
    if (!isValid() || row() < 1)
        return false;

    auto& st = score()->noteStream(stream());
    if (st.bar(bar(), p_row-1).isEmpty())
        return false;

    --p_row;
    p_column = get_limit(p_column, st.bar(bar(), row()).length());
    return true;
}

} // namespace Sonot
