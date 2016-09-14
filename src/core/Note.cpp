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

Note::Note(NameCrossing noteName, int8_t octave)
    : p_value_   (std::max(-1, noteName % 12)
                               + 12 * ((noteName == Ces ? -1 : 1) + octave) )
{

}

Note::Note(const char *str)
    : p_value_  (valueFromString(str))
{

}

Note::Note(const QString &str)
    : p_value_  (valueFromString(str))
{

}

bool Note::operator == (const Note& rhs) const
{
    return p_value_ == rhs.p_value_
        && p_annotation_ == rhs.p_annotation_;
}

int8_t Note::octave() const
{
    return p_value_ < 0 ? 0 : p_value_ / 12;
}

Note::Name Note::noteName() const
{
    return p_value_ < 0 ? C : Name(p_value_ % 12);
}

int8_t Note::valueFromString(const char *str)
{
    return valueFromString(QString(str));
}

int8_t Note::valueFromString(const QString &s)
{
    if (s.isEmpty())
        return Invalid;

    QString str = s.toLower();

    if (str.startsWith(" "))
        return Space;
    if (str.startsWith("-") || str.startsWith("p") || str.startsWith("r"))
        return Rest;

    int8_t val = Invalid;
    int8_t oct = 3;

    // spanish note names
    if (str.at(0).isDigit())
    {
        switch ((short)str.at(0).unicode() - short('0'))
        {
            case 1: val = F; break;
            case 2: val = G; break;
            case 3: val = A; break;
            case 4: val = B; break;
            case 5: val = C; ++oct; break;
            case 6: val = D; ++oct; break;
            case 7: val = E; ++oct; break;
        }
    }
    else
    if (str.at(0).isLetter())
    {
        switch ((short)str.at(0).unicode() - short('a'))
        {
            case 'a': val = A; break;
            case 'h':
            case 'b': val = B; break;
            case 'c': val = C; break;
            case 'd': val = D; break;
            case 'e': val = E; break;
            case 'f': val = F; break;
            case 'g': val = G; break;
        }
    }
    else
        return Invalid;

    // flat or sharp ?
    if (str.size() > 1)
    {
        QString add = str.mid(1);
        if (add.startsWith("#") || add.startsWith("is") || add.startsWith("x"))
            ++val;
        else if (add.startsWith("b")
                 || add.startsWith("es") || add.startsWith("s"))
            --val;
    }

    // octave digit?
    int i=1;
    while (i < str.size() && !str.at(i).isDigit())
        ++i;
    if (i < str.size())
    {
        oct = (short)str.at(i).unicode() - short('0');
    }

    // octave subscript?
    oct += str.count(QChar('\''));
    oct -= str.count(QChar(','));

    val += oct * 12;

    return val >= 0 ? val : int8_t(Invalid);
}

QString Note::to3String() const
{
    if (p_value_ == Invalid)
        return QString();
    if (p_value_ == Rest)
        return " p ";
    if (p_value_ == Space)
        return " . ";

    QString n;
    switch (noteName())
    {
        case C:  n = "C-"; break;
        case Cis: n = "C#"; break;
        case D:  n = "D-"; break;
        case Dis: n = "D#"; break;
        case E:  n = "E-"; break;
        case F:  n = "F-"; break;
        case Fis: n = "F#"; break;
        case G:  n = "G-"; break;
        case Gis: n = "G#"; break;
        case A:  n = "A-"; break;
        case Ais: n = "A#"; break;
        case B:  n = "B-"; break;
    }
    n += QString("%1").arg(octave());
    return n;
}

QString Note::toSpanishString() const
{
    if (p_value_ == Invalid)
        return QString();
    if (p_value_ == Rest)
        return "p";
    if (p_value_ == Space)
        return ".";

    QString n;
    switch (noteName())
    {
        case C:  n =  "5"; break;
        case Cis: n = "6b"; break;
        case D:  n =  "6"; break;
        case Dis: n = "7b"; break;
        case E:  n =  "7"; break;
        case F:  n =  "1"; break;
        case Fis: n = "2b"; break;
        case G:  n =  "2"; break;
        case Gis: n = "3b"; break;
        case A:  n =  "3"; break;
        case Ais: n = "4b"; break;
        case B:  n =  "4"; break;
    }
    if (octave() > 3)
        n += QString("'").repeated(octave()-3);
    else if (octave() < 3)
        n += QString(",").repeated(3-octave());
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

void Note::transpose(int8_t noteStep)
{
    if (isNote())
    {
        int value = int(p_value_) + int(noteStep);
        if (value < 0 || value > 127)
            value = Space;
        p_value_ = value;
    }
}

} // namespace Sonot
