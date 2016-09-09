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

    void init(bool oddOrEven);

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
    QRectF contentRect() const;
    /** The rect for the score = contectRect() - scoreMargins */
    QRectF scoreRect() const;

    const Properties& margins() const { return p_margins_; }

    // ---- setter ----

    void setPageSize(const PageSize& p) { p_pageSize_ = p; }

    void setMargins(const Properties& margins) { p_margins_ = margins; }

private:

    PageSize p_pageSize_;
    Properties p_margins_;
};

} // namespace Sonot

#endif // SONOTSRC_PAGELAYOUT_H
