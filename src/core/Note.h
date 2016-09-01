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

#ifndef SONOTSRC_NOTE_H
#define SONOTSRC_NOTE_H

#include <cstdint>
#include <QString>

namespace Sonot {

/** A single note value */
class Note
{
public:

    /** Musical notes
        @warning Order/values must not change for file persistence */
    enum Name
    {
        C, Cx, D, Dx, E, F, Fx, G, Gx, A, Ax, B
    };

    /** Special notes
        @warning Order/values must not change for file persistence */
    enum Special
    {
        Invalid = -1,
        Space = -2,
        Rest = -3
    };

    explicit Note(Special s = Invalid);
    explicit Note(Name noteName, int8_t octave);
    explicit Note(int8_t value);

    // --- getter ---

    /** Pure value, this is either a note if >= 0
        or a Special enum if < 0 */
    int8_t value() const { return p_value_; }

    int8_t octave() const;
    Name noteName() const;

    /** Returns a string in the format 'C#3' */
    QString toNoteString() const;

    bool isValid() const { return p_value_ != Invalid; }
    bool isNote() const { return p_value_ >= 0; }
    bool isAnnotated() const { return !p_annotation_.isEmpty(); }

    QString annotation() const { return p_annotation_; }

    bool operator == (const Note& rhs) const;
    bool operator != (const Note& rhs) const { return !(*this == rhs); }

    // --- setter ---

    Note& setValue(int8_t n) { p_value_ = n; return *this; }
    Note& setOctave(int8_t o);
    Note& setNoteName(Name n);

    Note& setAnnotation(const QString& a) { p_annotation_ = a; return *this; }

private:
    int8_t p_value_;
    QString p_annotation_;
};


} // namespace Sonot

#endif // SONOTSRC_NOTE_H

