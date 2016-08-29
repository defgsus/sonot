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

#ifndef SONOTSRC_BAR_H
#define SONOTSRC_BAR_H

#include <vector>

#include "Note.h"


namespace Sonot {

/** A bar/messure of notes. */
class Bar
{
public:
    Bar(int8_t length = 0, int8_t numRows = 0);

    // --- getter ---

    int8_t length() const { return p_numNotes_; }
    int8_t numRows() const { return p_numRows_; }

    Note note(int8_t column, int8_t row) const;

    // --- setter ---

    void resize(int8_t length, int8_t numRows);
    void setLength(int8_t length) { resize(length, p_numRows_); }
    void setNumRows(int8_t rows) { resize(p_numNotes_, rows); }

    void setNote(int8_t column, int8_t row, const Note& n);

private:
    int8_t p_numNotes_, p_numRows_;
    std::vector<Note> p_data_;
};

} // namespace Sonot

#endif // SONOTSRC_BAR_H
