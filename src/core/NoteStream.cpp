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
#include "io/error.h"

namespace Sonot {

NoteStream::NoteStream()
{

}

size_t NoteStream::numNotes() const
{
    size_t n = 0;
    for (const Bar& b : p_data_)
        n += b.length();
    return n;
}

uint8_t NoteStream::numRows() const
{
    uint8_t n = 0;
    for (const Bar& b : p_data_)
        n = std::max(n, b.numRows());
    return n;
}

const Bar& NoteStream::bar(size_t idx) const
{
    SONOT_ASSERT_LT(idx, numBars(), "in NoteStream::bar()");
    return p_data_[idx];
}

Bar& NoteStream::bar(size_t idx)
{
    SONOT_ASSERT_LT(idx, numBars(), "in NoteStream::bar()");
    return p_data_[idx];
}


void NoteStream::removeBar(size_t idx)
{
    SONOT_ASSERT_LT(idx, numBars(), "in NoteStream::removeBar()");
    p_data_.erase(p_data_.begin() + idx);
}

void NoteStream::removeBars(size_t idx, int count)
{
    SONOT_ASSERT_LT(idx, numBars(), "in NoteStream::removeBars()");
    if (count == 0)
        return;
    if (count < 0)
        count = numBars() - idx;
    SONOT_ASSERT_LTE(idx + count, numBars(),
                     "in NoteStream::removeBars(" << idx << ", " << count << ")");

    p_data_.erase(p_data_.begin() + idx, p_data_.begin() + idx + count);
}

void NoteStream::insertBar(size_t idx, const Bar &b)
{
    if (idx < numBars())
        p_data_.insert(p_data_.begin() + idx, b);
    else
        p_data_.push_back(b);
}

void NoteStream::appendBar(const Bar &b)
{
    p_data_.push_back(b);
}


QString NoteStream::toString() const
{
    size_t w = numNotes() + (numBars() + 1) + 1,
           h = numRows();

    // init string with rows and line-breaks
    QString s(h * w, '-');
    for (size_t y=0; y<h; ++y)
        s[uint(y*w + w-2)] = '|',
        s[uint(y*w + w-1)] = '\n';

    size_t x = 0;
    for (const Bar& b : p_data_)
    {
        // left Bar marker
        for (size_t y=0; y<h; ++y)
            s[uint(y*w + x)] = '|';
        x += 1;
        // note values
        for (int8_t y=0; y<b.numRows(); ++y)
        for (int8_t i=0; i<b.length(); ++i)
        {
            const Note n = b.note(i, y);
            if (n.isNote())
                s[uint(y*w + x + i)] = n.toNoteString()[0];
        }
        x += b.length();
    }

    return s;
}



QJsonObject NoteStream::toJson() const
{
    JsonHelper json("NoteStream");

    QJsonArray jbars;
    for (const Bar& bar : p_data_)
    {
        jbars.append(QJsonValue(bar.toJson()));
    }

    QJsonObject o;
    o.insert("bars", jbars);
    return o;
}

void NoteStream::fromJson(const QJsonObject& o)
{
    JsonHelper json("NoteStream");

    QJsonArray jbars = json.expectArray(json.expectChildValue(o, "bars"));

    std::vector<Bar> data;
    for (int i=0; i<jbars.size(); ++i)
    {
        Bar bar;
        bar.fromJson(json.expectObject(jbars.at(i)));
        data.push_back(bar);
    }

    p_data_.swap(data);
}


} // namespace Sonot
