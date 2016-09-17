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

#include <QJsonObject>
#include <QJsonValue>

#include "PageLayout.h"
#include "QProps/JsonInterfaceHelper.h"

namespace Sonot {


PageLayout::PageLayout()
    : p_pageSize_   (PageSize::F_ISO_A4)
    , p_margins_    ("margins")
{
    // content
    p_margins_.set(
                "left", tr("left margin"),
                tr("distance to left page border"),
                20., 1.);
    p_margins_.set(
                "right", tr("right margin"),
                tr("distance to right page border"),
                20., 1.);
    p_margins_.set(
                "top", tr("top margin"),
                tr("distance to top page border"),
                20., 1.);
    p_margins_.set(
                "bottom", tr("bottom margin"),
                tr("distance to bottom page border"),
                30., 1.);
    // score
    p_margins_.set(
                "score-left", tr("left score margin"),
                tr("distance of score to left content border"),
                5., 1.);
    p_margins_.set(
                "score-right", tr("right score margin"),
                tr("distance of score to right content border"),
                5., 1.);
    p_margins_.set(
                "score-top", tr("top score margin"),
                tr("distance of score to top content border"),
                5., 1.);
    p_margins_.set(
                "score-bottom", tr("bottom score margin"),
                tr("distance of score to bottom content border"),
                10., 1.);
}

bool PageLayout::operator == (const PageLayout& o) const
{
    return p_pageSize_ == o.p_pageSize_
        && p_margins_ == o.p_margins_;
}

QJsonObject PageLayout::toJson() const
{
    QJsonObject o;
    o.insert("page-size", p_pageSize_.toJson());
    o.insert("margins", p_margins_.toJson());
    return o;
}

void PageLayout::fromJson(const QJsonObject& o)
{
    QProps::JsonInterfaceHelper json("PageLayout");
    PageLayout tmp;
    tmp.p_pageSize_.fromJson(json.expectChildObject(o, "page-size"));
    tmp.p_margins_.fromJson(json.expectChildObject(o, "margins"));

    *this = tmp;
}

QRectF PageLayout::contentRect() const
{
    const double
            marginLeft = p_margins_.get("left").toDouble(),
            marginRight = p_margins_.get("right").toDouble(),
            marginTop = p_margins_.get("top").toDouble(),
            marginBottom = p_margins_.get("bottom").toDouble();

    auto r = pageRect();
    return QRectF(
            r.left()   + marginLeft,
            r.top()    + marginTop,
            r.width()  - marginLeft - marginRight,
            r.height() - marginTop - marginBottom);
}


QRectF PageLayout::scoreRect() const
{
    const double
            marginLeft = p_margins_.get("score-left").toDouble(),
            marginRight = p_margins_.get("score-right").toDouble(),
            marginTop = p_margins_.get("score-top").toDouble(),
            marginBottom = p_margins_.get("score-bottom").toDouble();

    auto r = contentRect();
    return QRectF(
            r.left()   + marginLeft,
            r.top()    + marginTop,
            r.width()  - marginLeft - marginRight,
            r.height() - marginTop - marginBottom);
}

} // namespace Sonot
