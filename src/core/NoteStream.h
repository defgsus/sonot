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

#include <QtCore>
#include <QString>

#include "QProps/JsonInterface.h"
#include "QProps/Properties.h"

#include "KeySignature.h"
#include "Notes.h"
#include "Bar.h"

namespace Sonot {

/** Collection of Bars */
class NoteStream : public QProps::JsonInterface
{
    Q_DECLARE_TR_FUNCTIONS(NoteStream);
public:
    NoteStream();

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // --- factory ---

    /** Default empty bar for this stream.
        If @p len == 0, the default length will be used. */
    Notes createDefaultNotes(size_t len = 0) const;

    /** Default silent bar block for this stream.
        If @p len == 0, the default length will be used.
        The bar will contain this number of silent notes.
        The number of rows will be equal
        to the number of rows in this stream, or at least 1. */
    Bar createDefaultBar(size_t len = 0) const;

    /** Creates a default NoteStream.
        The stream will contain at least one Bar created with
        createDefaultBarRows().
        The properties of the current stream are copied. */
    NoteStream createDefaultStream(size_t numBars = 0, size_t barLen = 0) const;

    // --- getter ---

    const QProps::Properties& props() const;

    KeySignature keySignature() const;

    bool isEmpty() const { return p_data_.empty(); }
    bool isPauseOnEnd() const
        { return props().get("pause-on-end").toBool(); }
    /** Number of Bars in this collection */
    size_t numBars() const { return p_data_.size(); }

    /** Returns the number of rows */
    size_t numRows() const;

    /** Returns the accumulated number of notes */
    size_t numNotes() const;

    /** Returns the maximum number of notes in Bar */
    size_t numNotes(size_t barIdx) const;

    /** The beats-per-minute for the given bar.
        Supposes a bar is typically 4 quarter notes. */
    double beatsPerMinute(size_t barIdx) const;
    /** The length of the given bar in seconds. */
    double barLengthSeconds(size_t barIdx) const;

    /** Read reference to @p idx'th Bar */
    const Bar& bar(size_t barIdx) const;

    /** Read reference to @p row'th Notes in the @p idx'th Bar */
    const Notes& notes(size_t barIdx, size_t row) const;

    /** Returns Note from Bar */
    const Note& note(size_t barIdx, size_t row, size_t column) const;

    /** Returns a copy of the bars at barIdx */
    QList<Notes> getRows(size_t barIdx) const;

    /** Returns a multi-line ascii string representing the data */
    QString toTabString() const;

    /** Returns multi-line info about sizes */
    QString toInfoString() const;

    bool operator == (const NoteStream& rhs) const;
    bool operator != (const NoteStream& rhs) const { return !(*this == rhs); }

    // --- setter ---

    void setProperties(const QProps::Properties& props);

    void clear() { p_data_.clear(); }

    /** Write-reference to @p idx'th Bar */
    Bar& bar(size_t barIdx);

    /** Overwrite specific Note from Bar */
    void setNote(size_t barIdx, size_t row, size_t column, const Note& n);

    /** Change number of voices. */
    void setNumRows(size_t numRows);

    void removeBar(size_t idx);
    /** Remove @p count Bars starting at @p idx.
        If @p count < 0, all Bars to the end will be removed */
    void removeBars(size_t idx, int64_t count = -1);

    /** Inserts a Bar before given index.
        If @p idx is >= numBars(), the Bar will be appended. */
    void insertBar(size_t idx, const Notes& row);
    void appendBar(const Notes& row) { insertBar(numBars(), row); }

    void insertBar(size_t idx, const Bar& bar);
    void appendBar(const Bar& bar) { insertBar(numBars(), bar); }

    /** Inserts a row of Bars before the given row. */
    void insertRow(size_t row);
    void removeRow(size_t row);

private:
    std::vector<Bar> p_data_;
    QProps::Properties p_props_;
};

} // namespace Sonot

#endif // SONOTSRC_NOTESTREAM_H
