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

#ifndef SONOTSRC_SCORE_H
#define SONOTSRC_SCORE_H

#include <QVariant>

#include "io/JsonInterface.h"

namespace Sonot {

class NoteStream;

/** Collection of NoteStream and custom properties */
class Score : public JsonInterface
{
public:

    // --- types ---

    /** Type to index a specific note.
        @note The Index is tied to the Score instance and
        must only be used during the lifetime of the Score instance! */
    struct Index
    {
        Score* score() const { return p_score; }
        size_t stream() const { return p_stream; }
        size_t bar() const { return p_bar; }
        uint8_t row() const { return p_row; }
        uint8_t column() const { return p_column; }

        /** Returns true if the index is within the range of
            the NoteStream, Bar, it's rows and columns.
            The check is performed against the current Score. */
        bool isValid() const;

    private:
        friend class Score;
        Index() { }
        Score* p_score;
        size_t p_stream, p_bar;
        uint8_t p_row, p_column;
    };


    // --- ctor ---

    Score();
    ~Score();

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // ---- getter ----

    /** Returns the given property, or invalid QVariant */
    QVariant property(const QString& key) const;

    size_t numNoteStreams() const;
    const NoteStream& noteStream(size_t idx) const;

    const QList<NoteStream>& noteStreams() const;

    QString title() const { return property("title").toString(); }
    QString author() const { return property("author").toString(); }
    QString copyright() const { return property("copyright").toString(); }

    /** Returns an index to the specified note.
        If any of the indices is out of range an invalid Index is returned.
        @note The returned Index is tied to this Score instance and
        only valid during it's lifetime. */
    Index createIndex(
            size_t stream, size_t bar, size_t row, size_t column) const;

    // ---- setter ----

    void clear();
    void clearProperties();
    void clearScore();

    void setProperty(const QString& key, const QVariant& prop);

    void setNoteStream(size_t idx, const NoteStream&);
    void appendNoteStream(const NoteStream&);
    void insertNoteStream(size_t idx, const NoteStream&);
    void removeNoteStream(size_t idx);

    void setTitle(const QString& s) { setProperty("title", QVariant(s)); }
    void setAuthor(const QString& s) { setProperty("author", QVariant(s)); }
    void setCopyright(const QString& s) { setProperty("copyright", QVariant(s)); }

private:
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_SCORE_H
