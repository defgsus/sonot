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
#include <QString>
#include <QDebug>

#include <cmath>
#include <cstddef>

#include "NoteFreq.h"

namespace Sonot {

template <typename F>
NoteFreq<F>::NoteFreq(F notes, F freq)
    : p_notes_per_octave    (notes),
      p_base_freq           (freq),
      p_num                 (0),
      p_denom               (0)
{
    recalc_();
}

template <typename F>
NoteFreq<F>::NoteFreq(int notes, F freq, int nom, int denom)
    : p_notes_per_octave    (notes),
      p_base_freq           (freq),
      p_num                 (nom),
      p_denom               (denom),
      p_table               (notes)
{
    recalc_();
}

template <typename F>
void NoteFreq<F>::setNotesPerOctave(F n)
{
    p_notes_per_octave = n;
    p_num = p_denom = 0;
    recalc_();
}

template <typename F>
void NoteFreq<F>::setNotesPerOctave(int n)
{
    p_notes_per_octave = n;
    p_table.resize(n);
    recalc_();
}

template <typename F>
void NoteFreq<F>::setPythagorean(int n, int d)
{
    p_num = n;
    p_denom = d;
    recalc_();
}

template <typename F>
void NoteFreq<F>::recalc_()
{
    // equal tuning
    if (p_num == 0 || p_denom == 0)
    {
        p_notes_per_octave = std::max(F(0.0000001), p_notes_per_octave);
        p_oct_pow = std::pow(F(2), F(1) / p_notes_per_octave);
        return;
    }

    // init table
    p_notes_per_octave = std::max(F(1), p_notes_per_octave);
    if (p_table.empty())
        p_table.resize(1);
    for (auto& f : p_table)
        f = 1.;


    // -- "pythagorean tuning" --

    int offset = 0;
    offset = 3; // kluge
    double freq = F(1);
    int k = offset;
    for (int i=0; i<p_table.size(); ++i)
    {
        if (size_t(k) < p_table.size())
            p_table[size_t(k)] = k<offset? freq / 2: freq;
        freq = freq * F(p_num) / F(p_denom);
        while (freq >= F(2))
            freq /= F(2);
        k = (k - 5 + (int)p_table.size()) % p_table.size();
    }
    QString str;
    F first = p_table[0];
    for (auto& f : p_table)
        f /= first,
        str += QString("%1 ").arg(f);
    qDebug() << str;
}

template <typename F>
F NoteFreq<F>::frequency(int note) const
{
    if (p_num == 0 || p_denom == 0)
    {
        return p_base_freq * std::pow(p_oct_pow, note);
    }

    note = std::max(0, note);
    int oct = note / p_table.size(),
        idx = note % p_table.size();
    return p_base_freq * p_table[idx] * (1 << oct);
}

template <typename F>
F NoteFreq<F>::octave(F f) const
{
    return (std::log(f) - std::log(p_base_freq)) / std::log(F(2));
}


template class NoteFreq<float>;
template class NoteFreq<double>;

} // namespace Sonot

