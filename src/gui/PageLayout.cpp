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
    , p_zeroBased_  (true)
{
    init();
}

bool PageLayout::operator == (const PageLayout& o) const
{
#define SONOT__COMP(mem__) \
            mem__[0] == o.mem__[0] && mem__[1] == o.mem__[1]

    return p_pageSize_ == o.p_pageSize_
        && p_zeroBased_ == o.p_zeroBased_
        && SONOT__COMP(p_marginLeft_)
        && SONOT__COMP(p_marginRight_)
        && SONOT__COMP(p_marginTop_)
        && SONOT__COMP(p_marginBottom_)
        && SONOT__COMP(p_scoreMarginLeft_)
        && SONOT__COMP(p_scoreMarginRight_)
        && SONOT__COMP(p_scoreMarginTop_)
        && SONOT__COMP(p_scoreMarginBottom_)
            ;
#undef SONOT__COMP
}

QJsonObject PageLayout::toJson() const
{
    QJsonObject o;
    o.insert("page-size", p_pageSize_.toJson());

    for (int i=0; i<2; ++i)
    {
        QJsonObject p;
        p.insert("margin-left", p_marginLeft_[i]);
        p.insert("margin-right", p_marginRight_[i]);
        p.insert("margin-top", p_marginTop_[i]);
        p.insert("margin-bottom", p_marginBottom_[i]);
        p.insert("score-margin-left",   p_scoreMarginLeft_[i]);
        p.insert("score-margin-right",  p_scoreMarginRight_[i]);
        p.insert("score-margin-top",    p_scoreMarginTop_[i]);
        p.insert("score-margin-bottom", p_scoreMarginBottom_[i]);
        o.insert(i == ODD ? "odd" : "even", p);
    }
    return o;
}

void PageLayout::fromJson(const QJsonObject& o)
{
    JsonHelper json("PageLayout");
    PageLayout tmp;
    tmp.p_pageSize_.fromJson(json.expectChildObject(o, "page-size"));
    for (int i=0; i<2; ++i)
    {
        QJsonObject p = json.expectChildObject(o, i == ODD ? "odd" : "even");
        tmp.p_marginLeft_[i] =   json.expectChild<double>(p, "margin-left");
        tmp.p_marginRight_[i] =  json.expectChild<double>(p, "margin-right");
        tmp.p_marginTop_[i] =    json.expectChild<double>(p, "margin-top");
        tmp.p_marginBottom_[i] = json.expectChild<double>(p, "margin-bottom");
        tmp.p_scoreMarginLeft_[i] =
                json.expectChild<double>(p, "score-margin-left");
        tmp.p_scoreMarginRight_[i] =
                json.expectChild<double>(p, "score-margin-right");
        tmp.p_scoreMarginTop_[i] =
                json.expectChild<double>(p, "score-margin-top");
        tmp.p_scoreMarginBottom_[i] =
                json.expectChild<double>(p, "score-margin-bottom");
    }

    *this = tmp;
}

void PageLayout::init()
{
    p_pageSize_.setFormat(PageSize::F_ISO_A4);

    p_marginLeft_[ODD] = 20.;   p_marginLeft_[EVEN] = 30.;
    p_marginRight_[ODD] = 30.;  p_marginRight_[EVEN] = 20.;
    p_marginTop_[ODD] =         p_marginTop_[EVEN] = 20.;
    p_marginBottom_[ODD] =      p_marginBottom_[EVEN] = 30.;

    p_scoreMarginLeft_[ODD] = 5.;    p_scoreMarginLeft_[EVEN] = 5.;
    p_scoreMarginRight_[ODD] = 5.;   p_scoreMarginRight_[EVEN] = 5.;
    p_scoreMarginTop_[ODD] =         p_scoreMarginTop_[EVEN] = 20.;
    p_scoreMarginBottom_[ODD] =      p_scoreMarginBottom_[EVEN] = 10.;
}

QRectF PageLayout::contentRect(int pageNum) const
{
    if (p_zeroBased_)
        ++pageNum;

    int idx = (pageNum & 1) == 0 ? EVEN : ODD;

    auto r = pageRect();
    return QRectF(
            r.left()   + p_marginLeft_[idx],
            r.top()    + p_marginTop_[idx],
            r.width()  - p_marginLeft_[idx] - p_marginRight_[idx],
            r.height() - p_marginTop_[idx] - p_marginBottom_[idx]);
}


QRectF PageLayout::scoreRect(int pageNum) const
{
    auto r = contentRect(pageNum);

    if (p_zeroBased_)
        ++pageNum;

    int idx = (pageNum & 1) == 0 ? EVEN : ODD;

    return QRectF(
            r.left()   + p_scoreMarginLeft_[idx],
            r.top()    + p_scoreMarginTop_[idx],
            r.width()  - p_scoreMarginLeft_[idx] - p_scoreMarginRight_[idx],
            r.height() - p_scoreMarginTop_[idx]  - p_scoreMarginBottom_[idx]);
}

} // namespace Sonot
