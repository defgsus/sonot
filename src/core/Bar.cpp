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
#include <QStringList>

#include "Bar.h"
#include "io/error.h"

namespace Sonot {

Bar::Bar(uint8_t length, uint8_t numRows)
    : p_numNotes_   (length)
    , p_numRows_    (numRows)
    , p_data_       (p_numNotes_ * p_numRows_, Note(Note::Space))
{

}

const Note& Bar::note(uint8_t column, uint8_t row) const
{
    SONOT_ASSERT_LT(column, p_numNotes_, "in Bar::note()");
    SONOT_ASSERT_LT(row, p_numRows_, "in Bar::note()");

    return p_data_[row * p_numNotes_ + column];
}

void Bar::resize(uint8_t length, uint8_t numRows)
{
    std::vector<Note> v(length * numRows, Note(Note::Space));

    const uint8_t
            rows = std::min(numRows, p_numRows_),
            cols = std::min(length, p_numNotes_);
    for (uint8_t y = 0; y < rows; ++y)
    for (uint8_t x = 0; x < cols; ++x)
        v[y * length + x] = note(x, y);

    p_data_.swap(v);
}

void Bar::setNote(uint8_t column, uint8_t row, const Note &n)
{
    SONOT_ASSERT_LT(column, length(), "in Bar::setNote ");
    SONOT_ASSERT_LT(row, numRows(), "in Bar::setNote");
    p_data_[row * p_numNotes_ + column] = n;
}

bool Bar::isAnnotated() const
{
    for (const Note& n : p_data_)
        if (n.isAnnotated())
            return true;
    return false;
}

QJsonObject Bar::toJson() const
{
    JsonHelper json("Bar");

    QJsonObject o;
    o.insert("len", QJsonValue(p_numNotes_));
    o.insert("rows", QJsonValue(p_numRows_));

    std::vector<int> v;
    for (const Note& n : p_data_)
        v.push_back(n.value());
    o.insert("notes", json.toArray(v));

    if (isAnnotated())
    {
        QJsonObject ann;
        for (uint8_t row=0; row<p_numRows_; ++row)
        for (uint8_t col=0; col<p_numNotes_; ++col)
        {
            const Note& n = p_data_[row*p_numNotes_+col];
            if (!n.isAnnotated())
                continue;
            ann.insert(QString::number(row*p_numNotes_+col), n.annotation());
        }
        o.insert("text", ann);
    }

    return o;
}

void Bar::fromJson(const QJsonObject& o)
{
    JsonHelper json("Bar");
    const int
            len = json.expectChild<int>(o, "len"),
            rows = json.expectChild<int>(o, "rows");

    QJsonArray jnotes = json.expectArray(json.expectChildValue(o, "notes"));
    if (jnotes.size() != len*rows)
        SONOT_IO_ERROR("Mismatching note data size " << jnotes.size()
                       << ", expected " << len << "x" << rows);
    std::vector<Note> notes;
    for (int i=0; i<jnotes.size(); ++i)
    {
        int8_t n = jnotes[i].toInt(Note::Invalid);
        notes.push_back(Note(n));
    }

    if (o.contains("text"))
    {
        QJsonObject ann = json.expectObject(o.value("text"));
        QStringList keys = ann.keys();
        for (const QString& key : keys)
        {
            bool ok;
            int k = key.toInt(&ok);
            if (!ok)
                SONOT_IO_ERROR("Expected integer key in Bar object, got '"
                               << key << "'");
            if (k < 0 || size_t(k) >= notes.size())
                SONOT_IO_ERROR("Integer key out of range in Bar object, got "
                               << k << ", expected [0," << notes.size() << ")");
            QString text = json.expectChild<QString>(ann, key);
            notes[k].setAnnotation(text);
        }
    }

    p_numNotes_ = len;
    p_numRows_ = rows;
    p_data_.swap(notes);
}

} // namespace Sonot
