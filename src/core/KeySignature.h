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

#ifndef SONOTSRC_KEYSIGNATURE_H
#define SONOTSRC_KEYSIGNATURE_H

#include <map>

#include "QProps/JsonInterface.h"

namespace Sonot {

class Note;

class KeySignature : public QProps::JsonInterface
{
public:
    KeySignature();

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // -- getter --

    bool operator == (const KeySignature& o) const;
    bool operator != (const KeySignature& o) const { return !(*this == o); }

    bool hasKey(int8_t note) const
        { return p_map.find(note % 12) != p_map.end(); }

    int8_t transform(int8_t note) const;
    Note transform(const Note& note) const;

    QString toString() const;

    // -- setter --

    void clear() { p_map.clear(); }

    void setKey(int8_t note, int8_t shift);

    void fromString(const QString&);

private:
    std::map<int8_t, int8_t> p_map;
};

} // namespace Sonot

#endif // SONOTSRC_KEYSIGNATURE_H
