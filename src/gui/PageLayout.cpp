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

namespace Sonote {

const int ODD = 0;
const int EVEN = 1;

PageLayout::PageLayout()
    : p_pageRect_   (0., 0., 190., 297.)
{
    p_marginLeft_[ODD] = 20.; p_marginLeft_[EVEN] = 30.;
    p_marginRight_[ODD] = 30.; p_marginRight_[EVEN] = 20.;
    p_marginTop_[ODD] = p_marginTop_[EVEN] = 20.;
    p_marginBottom_[ODD] = p_marginBottom_[EVEN] = 30.;
}

QRectF PageLayout::pageRect() const { return p_pageRect_; }

QRectF PageLayout::contentRect(int pageNum) const
{
    int idx = (pageNum & 1) == 0 ? EVEN : ODD;

    return QRectF(
                p_pageRect_.left() + p_marginLeft_[idx],
                p_pageRect_.top() + p_marginTop_[idx],
                p_pageRect_.width() - p_marginLeft_[idx] - p_marginRight_[idx],
                p_pageRect_.height() - p_marginTop_[idx] - p_marginBottom_[idx]);
}

} // namespace Sonote
