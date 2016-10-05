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

class Bar;
class Note;
class Notes;
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
        /** Creates Invalid index */
        Index() : p_score(nullptr) { }

        Score* score() const { return p_score; }
        /** The NoteStream index (zero-based) */
        size_t stream() const { return p_stream; }
        /** The index of the row/voice (zero-based) */
        size_t row() const { return p_row; }
        /** The index of the Bar (zero-based) */
        size_t bar() const { return p_bar; }
        /** The index of the Note in Notes (zero-based) */
        size_t column() const { return p_column; }

        /** Returns true if the index is within the range of
            the NoteStream, Bar, it's rows and columns.
            The check is performed against the current Score. */
        bool isValid() const;
        bool isValid(int stream, int barIdx, int row, int column) const;

        /** Is at start of row/Notes */
        bool isLeft() const { return isValid() && column() == 0; }
        /** Is at end of row/Notes */
        bool isRight() const;
        bool isTop() const { return isValid() && row() == 0; }
        bool isBottom() const;

        /** Returns true if the index is pointing into the first
            Bar of a NoteStream */
        bool isStreamLeft() const;
        bool isStreamRight() const;

        /** Returns true if this Bar's tempo is different to previous one's */
        bool isTempoChange() const;

        /** Returns true if the index points to an empty Stream or Bar.
            Getter functions are not allowed to use!
            @todo not used yet */
        bool isInserter() const;

        /** Returns the number of rows in current NoteStream */
        size_t numRows() const;

        bool operator == (const Index& rhs) const;
        bool operator != (const Index& rhs) const { return !(*this == rhs); }
        bool operator < (const Index& rhs) const;

        const NoteStream& getStream() const;
        const Bar& getBar() const;
        const Notes& getNotes(int row_offset = 0) const;
        const Note& getNote() const;
        //QList<Notes> getBar(int startRow = -1, int numRows = -1) const;

        const Note& getNote(int row, int column) const;

        /** The beats-per-minute at the current position */
        double getBeatsPerMinute() const;
        /** The length of this bar in seconds,
            depending on the BPM. */
        double getBarLengthSeconds() const;

        Index left() const { return score()->index(stream(), bar(), row(), 0); }
        Index right() const;
        Index top() const { return score()->index(stream(), bar(), 0, column());}
        Index bottom() const;
        Index topLeft() const { return score()->index(stream(), bar(), 0, 0); }
        Index bottomRight() const;
        Index streamTopLeft() const { return score()->index(stream(), 0,0,0); }
        Index streamBottomRight() const;

        Index offset(int row_, int column_) const
            { return score()->index(stream(), bar(),
                                    row()+row_, column()+column_); }
        /** Clamp to valid right stream/row/bar/note */
        Index limitRight() const;

        Index closest(const Index& i1, const Index& i2) const;
        Index closestManhatten(const Index& i1, const Index& i2) const;
        Index farthestManhatten(const Index& i1, const Index& i2) const;

        QString toString() const;

        // --- setters ---

        void setNote(const Note&);

        // --- iterators ---

        bool nextStream();
        bool prevStream();

        /** Iterates to the next Bar, or returns false if not possible.
            The column() and row() will be limited on success.
            @note This function never changes the isValid() state. */
        bool nextBar();
        bool prevBar();

        bool nextRow();
        bool prevRow();

        /** Iterates to the next Note, or returns false if not possible.
            @note This function never changes the isValid() state. */
        bool nextNote();
        bool prevNote();

    private:
        friend class Score;
        Score* p_score;
        size_t p_stream, p_bar, p_row, p_column;
    };

    /** A selection between two indices */
    struct Selection
    {
        /** Creates invalid selection */
        Selection() { }
        /** Creates selection from single note */
        Selection(const Index& idx) : Selection(idx, idx) { }
        /** Creates selection from smallest to largets index */
        Selection(const Index& i1, const Index& i2);

        // -- getter --

        bool isValid() const;
        bool isSingleNote() const;
        bool isSingleRow() const;
        bool isSingleColumn() const;
        bool isSingleBar() const;
        bool isSingleStream() const;
        bool isCompleteRow() const;
        bool isCompleteColumn() const;
        bool isCompleteBar() const;
        bool isCompleteStream() const;
        bool isCompleteScore() const;

        bool operator == (const Selection& o) const
            { return from() == o.from() && to() == o.to(); }
        bool operator != (const Selection& o) const { return !(*this == o); }

        Score* score() const;
        const Index& from() const { return p_from; }
        const Index& to() const { return p_to; }

        bool contains(const Index& idx) const;

        QList<Index> containedIndices() const;

        QString toString() const;

        // -- re-select --

        /** clear and reassign single point */
        void set(const Index& idx);
        /** Set selection between lowest and highest point */
        void set(const Index& idx1, const Index& idx2)
            { set(idx1); unifyWith(idx2); }
        /** expand current selection */
        Selection& unifyWith(const Index& idx);
        Selection unified(const Index& idx) const;

        /** Creates a selection from the Notes line at @p idx */
        static Selection fromNotes(const Index& idx);
        /** Creates a selection from the note column at @p idx */
        static Selection fromColumn(const Index& idx);
        /** Creates a selection from the whole bar at @p idx */
        static Selection fromBar(const Index& idx);
        /** Creates a selection from the whole stream at @p idx */
        static Selection fromStream(const Index& idx);
        /** Creates a selection containing all full bars of the
            original selection */
        static Selection fromBars(const Selection& sel);

    private:
        friend class Score;
        Index p_from, p_to;
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

    bool isEmpty() const { return numNoteStreams() == 0; }
    size_t numNoteStreams() const;
    const NoteStream& noteStream(size_t idx) const;

    const QList<NoteStream>& noteStreams() const;

    QString stringTitle() const { return props().get("title").toString(); }
    QString stringAuthor() const { return props().get("author").toString(); }
    QString stringCopyright() const
                            { return props().get("copyright").toString(); }
    QString stringSource() const { return props().get("source").toString(); }
    QString stringTranscriber() const
                            { return props().get("transcriber").toString(); }

    /** Returns an index to the specified note.
        If any of the indices is out of range an invalid Index is returned.
        @note The returned Index is tied to this Score instance and
        must only be referenced during it's lifetime. */
    Index index(
            size_t stream, size_t barIdx, size_t row, size_t column) const;

    bool operator == (const Score& rhs) const;
    bool operator != (const Score& rhs) const { return !(*this == rhs); }

    /** Returns a string containing the number of bars and rows per
        NoteStream */
    QString toInfoString() const;

    // ---- setter ----

    void clear();
    void clearProperties();
    void clearScore();

    NoteStream& noteStream(size_t idx);
    void setNoteStream(size_t idx, const NoteStream&);
    void appendNoteStream(const NoteStream&);
    void insertNoteStream(size_t idx, const NoteStream&);
    void removeNoteStream(size_t idx);
    void removeNoteStreams(size_t idx, int64_t count = -1);

    void setProperties(const QProps::Properties& p);
    void setTitle(const QString& s) { propsw().set("title", s); }
    void setAuthor(const QString& s) { propsw().set("author", s); }
    void setCopyright(const QString& s) { propsw().set("copyright", s); }

private:
    QProps::Properties& propsw();
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_SCORE_H
