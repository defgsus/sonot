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

#ifndef SONOTSRC_SCOREEDITOR_H
#define SONOTSRC_SCOREEDITOR_H

#include <QObject>

#include "Score.h"

class QMimeData;

namespace Sonot {

#ifdef SONOT_GUI
class ScoreDocument;
class ScoreLayout;
class PageLayout;
#endif

class ScoreEditor : public QObject
{
    Q_OBJECT
public:

    typedef QList<Score::Index> IndexList;

#ifdef SONOT_GUI
    explicit ScoreEditor(ScoreDocument* doc, QObject *parent = 0);
#else
    explicit ScoreEditor(QObject *parent = 0);
#endif
    ~ScoreEditor();

    Score* score() const;
#ifdef SONOT_GUI
    ScoreDocument* document() const;
#endif

    // --- editing ---

    bool undo();
    bool redo();
    void clearUndo();
    void setEnableUndo(bool enable);
    /** Enables or disabled collapsing similiar undo actions.
        Default is true */
    void setMergeUndo(bool enable);

    void setScore(const Score& s);

    void setScoreProperties(const QProps::Properties&);
    bool setStreamProperties(size_t streamIdx, const QProps::Properties& p);
#ifdef SONOT_GUI
    void setDocumentProperties(const QProps::Properties&);
    void setPageLayout(const QString& id, const PageLayout&);
    void setScoreLayout(const QString& id, const ScoreLayout&);
#endif

    bool insertNote(const Score::Index&, const Note& n, bool allRows);
    bool insertBar(const Score::Index&, const Bar& bar,
                    bool insertAfterIndex = false);
    /** @todo undo */
    bool insertBars(const Score::Index&, const NoteStream& stream,
                    bool insertAfterIndex = false);
    bool insertRow(const Score::Index&, bool insertAfterIndex = false);
    bool insertStream(const Score::Index&, const NoteStream& s,
                      bool insertAfterIndex = false);
    /** @todo undo */
    bool insertScore(const Score::Index&, const Score& s,
                      bool insertAfterIndex = false);
    bool changeNote(const Score::Index&, const Note& n);
    bool changeBar(const Score::Index&, const Bar& b);
    bool deleteNote(const Score::Index&, bool allRows);
    bool deleteBar(const Score::Index&);
    bool deleteRow(const Score::Index&);
    bool deleteStream(const Score::Index&);

    /** @todo undo */
    bool transpose(const Score::Selection&, int steps);

    /** Splits a stream at the given Bar.
        The second stream will start with the next Bar.
        Does nothing on the last Bar of a stream.
        @todo undo */
    bool splitStream(const Score::Index&);

    /** Pastes the clipboard data, if valid
        @todo undo */
    bool pasteMimeData(const Score::Index&,
                       const QMimeData*, bool insertAfterIndex = false);

signals:

    /** When a new score was assigned */
    void scoreReset(Score*);

    /** Emitted for every change that would require saving. */
    void documentChanged();
    /** Request to redraw the view */
    void refresh();

    /** The Properties of a NoteStream have been changed */
    void streamPropertiesChanged(const IndexList&);
    void scorePropertiesChanged();
#ifdef SONOT_GUI
    void documentPropertiesChanged();
    void pageLayoutChanged(const QString& id);
    void scoreLayoutChanged(const QString& id);
#endif

    /** Cursor position after undo() or redo() */
    void cursorChanged(const Score::Index&);
    /** Signals if undo-data is available or not */
    void undoAvailable(bool, const QString& desc, const QString& detail);
    /** Signals if redo-data is available or not */
    void redoAvailable(bool, const QString& desc, const QString& detail);

    void streamsChanged(const IndexList&);
    void barsChanged(const IndexList&);
    void noteValuesChanged(const IndexList&);

    void streamsAboutToBeDeleted(const IndexList&);
    void barsAboutToBeDeleted(const IndexList&);
    void notesAboutToBeDeleted(const IndexList&);

    void streamsDeleted(const IndexList&);
    void barsDeleted(const IndexList&);
    void notesDeleted(const IndexList&);

    /** Stuff has been pasted (notes, bars, streams).
        The selection contains the actual bounds */
    void pasted(const Score::Selection&);

public slots:
private:
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_SCOREEDITOR_H
