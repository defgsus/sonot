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

#include "PageLayout.h"

namespace Sonot {

const int ODD = 0;
const int EVEN = 1;

PageLayout::PageLayout()
    : p_pageSize_   (PageSize::F_ISO_A4)
    , p_zeroBased_  (true)
{
    init();
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
