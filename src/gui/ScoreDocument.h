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

#include "io/JsonInterface.h"

class QPointF;
class QRectF;

namespace Sonot {

class Score;
class ScoreLayout;
class PageLayout;
class PageSize;
class PageAnnotation;
class PageAnnotationTemplate;

/** Wrapper around Score and layout classes */
class ScoreDocument : public JsonInterface
{
public:
    ScoreDocument();
    ScoreDocument(const ScoreDocument& o);
    ~ScoreDocument();

    ScoreDocument& operator = (const ScoreDocument& o);

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // ----- getter -------

    const Score& score() const;

    QRectF pageRect() const;
    const PageLayout& pageLayout(int pageIdx) const;
    const ScoreLayout& scoreLayout(int pageIdx) const;
    PageAnnotation pageAnnotation(int pageIdx) const;

    /** Top-left position of given page */
    QPointF pagePosition(int pageIndex) const;

    /** Returns the page index for a given point in document-space.
        Returns -1 if p is not on a page. */
    int pageIndexForDocumentPosition(const QPointF& p) const;

    /** Returns the page number for the page index */
    int pageNumberForIndex(int pageIndex) const;

    // ----- settter ------

    void setScore(const Score&);

private:
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_SCOREDOCUMENT_H
