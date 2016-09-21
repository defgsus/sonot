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

#include <QJsonObject>
#include <QJsonArray>

#include "NoteStream.h"
#include "QProps/JsonInterfaceHelper.h"
#include "QProps/error.h"

namespace Sonot {

namespace { const double defaultBpm_ = 120.; }

NoteStream::NoteStream()
    : p_props_      ("note-stream")
{
    p_props_.set("bpm", tr("tempo"), tr("Tempo in beats-per-minute"),
                 defaultBpm_);
    p_props_.setMin("bpm", 1.);

    p_props_.set("keysig", tr("key signature"),
                 tr("a list of keys applying to this part"),
                 QString());

    p_props_.set("title", tr("title"),
                 tr("The title of this part"),
                 QString());
    p_props_.set("source", tr("source"),
                 tr("Narrow description of the source, like page number"),
                 QString());

    p_props_.set("transcriber", tr("transcriber"),
                 tr("The one who did the typing"),
                 QString());
}

bool NoteStream::operator == (const NoteStream& rhs) const
{
    return p_data_ == rhs.p_data_;
}

Notes NoteStream::createDefaultNotes(size_t len) const
{
    return Notes(std::max(size_t(1), len));
}

Bar NoteStream::createDefaultBar(size_t len) const
{
    size_t c = std::max(size_t(1), numRows());
    Bar bar;
    for (size_t i=0; i<c; ++i)
        bar.append( createDefaultNotes(len) );
    return bar;
}

NoteStream NoteStream::createDefaultStream(
        size_t numBars, size_t barLen) const
{
    NoteStream s;
    for (size_t i=0; i<std::max(size_t(1), numBars); ++i)
        s.appendBar( createDefaultBar(barLen) );
    s.p_props_ = p_props_;
    return s;
}

const QProps::Properties& NoteStream::props() const
{
    return p_props_;
}

void NoteStream::setProperties(const QProps::Properties& props)
{
    p_props_ = props;
}

KeySignature NoteStream::keySignature() const
{
    KeySignature s;
    s.fromString(props().get("keysig").toString());
    return s;
}

size_t NoteStream::numNotes() const
{
    size_t n = 0;
    for (const Bar& bar : p_data_)
        n += bar.maxNumberNotes();
    return n;
}

size_t NoteStream::numNotes(size_t barIdx) const
{
    QPROPS_ASSERT_LT(barIdx, numBars(), "in NoteStream::numNotes()");
    size_t n = 0;
    for (const Notes& b : p_data_[barIdx])
        n = std::max(n, b.length());
    return n;
}

size_t NoteStream::numRows() const
{
    return p_data_.empty() ? 0
                           : p_data_.front().numRows();
}

const Bar& NoteStream::bar(size_t idx) const
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::bar()");
    return p_data_[idx];
}

Bar& NoteStream::bar(size_t idx)
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::bar()");
    return p_data_[idx];
}

const Notes& NoteStream::notes(size_t idx, size_t row) const
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::bar()");
    QPROPS_ASSERT_LT(row, numRows(), "in NoteStream::bar()");
    return p_data_[idx][row];
}

const Note& NoteStream::note(size_t idx, size_t row, size_t column) const
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::note("
                     << idx << "," << row << "," << column << ")");
    QPROPS_ASSERT_LT(row, numRows(), "in NoteStream::note("
                     << idx << "," << row << "," << column << ")");
    const Notes& b = notes(idx, row);
    QPROPS_ASSERT_LT(column, b.length(), "in NoteStream::note()");
    return b.note(column);
}

QList<Notes> NoteStream::getRows(size_t idx) const
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::getRows("
                     << idx << ")");

    QList<Notes> rows;
    for (size_t i=0; i<numRows(); ++i)
        rows << notes(idx, i);
    return rows;
}

double NoteStream::beatsPerMinute(size_t idx) const
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::beatsPerMinute("
                     << idx << ")");

    return std::max(1., p_props_.get("bpm", defaultBpm_).toDouble());
}

double NoteStream::barLengthSeconds(size_t idx) const
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::barLengthSeconds("
                     << idx << ")");
    return 4. * 60. / beatsPerMinute(idx);
}


void NoteStream::setNote(size_t idx, size_t row, size_t column, const Note& n)
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::setNote("
                     << idx << "," << row << "," << column << ")");
    QPROPS_ASSERT_LT(row, numRows(), "in NoteStream::setNote("
                     << idx << "," << row << "," << column << ")");
    Notes& b = p_data_[idx][row];
    QPROPS_ASSERT_LT(column, b.length(), "in NoteStream::setNote("
                     << idx << "," << row << "," << column << ")");
    b.setNote(column, n);
}

void NoteStream::removeBar(size_t idx)
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::removeBar("
                     << idx << ")");
    p_data_.erase(p_data_.begin() + idx);
}

