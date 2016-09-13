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

#ifndef SONOTSRC_NOTEFREQ_H
#define SONOTSRC_NOTEFREQ_H

#include <cmath>

namespace Sonot {

template <typename F>
class NoteFreq
{
public:

    static F defaultBaseFrequency() { return F(16.35155); }

    NoteFreq(F notesPerOctave = F(12), F baseFrequency = F(16.35155));

    // ------- getter -------

    F notesPerOctave() const { return notes_per_octave_; }
    F baseFrequency() const { return base_freq_; }

    // ------- setter -------

    void setNotesPerOctave(F notes);
    void setBaseFrequency(F f) { base_freq_ = f; }

    // ----- conversion -----

    /** Returns the frequency for the given note */
    F frequency(F note) const;

    /** Returns the octave of the frequency */
    F octave(F freq) const;

private:

    void recalc_();

    F notes_per_octave_,
      base_freq_,
      oct_pow_;
};



// ----------- IMPL ---------

template <typename F>
NoteFreq<F>::NoteFreq(F notes, F freq)
    : notes_per_octave_ (notes),
      base_freq_        (freq)
{
    recalc_();
}

template <typename F>
void NoteFreq<F>::setNotesPerOctave(F n)
{
    notes_per_octave_ = std::max(F(0.0000001), n);
    recalc_();
}

template <typename F>
void NoteFreq<F>::recalc_()
{
    oct_pow_ = std::pow(F(2), F(1) / notes_per_octave_);
}

template <typename F>
F NoteFreq<F>::frequency(F note) const
{
    return base_freq_ * std::pow(oct_pow_, note);
}

template <typename F>
F NoteFreq<F>::octave(F f) const
{
    return (std::log(f) - std::log(base_freq_)) / std::log(F(2));
}


} // namespace Sonot

#endif // SONOTSRC_NOTEFREQ_H

