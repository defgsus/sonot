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

namespace Sonot {

namespace {
    const int ODD = 0;
    const int EVEN = 1;
}


PageLayout::PageLayout()
    : p_pageSize_   (PageSize::F_ISO_A4)
    , p_margins_    ({ Properties("margins-odd"),
                       Properties("margins-even"),
                       Properties("margins-top"),
                       Properties("margins-bottom")})
    , p_zeroBased_  (true)
{
    // content
    p_margins_[ODD].set(
                "left", tr("left margin"),
                tr("distance to left page border"),
                20., 1.);
    p_margins_[ODD].set(
                "right", tr("right margin"),
                tr("distance to right page border"),
                30., 1.);
    p_margins_[ODD].set(
                "top", tr("top margin"),
                tr("distance to top page border"),
                20., 1.);
    p_margins_[ODD].set(
                "bottom", tr("bottom margin"),
                tr("distance to bottom page border"),
                30., 1.);
    p_margins_[EVEN] = p_margins_[ODD];
    p_margins_[EVEN].set("left", 30.);
    p_margins_[EVEN].set("right", 20.);

    // score
    p_margins_[2+ODD].set(
                "left", tr("left score margin"),
                tr("distance of score to left content border"),
                5., 1.);
    p_margins_[2+ODD].set(
                "right", tr("right score margin"),
                tr("distance of score to right content border"),
                5., 1.);
    p_margins_[2+ODD].set(
                "top", tr("top score margin"),
                tr("distance of score to top content border"),
                20., 1.);
    p_margins_[2+ODD].set(
                "bottom", tr("bottom score margin"),
                tr("distance of score to bottom content border"),
                10., 1.);
    p_margins_[2+EVEN] = p_margins_[2+ODD];
}

bool PageLayout::operator == (const PageLayout& o) const
{
    return p_pageSize_ == o.p_pageSize_
        && p_zeroBased_ == o.p_zeroBased_
        && p_margins_[0] == o.p_margins_[0]
        && p_margins_[1] == o.p_margins_[1]
        && p_margins_[2] == o.p_margins_[2]
        && p_margins_[3] == o.p_margins_[3];
}

QJsonObject PageLayout::toJson() const
{
    QJsonObject o;
    o.insert("page-size", p_pageSize_.toJson());
    o.insert("margins-odd", p_margins_[ODD].toJson());
    o.insert("margins-even", p_margins_[EVEN].toJson());
    o.insert("score-margins-odd", p_margins_[2+ODD].toJson());
    o.insert("score-margins-even", p_margins_[2+EVEN].toJson());
    return o;
}

void PageLayout::fromJson(const QJsonObject& o)
{
    JsonHelper json("PageLayout");
    PageLayout tmp;
    tmp.p_pageSize_.fromJson(json.expectChildObject(o, "page-size"));
    tmp.p_margins_[ODD].fromJson(json.expectChildObject(o, "margins-odd"));
    tmp.p_margins_[EVEN].fromJson(json.expectChildObject(o, "margins-even"));
    tmp.p_margins_[2+ODD].fromJson(json.expectChildObject(o, "score-margins-odd"));
    tmp.p_margins_[2+EVEN].fromJson(json.expectChildObject(o, "score-margins-even"));

    *this = tmp;
}

void PageLayout::init()
{
    p_pageSize_.setFormat(PageSize::F_ISO_A4);

#if 0
    p_marginLeft_[ODD] = 20.;   p_marginLeft_[EVEN] = 30.;
    p_marginRight_[ODD] = 30.;  p_marginRight_[EVEN] = 20.;
    p_marginTop_[ODD] =         p_marginTop_[EVEN] = 20.;
    p_marginBottom_[ODD] =      p_marginBottom_[EVEN] = 30.;

    p_scoreMarginLeft_[ODD] = 5.;    p_scoreMarginLeft_[EVEN] = 5.;
    p_scoreMarginRight_[ODD] = 5.;   p_scoreMarginRight_[EVEN] = 5.;
    p_scoreMarginTop_[ODD] =         p_scoreMarginTop_[EVEN] = 20.;
    p_scoreMarginBottom_[ODD] =      p_scoreMarginBottom_[EVEN] = 10.;
#endif
}

QRectF PageLayout::contentRect(int pageNum) const
{
    if (p_zeroBased_)
        ++pageNum;

    int idx = (pageNum & 1) == 0 ? EVEN : ODD;

    const double
            marginLeft = p_margins_[idx].get("left").toDouble(),
            marginRight = p_margins_[idx].get("right").toDouble(),
            marginTop = p_margins_[idx].get("top").toDouble(),
            marginBottom = p_margins_[idx].get("bottom").toDouble();

    auto r = pageRect();
    return QRectF(
            r.left()   + marginLeft,
            r.top()    + marginTop,
            r.width()  - marginLeft - marginRight,
            r.height() - marginTop - marginBottom);
}


QRectF PageLayout::scoreRect(int pageNum) const
{
    auto r = contentRect(pageNum);

    if (p_zeroBased_)
        ++pageNum;

    int idx = (pageNum & 1) == 0 ? EVEN : ODD;
    idx += 2;

    const double
            marginLeft = p_margins_[idx].get("left").toDouble(),
            marginRight = p_margins_[idx].get("right").toDouble(),
            marginTop = p_margins_[idx].get("top").toDouble(),
            marginBottom = p_margins_[idx].get("bottom").toDouble();

    return QRectF(
            r.left()   + marginLeft,
            r.top()    + marginTop,
            r.width()  - marginLeft - marginRight,
            r.height() - marginTop - marginBottom);
}

} // namespace Sonot
