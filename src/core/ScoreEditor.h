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

namespace Sonot {

class ScoreEditor : public QObject
{
    Q_OBJECT
public:

    typedef QList<Score::Index> IndexList;

    explicit ScoreEditor(QObject *parent = 0);
    ~ScoreEditor();

    Score* score() const;

    // --- editing ---

    void setScore(Score*);
    void insertNote(const Score::Index&, const Note& n);
    void insertBars(const Score::Index&, const QList<Bar>& rows);
    void changeNote(const Score::Index&, const Note& n);
    void changeBar(const Score::Index&, const Bar& b);
    void deleteNote(const Score::Index&);
    void deleteBar(const Score::Index&);
    void deleteStream(const Score::Index&);

signals:

    void scoreReset(Score*);

    void streamsChanged(const IndexList&);
    void barsChanged(const IndexList&);
    void noteValuesChanged(const IndexList&);

    void streamsAboutToBeDeleted(const IndexList&);
    void barsAboutToBeDeleted(const IndexList&);
    void notesAboutToBeDeleted(const IndexList&);

    void streamsDeleted(const IndexList&);
    void barsDeleted(const IndexList&);
    void notesDeleted(const IndexList&);

public slots:
private:
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_SCOREEDITOR_H
