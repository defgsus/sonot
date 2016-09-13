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

#include <QStringList>

#include "Bar.h"
#include "QProps/JsonInterfaceHelper.h"
#include "QProps/error.h"

namespace Sonot {

Bar::Bar(size_t length)
    : p_numNotes_   (length)
    , p_data_       (p_numNotes_, Note(Note::Space))
{

}

bool Bar::operator == (const Bar& rhs) const
{
    return p_numNotes_ == rhs.p_numNotes_
        && p_data_ == rhs.p_data_;
}


const Note& Bar::note(size_t column) const
{
    QPROPS_ASSERT_LT(column, p_numNotes_, "in Bar::note()");

    return p_data_[column];
}

void Bar::resize(size_t length)
{
    std::vector<Note> v(length, Note(Note::Space));

    const size_t cols = std::min(length, p_numNotes_);
    for (size_t x = 0; x < cols; ++x)
        v[x] = note(x);

    p_data_.swap(v);
    p_numNotes_ = length;
}

void Bar::setNote(size_t column, const Note &n)
{
    QPROPS_ASSERT_LT(column, length(), "in Bar::setNote ");
    p_data_[column] = n;
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
    QProps::JsonInterfaceHelper json("Bar");

    QJsonObject o;
    o.insert("len", QJsonValue((qint64)p_numNotes_));

    std::vector<int> v;
    for (const Note& n : p_data_)
        v.push_back(n.value());
    o.insert("notes", json.toArray(v));

    if (isAnnotated())
    {
        QJsonObject ann;
        for (size_t col=0; col<p_numNotes_; ++col)
        {
            const Note& n = note(col);
            if (!n.isAnnotated())
                continue;
            ann.insert(QString::number(col), n.annotation());
        }
        o.insert("text", ann);
    }

    return o;
}

void Bar::fromJson(const QJsonObject& o)
{
    QProps::JsonInterfaceHelper json("Bar");
    const int len = json.expectChild<int>(o, "len");

    QJsonArray jnotes = json.expectArray(json.expectChildValue(o, "notes"));
    if (jnotes.size() != len)
        QPROPS_IO_ERROR("Mismatching note data size " << jnotes.size()
                       << ", expected " << len);
    std::vector<Note> notes;
    for (int i=0; i<jnotes.size(); ++i)
    {
        size_t n = jnotes[i].toInt(Note::Invalid);
        notes.push_back(Note(n));
    }

    if (o.contains("text"))
    {
        json.beginContext("read Bar annotation");

        QJsonObject ann = json.expectObject(o.value("text"));
        QStringList keys = ann.keys();
        for (const QString& key : keys)
        {
            bool ok;
            int k = key.toInt(&ok);
            if (!ok)
                QPROPS_IO_ERROR("Expected integer key in Bar object, got '"
                               << key << "'");
            if (k < 0 || size_t(k) >= notes.size())
                QPROPS_IO_ERROR("Integer key out of range in Bar object, got "
                               << k << ", expected [0," << notes.size() << ")");
            QString text = json.expectChild<QString>(ann, key);
            notes[k].setAnnotation(text);
        }

        json.endContext();
    }

    p_numNotes_ = len;
    p_data_.swap(notes);
}

} // namespace Sonot
