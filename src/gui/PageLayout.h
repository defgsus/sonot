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

#ifndef SONOTSRC_PAGELAYOUT_H
#define SONOTSRC_PAGELAYOUT_H

#include <QtCore>
#include <QRectF>

#include "PageSize.h"
#include "io/JsonInterface.h"
#include "io/Properties.h"

namespace Sonot {

/** Layout settings per odd/even page pair.
    Mainly controls size and margins
    */
class PageLayout : public JsonInterface
{
    Q_DECLARE_TR_FUNCTIONS(PageLayout)

public:
    PageLayout();
    void init();

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // --- getter ---

    bool operator == (const PageLayout& o) const;
    bool operator != (const PageLayout& o) const { return !(*this == o); }

    PageSize pageSize() const { return p_pageSize_; }
    /** The whole page (starting at <0,0>) */
    QRectF pageRect() const { return p_pageSize_.rect(); }
    /** The rect for the content (annotations) = pageRect - margins */
    QRectF contentRect(int pageIndex) const;
    /** The rect for the score = contectRect() - scoreMargins */
    QRectF scoreRect(int pageIndex) const;

    /** Is pageIndex in contentRect() zero-based or one-based? */
    bool isZeroBased() const { return p_zeroBased_; }

    const Properties& marginsOdd() const { return p_margins_[0]; }
    const Properties& marginsEven() const { return p_margins_[1]; }
    const Properties& scoreMarginsOdd() const { return p_margins_[2]; }
    const Properties& scoreMarginsEven() const { return p_margins_[3]; }

    // ---- setter ----

    void setPageSize(const PageSize& p) { p_pageSize_ = p; }

    Properties& marginsOdd() { return p_margins_[0]; }
    Properties& marginsEven() { return p_margins_[1]; }
    Properties& scoreMarginsOdd() { return p_margins_[2]; }
    Properties& scoreMarginsEven() { return p_margins_[3]; }

private:

    PageSize p_pageSize_;
    Properties p_margins_[4];
    bool p_zeroBased_;
};

} // namespace Sonot

#endif // SONOTSRC_PAGELAYOUT_H
