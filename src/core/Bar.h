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
#include "io/JsonInterface.h"

namespace Sonot {

/** A bar/messure of notes. */
class Bar : public JsonInterface
{
public:
    Bar(size_t length = 0, size_t numRows = 0);

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // --- getter ---

    size_t length() const { return p_numNotes_; }
    size_t numRows() const { return p_numRows_; }

    /** Returns the note at given column and row.
        @warning No range checking! */
    const Note& note(size_t column, size_t row) const;

    /** Is any of the Notes annotated? */
    bool isAnnotated() const;

    // --- setter ---

    void resize(size_t length, size_t numRows);
    void setLength(size_t length) { resize(length, p_numRows_); }
    void setNumRows(size_t rows) { resize(p_numNotes_, rows); }

    /** Stores the Note at given column and row.
        @warning No range checking! */
    void setNote(size_t column, size_t row, const Note& n);

private:
    size_t p_numNotes_, p_numRows_;
    std::vector<Note> p_data_;
};

} // namespace Sonot

#endif // SONOTSRC_BAR_H
