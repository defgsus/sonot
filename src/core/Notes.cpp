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

bool Notes::containsNotes() const
{
    for (const Note& n : p_data_)
        if (n.isNote())
            return true;
    return false;
}

const Note& Notes::note(size_t column) const
{
    QPROPS_ASSERT_LT(column, length(), "in Bar::note()");

    return p_data_[column];
}

double Notes::columnTime(double column) const
{
    //double t = column*3.14159265;
    //column += .2*std::sin(t/2);
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
    else
        p_data_.insert(p_data_.begin() + column, n);
}

void Notes::removeNote(size_t column)
{
    QPROPS_ASSERT_LT(column, length(), "in Bar::removeNote");
    p_data_.erase(p_data_.begin() + column);
}

void Notes::transpose(int8_t noteStep, bool wholeNotes)
{
    for (Note& n : p_data_)
        if (n.isNote())
            n.transpose(noteStep, wholeNotes);
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

bool Notes::insertNotes(const Note &n, size_t steps, size_t times, size_t offset)
{
    if (steps == 0 || times == 0)
        return false;
    size_t num = length() / steps;
    size_t oldLen = length();
    for (size_t i=offset, k=0; i<=length() && k<num; i += steps, ++k)
    {
        for (size_t j=0; j<times; ++j)
        {
            insertNote(i, n);
            ++i;
        }
    }
    return oldLen != length();
}

bool Notes::deleteNotes(size_t steps, size_t times, size_t offset)
{
    if (steps == 0 || times == 0)
        return false;
    size_t num = length() / steps;
    size_t oldLen = length();
    for (size_t i=offset, k=0; i<length() && k<num; i += steps, ++k)
    {
        for (size_t j=0; j<times && i<length(); ++j)
        {
            removeNote(i);
        }
    }
    return oldLen != length();
}

QString Notes::toString() const
{
    QString s;
    for (const Note& n : p_data_)
    {
        if (!s.isEmpty())
            s += " ";
        s += n.toString();
    }
    return s;
}


QJsonObject Notes::toJson() const
{
    QProps::JsonInterfaceHelper json("Notes");

    QJsonObject o;

    if (length() > 0)
    {
        std::vector<int> v;
        for (const Note& n : p_data_)
        {
            v.push_back(n.note());
            v.push_back(n.octave());
            v.push_back(n.accidental());
        }
        o.insert("noa", json.toArray(v));
    }

    return o;
}

void Notes::fromJson(const QJsonObject& o)
{
    QProps::JsonInterfaceHelper json("Notes");

    std::vector<Note> notes;

    // old version
    if (o.contains("notes"))
    {
        QJsonArray jnotes = json.expectArray(json.expectChildValue(o, "notes"));
        for (int i=0; i<jnotes.size(); ++i)
        {
#if QT_VERSION >= 0x050200
            int n = jnotes[i].toInt(Note::Invalid);
#else
            int n = jnotes[i].isDouble() ? (int)jnotes[i].toDouble() : (int)Note::Invalid;
#endif
            if (n == Note::Invalid)
                notes.push_back(Note());
            else
                notes.push_back(Note::fromValue(n));
        }
    }
    else if (o.contains("noa"))
    {
        QJsonArray jnotes = json.expectArray(json.expectChildValue(o, "noa"));
        for (int i=0; i<jnotes.size()/3; ++i)
        {
#if QT_VERSION >= 0x050200
            int n = jnotes[i*3].toInt(Note::Invalid);
#else
            int n = jnotes[i*3].isDouble() ? (int)jnotes[i*3].toDouble() : (int)Note::Invalid;
#endif
            if (n == Note::Invalid)
                notes.push_back(Note());
            else
                notes.push_back(Note(Note::Name(n),
                     #if QT_VERSION >= 0x050200
                                 jnotes[i*3+1].toInt(3),
                                 jnotes[i*3+2].toInt(0)));
                     #else
                                 jnotes[i*3+1].isDouble() ? (int)jnotes[i*3+1].toDouble() : (int)3,
                                 jnotes[i*3+2].isDouble() ? (int)jnotes[i*3+2].toDouble() : (int)0 ));
                     #endif
        }
    }

    p_data_.swap(notes);
}

} // namespace Sonot
