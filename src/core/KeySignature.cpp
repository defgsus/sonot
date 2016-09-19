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

#include "QProps/JsonInterfaceHelper.h"
#include "QProps/error.h"

#include "KeySignature.h"
#include "Note.h"

namespace Sonot {


KeySignature::KeySignature()
{

}

QJsonObject KeySignature::toJson() const
{
    QJsonObject o;
    o.insert("key", toString());
    return o;
}

void KeySignature::fromJson(const QJsonObject& o)
{
    QProps::JsonInterfaceHelper json("KeySignature");

    KeySignature tmp;
    tmp.fromString( json.expectChild<QString>(o, "key") );

    p_map.swap(tmp.p_map);
}

// -- getter --

bool KeySignature::operator == (const KeySignature& o) const
{
    return p_map == o.p_map;
}

int8_t KeySignature::transform(int8_t note) const
{
    auto i = p_map.find(note % 12);
    if (i != p_map.end())
        return std::max(0, note + i->second);
    return note;
}

Note KeySignature::transform(const Note &note) const
{
    Note n(note);
    n.setValue( transform(n.value()) );
    return n;
}

QString KeySignature::toString() const
{
    QString s;
    for (auto i = p_map.begin(); i != p_map.end(); ++i)
    {
        Note n(i->first);
        QString nn = n.toShortAlphaNumString();
        QPROPS_ASSERT(!nn.isEmpty(), "in KeySignature::toString()");
        if (!s.isEmpty())
            s += " ";
        s += nn[0] + QString("b").repeated(-i->second)
                   + QString("#").repeated( i->second);
    }
    return s;
}

// -- setter --

void KeySignature::fromString(const QString& s)
{
    clear();
    auto keys = s.split(' ', QString::SkipEmptyParts);
    for (const QString& key : keys)
    {
        QPROPS_ASSERT(!key.isEmpty(), "in KeySignature::fromString()");
        Note n(key[0]);
        if (!n.isNote())
            continue;
        setKey(n.value()%12, -key.count('b') + key.count('#'));
    }
}

void KeySignature::setKey(int8_t note, int8_t shift)
{
    p_map.insert(std::make_pair(int8_t(note%12), shift));
}


} // namespace Sonot
