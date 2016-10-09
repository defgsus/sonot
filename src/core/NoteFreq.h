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

#include <vector>

namespace Sonot {

template <typename F>
class NoteFreq
{
public:

    /** Base C */
    static constexpr F defaultBaseFrequency() { return F(16.35158); }

    NoteFreq(F notesPerOctave = F(12),
             F baseFrequency = defaultBaseFrequency());

    NoteFreq(int notesPerOctave,
             F baseFrequency = defaultBaseFrequency(),
             int pythagoreanNum = 3,
             int pythagoreanDenom = 2);

    // ------- getter -------

    F notesPerOctave() const { return p_notes_per_octave; }
    F baseFrequency() const { return p_base_freq; }
    int pythagoeanNum() const { return p_num; }
    int pythagoeanDenom() const { return p_denom; }

    // ------- setter -------

    void setNotesPerOctave(F notes);
    void setNotesPerOctave(int notes);
    void setBaseFrequency(F f) { p_base_freq = f; }
    void setPythagorean(int nom, int denom);

    // ----- conversion -----

    /** Returns the frequency for the given note.
        @p note is lower-clamped to 0 */
    F frequency(int note) const;

    /** Returns the octave of the frequency */
    F octave(F freq) const;

private:

    void recalc_();

    F p_notes_per_octave,
      p_base_freq,
      p_oct_pow;
    int p_num,
        p_denom;
    std::vector<F> p_table;
};


} // namespace Sonot

#endif // SONOTSRC_NOTEFREQ_H