void NoteStream::setNumRows(size_t newRows)
{
    if (newRows == numRows())
        return;
    for (Bar& bar : p_data_)
    {
        size_t n = bar.numRows();
        bar.resize(newRows);
        for (size_t i=n; i<newRows; ++i)
            bar[n] = createDefaultNotes();
    }
}


void NoteStream::removeBars(size_t idx, int64_t count)
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::removeBars("
                     << idx << "," << count << ")");
    if (count == 0)
        return;
    if (count < 0)
        count = numBars() - idx;
    QPROPS_ASSERT_LTE(idx + count, numBars(),
                      "in NoteStream::removeBars("
                      << idx << ", " << count << ")");

    p_data_.erase(p_data_.begin() + idx, p_data_.begin() + idx + count);
}

void NoteStream::insertBar(size_t idx, const Notes &b)
{
    Bar bar;
    bar.append(b);
    for (size_t i=1; i<numRows(); ++i)
        bar.append( Notes(b.length()) );

    if (idx < numBars())
        p_data_.insert(p_data_.begin() + idx, bar);
    else
        p_data_.push_back(bar);
}



void NoteStream::insertBar(size_t idx, const Bar& barIn)
{
    Bar bar(barIn);

    if (bar.numRows() != numRows())
        bar.resize(bar.numRows());

    if (idx < numBars())
        p_data_.insert(p_data_.begin() + idx, bar);
    else
        p_data_.push_back(bar);
}

void NoteStream::insertRow(size_t row)
{
    for (Bar& bar : p_data_)
    {
        Notes notes = createDefaultNotes( bar.maxNumberNotes() );
        bar.insert(row, notes);
    }
}

void NoteStream::removeRow(size_t row)
{
    if (row >= numRows())
        return;

    for (Bar& bar : p_data_)
        bar.remove(row);
}

QString NoteStream::toTabString() const
{
    size_t w = numNotes() + (numBars() + 1) + 1,
           h = numRows();

    // init string with rows and line-breaks
    QString s(h * w, '-');
    for (size_t y=0; y<h; ++y)
        s[uint(y*w + w-2)] = '|',
        s[uint(y*w + w-1)] = '\n';

    size_t x = 0;
    for (size_t barIdx = 0; barIdx < numBars(); ++barIdx)
    {
        // left Bar marker
        for (size_t y=0; y<h; ++y)
            s[uint(y*w + x)] = '|';
        x += 1;
        // note values
        const size_t blength = numNotes(barIdx);
        for (size_t i=0; i<blength; ++i)
        for (size_t y=0; y<numRows(); ++y)
        {
            const Notes& b = notes(barIdx, y);
            Note n(Note::Space);
            if (i < b.length())
                n = b.note(i);
            if (n.isNote())
                s[uint(y*w + x + i)] = n.to3String()[0];
        }
        x += blength;
    }

    return s;
}

QString NoteStream::toInfoString() const
{
    if (numRows() < 1)
        return QString();

    QVector<QString> rows(numRows());
    for (QString& r : rows)
        r = "|";
    for (size_t b=0; b<numBars(); ++b)
    {
        // info per bar-block
        int ma = 0;
        for (size_t r=0; r<numRows(); ++r)
        {
            rows[r] += QString::number(notes(b, r).length());
            ma = std::max(ma, rows[r].length());
        }
        // padding
        for (size_t r=0; r<numRows(); ++r)
        {
            rows[r] += QString(" ").repeated(ma - rows[r].length());
            rows[r] += "|";
        }
    }

    QString s = rows[0];
    for (int i=1; i<rows.size(); ++i)
        s += "\n" + rows[i];
    return s;
}



QJsonObject NoteStream::toJson() const
{
    QProps::JsonInterfaceHelper json("NoteStream");

    QJsonArray jbars;
    for (const Bar& p : p_data_)
    {
        jbars.append( p.toJson() );
    }

    QJsonObject o;
    o.insert("music", jbars);

    o.insert("properties", p_props_.toJson());
    return o;
}

void NoteStream::fromJson(const QJsonObject& o)
{
    QProps::JsonInterfaceHelper json("NoteStream");

    std::vector<Bar> data;

    QJsonArray jbars = json.expectChildArray(o, "music");

    for (int i=0; i<jbars.size(); ++i)
    {
        json.beginContext(QString("Reading bar %1").arg(i));
        Bar bar;
        bar.fromJson(json.expectObject(jbars[i]));
        data.push_back(bar);
        if (i > 0 && data[i].numRows() != data[i-1].numRows())
            QPROPS_IO_ERROR("row length mismatch, expected "
                            << data[i-1].numRows()
                            << ", got " << data[i].numRows());
        json.endContext();
    }

    if (o.contains("properties"))
    {
        QJsonObject jprops = json.expectChildObject(o, "properties");
        auto props = p_props_;
        props.fromJson(jprops);
        p_props_.swap(props);
    }

    p_data_.swap(data);
}


} // namespace Sonot
