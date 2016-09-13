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

#include <QtCore>
#include <QVariant>

#include "QProps/JsonInterface.h"
#include "QProps/Properties.h"

namespace Sonot {

class Note;
class Bar;
class NoteStream;

/** Collection of NoteStream and custom properties */
class Score : public QProps::JsonInterface
{
    Q_DECLARE_TR_FUNCTIONS(Score)
public:

    // --- types ---

    /** Type to index a specific note.
        @note The Index is tied to the Score instance and
        must only be used during the lifetime of the Score instance! */
    struct Index
    {
        Score* score() const { return p_score; }
        /** The NoteStream index (zero-based) */
        size_t stream() const { return p_stream; }
        /** The index of the row/voice (zero-based) */
        size_t row() const { return p_row; }
        /** The index of the Bar within the row() (zero-based) */
        size_t bar() const { return p_bar; }
        /** The index of the Note in the Bar (zero-based) */
        size_t column() const { return p_column; }

        /** Returns true if the index is within the range of
            the NoteStream, Bar, it's rows and columns.
            The check is performed against the current Score. */
        bool isValid() const;
        bool isValid(int stream, int barIdx, int row, int column) const;

        bool operator == (const Index& rhs) const;
        bool operator != (const Index& rhs) const { return !(*this == rhs); }

        const NoteStream& getNoteStream() const;
        const Bar& getBar(int row = 0) const;
        const Note& getNote() const;

        const Note& getNote(int row, int column) const;

        Index topLeft() const { return score()->index(stream(), bar(), 0, 0); }

        QString toString() const;

        // --- iterators ---

        /** Iterates to the next Note, or returns false if not possible.
            @note This function never changes the isValid() state. */
        bool nextNote();
        bool prevNote();

        /** Iterates to the next Bar, or returns false if not possible.
            The column() and row() will be set to zero on success.
            @note This function never changes the isValid() state. */
        bool nextBar();
        bool prevBar();

    private:
        friend class Score;
        Index() { }
        Score* p_score;
        size_t p_stream, p_bar, p_row, p_column;
    };


    // --- ctor ---

    Score();
    ~Score();

    Score(const Score& o);
    Score& operator = (const Score& o);

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // ---- getter ----

    const QProps::Properties& props() const;

    size_t numNoteStreams() const;
    const NoteStream& noteStream(size_t idx) const;

    const QList<NoteStream>& noteStreams() const;

    QString title() const { return props().get("title").toString(); }
    QString author() const { return props().get("author").toString(); }
    QString copyright() const { return props().get("copyright").toString(); }

    /** Returns an index to the specified note.
        If any of the indices is out of range an invalid Index is returned.
        @note The returned Index is tied to this Score instance and
        must only be referenced during it's lifetime. */
    Index index(
            size_t stream, size_t barIdx, size_t row, size_t column) const;

    bool operator == (const Score& rhs) const;
    bool operator != (const Score& rhs) const { return !(*this == rhs); }

    // ---- setter ----

    void clear();
    void clearProperties();
    void clearScore();

    QProps::Properties& props();

    void setNoteStream(size_t idx, const NoteStream&);
    void appendNoteStream(const NoteStream&);
    void insertNoteStream(size_t idx, const NoteStream&);
    void removeNoteStream(size_t idx);

    void setTitle(const QString& s) { props().set("title", s); }
    void setAuthor(const QString& s) { props().set("author", s); }
    void setCopyright(const QString& s) { props().set("copyright", s); }

private:
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_SCORE_H
