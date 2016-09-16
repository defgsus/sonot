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

NoteStream::NoteStream()
{

}

bool NoteStream::operator == (const NoteStream& rhs) const
{
    return p_data_ == rhs.p_data_;
}

Bar NoteStream::defaultBar(size_t len) const
{
    return Bar(std::max(size_t(1), len));
}

QList<Bar> NoteStream::defaultBarRows(size_t len) const
{
    size_t c = std::max(size_t(1), numRows());
    QList<Bar> rows;
    for (size_t i=0; i<c; ++i)
        rows << defaultBar(len);
    return rows;
}

size_t NoteStream::numNotes() const
{
    size_t n = 0;
    for (const std::vector<Bar>& bars : p_data_)
    {
        size_t m = 0;
        for (const Bar& b : bars)
            m = std::max(m, b.length());
        n += m;
    }
    return n;
}

size_t NoteStream::numNotes(size_t barIdx) const
{
    QPROPS_ASSERT_LT(barIdx, numBars(), "in NoteStream::numNotes()");
    size_t n = 0;
    for (const Bar& b : p_data_[barIdx])
        n = std::max(n, b.length());
    return n;
}

size_t NoteStream::numRows() const
{
    return p_data_.empty() ? 0
                           : p_data_.front().size();
}

const Bar& NoteStream::bar(size_t idx, size_t row) const
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::bar()");
    QPROPS_ASSERT_LT(row, numRows(), "in NoteStream::bar()");
    return p_data_[idx][row];
}

Bar& NoteStream::bar(size_t idx, size_t row)
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::bar()");
    QPROPS_ASSERT_LT(row, numRows(), "in NoteStream::bar()");
    return p_data_[idx][row];
}

const Note& NoteStream::note(size_t idx, size_t row, size_t column) const
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::note()");
    QPROPS_ASSERT_LT(row, numRows(), "in NoteStream::note()");
    const Bar& b = bar(idx, row);
    QPROPS_ASSERT_LT(column, b.length(), "in NoteStream::note()");
    return b.note(column);
}

QList<Bar> NoteStream::getRows(size_t idx) const
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::getRows()");

    QList<Bar> rows;
    for (size_t i=0; i<numRows(); ++i)
        rows << bar(idx, i);
    return rows;
}

void NoteStream::setNote(size_t idx, size_t row, size_t column, const Note& n)
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::setNote()");
    QPROPS_ASSERT_LT(row, numRows(), "in NoteStream::setNote()");
    Bar& b = bar(idx, row);
    QPROPS_ASSERT_LT(column, b.length(), "in NoteStream::setNote()");
    b.setNote(column, n);
}

void NoteStream::removeBar(size_t idx)
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::removeBar()");
    p_data_.erase(p_data_.begin() + idx);
}

void NoteStream::setNumRows(size_t newRows)
{
    if (newRows == numRows())
        return;
    for (std::vector<Bar>& rows : p_data_)
    {
        size_t n = rows.size();
        rows.resize(newRows);
        for (size_t i=n; i<newRows; ++i)
            rows[n] = defaultBar();
    }
}


void NoteStream::removeBars(size_t idx, int count)
{
    QPROPS_ASSERT_LT(idx, numBars(), "in NoteStream::removeBars()");
    if (count == 0)
        return;
    if (count < 0)
        count = numBars() - idx;
    QPROPS_ASSERT_LTE(idx + count, numBars(),
                      "in NoteStream::removeBars("
                      << idx << ", " << count << ")");

    p_data_.erase(p_data_.begin() + idx, p_data_.begin() + idx + count);
}

void NoteStream::insertBar(size_t idx, const Bar &b)
{
    std::vector<Bar> bars;
    bars.push_back(b);
    for (size_t i=1; i<numRows(); ++i)
        bars.push_back( Bar(1) );

    if (idx < numBars())
        p_data_.insert(p_data_.begin() + idx, bars);
    else
        p_data_.push_back(bars);
}



void NoteStream::insertBar(size_t idx, const QList<Bar>& barList)
{
    if (barList.isEmpty())
        return;

    std::vector<Bar> bars;
    for (auto& b : barList)
        bars.push_back( b );

    if (bars.size() < numRows())
    {
        // append empty bars to insertion
        for (size_t i=1; i<numRows(); ++i)
            bars.push_back( defaultBar() );
    }
    else if (bars.size() > numRows())
        // otherwise resize data
        setNumRows(bars.size());

    if (idx < numBars())
        p_data_.insert(p_data_.begin() + idx, bars);
    else
        p_data_.push_back(bars);
}

void NoteStream::insertRow(size_t row)
{
    for (std::vector<Bar>& barBlock : p_data_)
    {
        size_t len = 1;
        if (!barBlock.empty())
        {
            len = barBlock.front().length();
            for (auto& b : barBlock)
                len = std::max(len, b.length());
        }
        Bar bar = defaultBar(len);

        if (row < barBlock.size())
            barBlock.insert(barBlock.begin() + row, bar);
        else
            barBlock.push_back(bar);
    }
}

void NoteStream::removeRow(size_t row)
{
    if (row >= numRows())
        return;

    for (std::vector<Bar>& barBlock : p_data_)
    {
        barBlock.erase(barBlock.begin() + row);
    }
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
            const Bar& b = bar(barIdx, y);
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
            rows[r] += QString::number(bar(b, r).length());
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

    // write [[bar,bar,bar],[bar,barbar]]
    QJsonArray jrows;
    for (size_t row = 0; row < numRows(); ++row)
    {
        QJsonArray jbars;
        for (size_t i = 0; i < numBars(); ++i)
        {
            jbars.append(QJsonValue(bar(i, row).toJson()));
        }
        jrows.append( jbars );
    }

    QJsonObject o;
    o.insert("bars", jrows);
    return o;
}

void NoteStream::fromJson(const QJsonObject& o)
{
    QProps::JsonInterfaceHelper json("NoteStream");

    QJsonArray jrows = json.expectArray(json.expectChildValue(o, "bars"));

    std::vector<std::vector<Bar>> data;
    for (int row=0; row<jrows.size(); ++row)
    {
        json.beginContext(QString("Reading row %1").arg(row));
        QJsonArray jbars = json.expectArray(jrows[row]);
        if (data.empty())
        {
            data.resize(jbars.size());
            for (int i=0; i<jbars.size(); ++i)
                data[i].resize(jrows.size());
        }
        else if (data.size() != size_t(jbars.size()))
                QPROPS_IO_ERROR("row length mismatch, expected "
                                << data.size() << ", got " << jbars.size());
        for (int i=0; i<jbars.size(); ++i)
        {
            data[i][row].fromJson(json.expectObject(jbars.at(i)));
        }

        json.endContext();
    }

    p_data_.swap(data);
}


} // namespace Sonot
