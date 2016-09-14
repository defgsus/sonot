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

#ifndef SONOTSRC_SCOREDOCUMENT_H
#define SONOTSRC_SCOREDOCUMENT_H

#include <QtCore>

#include "QProps/JsonInterface.h"
#include "QProps/Properties.h"
#include "PageLayout.h"
#include "PageAnnotation.h"
#include "ScoreLayout.h"
#include "core/Score.h"

class QRectF;
class QPointF;
class QPainter;

namespace Sonot {

class ScoreItem;
class ScoreEditor;

/** Wrapper around Score and layout classes */
class ScoreDocument : public QProps::JsonInterface
{
    Q_DECLARE_TR_FUNCTIONS(ScoreDocument);

public:

    // --- types ---

    struct Index
    {
        int pageIdx, lineIdx, barIdx;

        Index (int page = 0, int line = 0, int bar = 0)
            : pageIdx(page), lineIdx(line), barIdx(bar) { }

        bool operator == (const Index& o) const
            { return barIdx == o.barIdx && lineIdx == o.lineIdx
                    && pageIdx == o.pageIdx; }
        bool operator < (const Index& o) const
        {
            return pageIdx == o.pageIdx
                     ? lineIdx == o.lineIdx
                         ? barIdx < o.barIdx
                         : lineIdx < o.lineIdx
                     : pageIdx < o.pageIdx;
        }
    };

    struct BarItems
    {
        QList<ScoreItem> items;
        Index docIndex;
        Score::Index scoreIndex;
        QRectF boundingBox;
    };

    // --- ctor ---

    ScoreDocument();
    ScoreDocument(const ScoreDocument& o) = delete;
    ~ScoreDocument();

    ScoreDocument& operator = (const ScoreDocument& o) = delete;

    bool operator == (const ScoreDocument& rhs) const;
    bool operator != (const ScoreDocument& rhs) const { return !(*this == rhs); }

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // ----- getter -------

    const Score* score() const;
    ScoreEditor* editor() const;

    QRectF pageRect() const;
    const PageLayout& pageLayout(const QString& id) const;
    const ScoreLayout& scoreLayout(const QString& id) const;
    const PageAnnotation& pageAnnotation(const QString& id) const;

    const PageLayout& pageLayout(int pageIdx) const
        { return pageLayout(layoutKeyForIndex(pageIdx)); }
    const ScoreLayout& scoreLayout(int pageIdx) const
        { return scoreLayout(layoutKeyForIndex(pageIdx)); }
    const PageAnnotation& pageAnnotation(int pageIdx) const
        { return pageAnnotation(layoutKeyForIndex(pageIdx)); }

    const QProps::Properties& props() const;

    // ---- helper ----

    /** Top-left position of given page */
    QPointF pagePosition(int pageIndex) const;

    /** Spacing between displayed pages */
    QPointF pageSpacing() const { return props().get("page-spacing").toPointF(); }

    /** Returns the page index for a given point in document-space.
        Returns -1 if p is not on a page. */
    int pageIndexForDocumentPosition(const QPointF& p) const;

    /** Returns the page number for the page index */
    int pageNumberForIndex(int pageIndex) const;

    QString layoutKeyForIndex(int pageIdx) const;

    BarItems* getBarItems(const Index&) const;
    BarItems* getBarItems(int pageIdx, int lineIdx, int barIdx) const
        { return getBarItems(Index(pageIdx, lineIdx, barIdx)); }
    BarItems* getBarItems(int pageIdx, const QPointF& pagePos) const;
    Score::Index getScoreIndex(int pageIdx, const QPointF& pagePos) const;
    Score::Index getScoreIndex(const QPointF& documentPos) const;
    ScoreItem* getScoreItem(const Score::Index& i) const;
    void updateScoreIndex(const Score::Index& i);

    // ----- settter ------

    void initLayout();

    void setScore(const Score&);

    void setPageAnnotation(int pageIdx, const PageAnnotation& p);
    void setPageLayout(int pageIdx, const PageLayout& p);

    void setPageSpacing(const QPointF& f) { props().set("page-spacing", f); }
    void setProperties(const QProps::Properties& p);


    // ----- render -----

    void paintScoreItems(QPainter& p, int pageIdx) const;

private:
    QProps::Properties& props();
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_SCOREDOCUMENT_H
