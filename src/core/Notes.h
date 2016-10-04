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

#ifndef SONOTSRC_NOTES_H
#define SONOTSRC_NOTES_H

#include <vector>

#include "QProps/JsonInterface.h"

#include "Note.h"

namespace Sonot {

/** A monophonic single-row bar/messure of notes. */
class Notes : public QProps::JsonInterface
{
public:
    Notes(size_t length = 0);

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // --- getter ---

    bool isEmpty() const { return p_data_.empty(); }

    /** Number of Notes */
    size_t length() const { return p_data_.size(); }

    /** Returns the note at given column.
        @warning No range checking! */
    const Note& note(size_t column) const;
    const Note& operator[](size_t column) const { return note(column); }

    /** Returns the given column's time in range [0,1]
        scaled from length(). */
    double columnTime(double columnWithFraction) const;

    /** Is any of the Notes annotated? */
    bool isAnnotated() const;

    bool operator == (const Notes& rhs) const;
    bool operator != (const Notes& rhs) const { return !(*this == rhs); }

    QString toString() const;

    // --- setter ---

    void resize(size_t length);

    /** Stores the Note at given column and row.
        @warning No range checking! */
    void setNote(size_t column, const Note& n);

    void insertNote(size_t column, const Note& n);
    void removeNote(size_t column);

    Notes& append(const Note& n);

    Notes& operator << (const Note& n) { return append(n); }
    Notes& operator << (const char* name);
    Notes& operator << (const QString& name);

    void transpose(int8_t noteStep);

private:
    std::vector<Note> p_data_;
};

} // namespace Sonot

#endif // SONOTSRC_NOTES_H
