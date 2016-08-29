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

    enum Name
    {
        C, Cx, D, Dx, E, F, Fx, G, Gx, A, Ax, B
    };

    enum Special
    {
        Invalid = -1,
        Space = -2,
        Rest = -3
    };

    explicit Note(Special s = Invalid);
    explicit Note(int8_t note);
    explicit Note(Name noteName, int8_t octave);

    // --- getter ---

    int8_t octave() const;
    int8_t note() const { return p_note_; }
    Name noteName() const;
    QString toString() const;

    bool isValid() const { return p_note_ != Invalid; }
    bool isNote() const { return p_note_ >= 0; }

    // --- setter ---

    void setOctave(int8_t o);
    void setNote(int8_t n) { p_note_ = n; }
    void setNoteName(Name n);

private:
    int8_t p_note_;

};


} // namespace Sonot

#endif // SONOTSRC_NOTE_H

