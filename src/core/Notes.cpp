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

#include "Notes.h"
#include "QProps/JsonInterfaceHelper.h"
#include "QProps/error.h"

namespace Sonot {

Notes::Notes(size_t length)
    : p_data_       (length, Note(Note::Space))
{

}

bool Notes::operator == (const Notes& rhs) const
{
    return p_data_ == rhs.p_data_;
}


const Note& Notes::note(size_t column) const
{
    QPROPS_ASSERT_LT(column, length(), "in Bar::note()");

    return p_data_[column];
}

double Notes::columnTime(double column) const
{
    return length() ? column / length() : 0;
}

void Notes::resize(size_t length)
{
    std::vector<Note> v(length, Note(Note::Space));

    const size_t cols = std::min(length, p_data_.size());
    for (size_t x = 0; x < cols; ++x)
        v[x] = note(x);

    p_data_.swap(v);
}

void Notes::setNote(size_t column, const Note &n)
{
    QPROPS_ASSERT_LT(column, length(), "in Bar::setNote ");
    p_data_[column] = n;
}

void Notes::insertNote(size_t column, const Note &n)
{
    if (column >= length())
        append(n);
    p_data_.insert(p_data_.begin() + column, n);
}

void Notes::removeNote(size_t column)
{
    QPROPS_ASSERT_LT(column, length(), "in Bar::removeNote");
    p_data_.erase(p_data_.begin() + column);
}

void Notes::transpose(int8_t noteStep)
{
    for (Note& n : p_data_)
        if (n.isNote())
            n.transpose(noteStep);
}

Notes& Notes::append(const Note &n)
{
    resize(length() + 1);
    p_data_[length() - 1] = n;
    return *this;
}

Notes& Notes::operator<<(const char* name) { return *this << QString(name); }
Notes& Notes::operator<<(const QString& name)
{
    Note n(name);
    if (n.isValid())
        append(n);
    return *this;
}

bool Notes::isAnnotated() const
{
    for (const Note& n : p_data_)
        if (n.isAnnotated())
            return true;
    return false;
}

QString Notes::toString() const
{
    QString s;
    for (const Note& n : p_data_)
        s += n.to3String() + " ";
    return s;
}



QJsonObject Notes::toJson() const
{
    QProps::JsonInterfaceHelper json("Bar");

    QJsonObject o;

    if (length() > 0)
    {
        std::vector<int> v;
        for (const Note& n : p_data_)
            v.push_back(n.value());
        o.insert("notes", json.toArray(v));
    }

    if (isAnnotated())
    {
        QJsonObject ann;
        for (size_t col=0; col<length(); ++col)
        {
            const Note& n = note(col);
            if (n.isAnnotated())
                ann.insert(QString::number(col), n.annotation());
        }
        o.insert("text", ann);
    }

    return o;
}

void Notes::fromJson(const QJsonObject& o)
{
    QProps::JsonInterfaceHelper json("Bar");

    std::vector<Note> notes;

    if (o.contains("notes"))
    {
        QJsonArray jnotes = json.expectArray(json.expectChildValue(o, "notes"));
        for (int i=0; i<jnotes.size(); ++i)
        {
            size_t n = jnotes[i].toInt(Note::Invalid);
            notes.push_back(Note(n));
        }
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

    p_data_.swap(notes);
}

} // namespace Sonot
