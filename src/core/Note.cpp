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

#include <QDebug>
#include "Note.h"

namespace Sonot {

namespace
{
    static const int8_t name2Val[] =
       // c c# d d# e f f# g g# a a# b
        { 0,   2,   4,5,   7,   9,   11 };
    static const int8_t val2NameAcc[] =
        { 0, 0,    // c
          0, 1,    // c#
          1, 0,    // d
          1, 1,    // d#
          2, 0,    // e
          3, 0,    // f
          3, 1,    // f#
          4, 0,    // g
          4, 1,    // g#
          5, 0,    // a
          5, 1,    // a#
          6, 0 };  // b
}


Note::Note(Special s)
    : p_note_   (s)
    , p_oct_    (0)
    , p_acc_    (0)
{

}

Note::Note(Name note, int8_t octave, int8_t accidental)
    : p_note_   (note)
    , p_oct_    (octave)
    , p_acc_    (accidental)
{

}

Note::Note(const QString &str)
{
    *this = fromString(str);
}

bool Note::operator == (const Note& rhs) const
{
    bool sameNote = p_note_ == rhs.p_note_;
    if (sameNote && p_note_ < 0)
        return true;
    return sameNote
        && p_oct_ == rhs.p_oct_
        && p_acc_ == rhs.p_acc_;
}

int8_t Note::value() const
{
    return isNote() ? name2Val[p_note_] + p_acc_ + p_oct_ * 12
                    : p_note_;
}

int8_t Note::octaveSpanish() const
{
    return p_note_ < 5 ? octave() + 1 : octave();
}


Note Note::fromString(const QString& s)
{
    if (s.isEmpty())
        return Note();

    QString str = s.toLower();

    if (str.startsWith(" "))
        return Note(Space);
    if (str.startsWith("-") || str.startsWith("p") || str.startsWith("r"))
        return Note(Rest);

    Name note = C;
    int oct = 3, acc = 0;

    // spanish note names
    if (str.at(0).isDigit())
    {
        switch ((short)str.at(0).unicode() - short('0'))
        {
            case 1: note = F; break;
            case 2: note = G; break;
            case 3: note = A; break;
            case 4: note = B; break;
            case 5: note = C; ++oct; break;
            case 6: note = D; ++oct; break;
            case 7: note = E; ++oct; break;
        }
    }
    else
    if (str.at(0).isLetter())
    {
        switch ((short)str.at(0).unicode())
        {
            case 'a': note = A; break;
            case 'h':
            case 'b': note = B; break;
            case 'c': note = C; break;
            case 'd': note = D; break;
            case 'e': note = E; break;
            case 'f': note = F; break;
            case 'g': note = G; break;
        }
    }
    else
        return Note();

    // flat or sharp ?
    if (str.size() > 1)
    {
        acc += -str.mid(1).count('b') + str.count('#');
    }

    // octave digit?
    int i=1;
    while (i < str.size() && !str.at(i).isDigit())
        ++i;
    if (i < str.size())
    {
        oct = str.mid(i).toInt();
    }

    // octave subscript?
    oct += str.count(QChar('\''));
    oct -= str.count(QChar(','));

    return Note(note, oct, acc);
}

void Note::setFromValue(int8_t v)
{
    if (v < 0)
    {
        p_note_ = v;
        p_acc_ = 0;
        p_oct_ = 0;
        return;
    }
    size_t idx = (v % 12) * 2;
    p_note_ = val2NameAcc[idx];
    p_oct_ = v / 12;
    p_acc_ = val2NameAcc[idx+1];
}

Note Note::fromValue(int8_t v)
{
    Note n;
    n.setFromValue(v);
    return n;
}

const char* Note::noteName(Name n)
{
    switch (n)
    {
        case C: return "C";
        case D: return "D";
        case E: return "E";
        case F: return "F";
        case G: return "G";
        case A: return "A";
        case B: return "B";
        default: return "X";
    }
}

const char* Note::noteName() const { return noteName(note()); }

QString Note::toString() const
{
    if (p_note_ == Invalid)
        return QString();
    if (p_note_ == Rest)
        return "p";
    if (p_note_ == Space)
        return ".";

    QString n = noteName(Name(p_note_));
    if (p_acc_ > 0)
        n += QString("#").repeated(p_acc_);
    else if (p_acc_ < 0)
        n += QString("b").repeated(-p_acc_);
    n += QString("%1").arg(octave());
    return n;
}

QString Note::toNoaString() const
{
    return QString("%1,%2,%3")
            .arg(note()).arg(octave()).arg(accidental());
}

QString Note::to3String() const
{
    if (p_note_ == Invalid)
        return QString();
    if (p_note_ == Rest)
        return " p ";
    if (p_note_ == Space)
        return " . ";

    QString n = noteName(Name(p_note_));
    if (p_acc_ == 0)
        n += "-";
    else if (p_acc_ > 0)
        n += "#";
    else
        n += "b";
    n += QString("%1").arg(octave());
    return n;
}

QString Note::toSpanishString() const
{
    if (p_note_ == Invalid)
        return QString();
    if (p_note_ == Rest)
        return "p";
    if (p_note_ == Space)
        return ".";

    QString n;
    int oct = octave();
    switch (note())
    {
        case C:   n = "5";  --oct; break;
        case D:   n = "6";  --oct; break;
        case E:   n = "7";  --oct; break;
        case F:   n = "1";  break;
        case G:   n = "2";  break;
        case A:   n = "3";  break;
        case B:   n = "4";  break;
    }
    if (p_acc_ > 0)
        n += QString("#").repeated(p_acc_);
    else if (p_acc_ < 0)
        n += QString("b").repeated(-p_acc_);
    if (oct > 3)
        n += QString("'").repeated(oct-3);
    else if (oct < 3)
        n += QString(",").repeated(3-oct);
    return n;
}

QString Note::toShortAlphaNumString() const
{
    QString s = to3String().remove("-");
    if (s.endsWith("3"))
        s.chop(1);
    return s;
}

QString Note::accidentalString() const
{
    if (p_acc_ > 0)
        return QString("#").repeated(p_acc_);
    else if (p_acc_ < 0)
        return QString("b").repeated(-p_acc_);
    else
        return QString();
}

void Note::transpose(int8_t step, bool wholeSteps)
{
    if (step != 0 && isNote())
    {
        if (!wholeSteps)
        {
            setFromValue(std::max(0,std::min(127,
                         int(value()) + int(step) )));
        }
        else
        {
            int note = p_note_ + step;
            int octChange = note > 0
                    ? note / 7
                    : note / 7 - 1;
            p_note_ = (note+700) % 7;
            p_oct_ = std::max(0, octChange + p_oct_);
        }
    }
}

} // namespace Sonot
