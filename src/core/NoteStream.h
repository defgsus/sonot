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

#ifndef SONOTSRC_NOTESTREAM_H
#define SONOTSRC_NOTESTREAM_H

#include <vector>

#include <QString>

#include "Bar.h"
#include "QProps/JsonInterface.h"

namespace Sonot {

/** Collection of Bars */
class NoteStream : public QProps::JsonInterface
{
public:
    NoteStream();

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // --- getter ---

    const Bar defaultBar() const { return Bar(1); }

    bool isEmpty() const { return p_data_.empty(); }

    /** Number of Bars in this collection */
    size_t numBars() const { return p_data_.size(); }

    /** Returns the number of rows */
    size_t numRows() const;

    /** Returns the accumulated number of notes */
    size_t numNotes() const;

    /** Returns the maximum number of notes in Bar */
    size_t numNotes(size_t barIdx) const;

    /** Read reference to @p idx'th Bar */
    const Bar& bar(size_t barIdx, size_t row) const;

    /** Returns Note from Bar */
    const Note& note(size_t barIdx, size_t row, size_t column) const;

    /** Returns a multi-line ascii string representing the data */
    QString toTabString() const;

    /** Returns multi-line info about sizes */
    QString toInfoString() const;

    bool operator == (const NoteStream& rhs) const;
    bool operator != (const NoteStream& rhs) const { return !(*this == rhs); }

    // --- setter ---

    /** Read/Write reference to @p idx'th Bar */
    Bar& bar(size_t idx, size_t row);

    void clear() { p_data_.clear(); }

    /** Overwrite specific Note from Bar */
    void setNote(size_t barIdx, size_t row, size_t column, const Note& n);

    /** Change number of voices. */
    void setNumRows(size_t numRows);

    void removeBar(size_t idx);
    /** Remove @p count Bars starting at @p idx.
        If @p count < 0, all Bars to the end will be removed */
    void removeBars(size_t idx, int count);

    /** Inserts a Bar before given index.
        If @p idx is >= numBars(), the Bar will be appended. */
    void insertBar(size_t idx, const Bar& row);
    void appendBar(const Bar& row) { insertBar(numBars(), row); }

    void insertBar(size_t idx, const QList<Bar>& rows);
    void appendBar(const QList<Bar>& rows) { insertBar(numBars(), rows); }

private:
    std::vector<std::vector<Bar>> p_data_;
};

} // namespace Sonot

#endif // SONOTSRC_NOTESTREAM_H
