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

#include <algorithm>

#include "Bar.h"
#include "io/error.h"

namespace Sonot {

Bar::Bar(int8_t length, int8_t numRows)
    : p_numNotes_   (length)
    , p_numRows_    (numRows)
    , p_data_       (p_numNotes_ * p_numRows_, Note(Note::Space))
{

}

Note Bar::note(int8_t column, int8_t row) const
{
    return column < 0 || column >= p_numNotes_
        || row < 0 || row >= p_numRows_
            ? Note(Note::Invalid)
            : p_data_[row * p_numNotes_ + column];
}

void Bar::resize(int8_t length, int8_t numRows)
{
    std::vector<Note> v(length * numRows, Note(Note::Space));

    const int8_t
            rows = std::min(numRows, p_numRows_),
            cols = std::min(length, p_numNotes_);
    for (int8_t y = 0; y < rows; ++y)
    for (int8_t x = 0; x < cols; ++x)
        v[y * length + x] = note(x, y);

    p_data_.swap(v);
}

void Bar::setNote(int8_t column, int8_t row, const Note &n)
{
    SONOT_ASSERT_LT(column, length(), "in Bar::setNote ");
    SONOT_ASSERT_LT(row, numRows(), "in Bar::setNote");
    p_data_[row * p_numNotes_ + column] = n;
}


} // namespace Sonot
