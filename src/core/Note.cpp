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

#include "Note.h"

namespace Sonot {

Note::Note(Special s)
    : p_value_   (s)
{

}

Note::Note(int8_t note)
    : p_value_   (note)
{

}

Note::Note(Name noteName, int8_t octave)
    : p_value_   ((noteName % 12) + octave * 12)
{

}

bool Note::operator == (const Note& rhs) const
{
    return p_value_ == rhs.p_value_
        && p_annotation_ == rhs.p_annotation_;
}

int8_t Note::octave() const
{
    return p_value_ < 0 ? p_value_ / 12 : 0;
}

Note::Name Note::noteName() const
{
    return p_value_ < 0 ? C : Name(p_value_ % 12);
}

QString Note::toNoteString() const
{
    if (p_value_ == Invalid)
        return QString();
    if (p_value_ == Rest)
        return "p";
    if (p_value_ == Space)
        return " ";

    QString n;
    switch (noteName())
    {
        case C:  n = "C-"; break;
        case Cx: n = "C#"; break;
        case D:  n = "D-"; break;
        case Dx: n = "D#"; break;
        case E:  n = "E-"; break;
        case F:  n = "F-"; break;
        case Fx: n = "F#"; break;
        case G:  n = "G-"; break;
        case Gx: n = "G#"; break;
        case A:  n = "A-"; break;
        case Ax: n = "A#"; break;
        case B:  n = "B-"; break;
    }
    n += QString("%1").arg(octave());
    return n;
}



Note& Note::setOctave(int8_t o)
{
    if (p_value_ >= 0)
        p_value_ = o * 12 + (p_value_ % 12);
    return *this;
}

Note& Note::setNoteName(Name n)
{
    p_value_ = n + octave() * 12;
    return *this;
}

} // namespace Sonot
