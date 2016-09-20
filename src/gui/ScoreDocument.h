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
        /** Creates invalid document index */
        Index() : p_pageIdx(-1), p_lineIdx(-1), p_barIdx(-1) { }

        int pageIdx() const { return p_pageIdx; }
        int lineIdx() const { return p_lineIdx; }
        int barIdx() const { return p_barIdx; }

        bool isValid() const
            { return p_pageIdx >= 0 && p_lineIdx >= 0 && p_barIdx >= 0; }

        bool operator == (const Index& o) const
            { return p_barIdx == o.p_barIdx && p_lineIdx == o.p_lineIdx
                    && p_pageIdx == o.p_pageIdx; }

        bool operator < (const Index& o) const
        {
            return p_pageIdx == o.p_pageIdx
                     ? p_lineIdx == o.p_lineIdx
                         ? p_barIdx < o.p_barIdx
                         : p_lineIdx < o.p_lineIdx
                     : p_pageIdx < o.p_pageIdx;
        }

    private:
        friend class ScoreDocument;
        Index (int page, int line, int bar)
            : p_pageIdx(page), p_lineIdx(line), p_barIdx(bar) { }
        int p_pageIdx, p_lineIdx, p_barIdx;
    };

    struct BarItems
    {
        QList<ScoreItem> items;
        Index docIndex;
        Score::Index scoreIndex;
        QRectF boundingBox;
    };

    /** Display order of pages */
    enum PageOrdering
    {
        PO_HORIZONTAL,
        PO_VERTICAL,
        PO_TWO_SIDED
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

    size_t numPages() const;

    PageOrdering pageOrdering() const;
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

    /** Rectangle of page in document space */
    QRectF pageRect(int pageIdx) const;

    /** Top-left position of given page in document space */
    QPointF pagePosition(int pageIndex) const;

    /** Spacing between displayed pages */
    QPointF pageSpacing() const { return props().get("page-spacing").toPointF(); }

    /** Returns the page index for a given point in document-space.
        Returns -1 if p is not on a page. */
    int pageIndexForDocumentPosition(const QPointF& p) const;
    int pageIndexForScoreIndex(const Score::Index& idx) const;

    Index docIndexForScoreIndex(const Score::Index& idx) const;

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
    ScoreItem* getClosestScoreItem(int pageIdx, const QPointF& pagePos) const;
    ScoreItem* getLeftScoreItem(const Score::Index& i) const;
    ScoreItem* getRightScoreItem(int pageIdx, int lineIdx) const;

    //void updateScoreIndex(const Score::Index& i);

    /** Returns the score index for the next row,
        finding the "visually appropriate" position.
        Returns invalid index if not possible */
    Score::Index goToNextRow(const Score::Index& idx) const;
    /** Returns the score index for the previous row,
        finding the "visually appropriate" position.
        Returns invalid index if not possible */
    Score::Index goToPrevRow(const Score::Index& idx) const;

    /** Returns score index for the first note in the current
        line of score or the start of the page if already at
        first pos */
    Score::Index goToStart(const Score::Index& idx) const;

    /** Returns score index for the last note in the current
        line of score or the end of the page if already at
        last pos */
    Score::Index goToEnd(const Score::Index& idx) const;

    // ----- settter ------

    void initLayout();

    void setScore(const Score&);

    void setPageAnnotation(int pageIdx, const PageAnnotation& p);
    void setPageAnnotation(const QString&, const PageAnnotation& p);
    void setPageLayout(int pageIdx, const PageLayout& p);
    void setPageLayout(const QString&, const PageLayout& p);
    void setScoreLayout(int pageIdx, const ScoreLayout& p);
    void setScoreLayout(const QString&, const ScoreLayout& p);

    void setProperties(const QProps::Properties& p);
    void setScoreProperties(const QProps::Properties& p);

    // ----- render -----

    void paintScoreItems(QPainter& p, int pageIdx,
                         const QRectF& updateRect) const;

    QList<QRectF> getSelectionRects(int pageIdx,
                                    const Score::Selection& s) const;
private:
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_SCOREDOCUMENT_H
