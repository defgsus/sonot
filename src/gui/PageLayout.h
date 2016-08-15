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

#ifndef SONOTESRC_PAGELAYOUT_H
#define SONOTESRC_PAGELAYOUT_H

#include <QRectF>

namespace Sonot {

class PageLayout
{
public:
    PageLayout();

    // --- getter ---

    QRectF pageRect() const;
    QRectF contentRect(int pageIndex) const;

    /** Is pageIndex in contentRect() zero-based or one-based? */
    bool isZeroBased() const { return p_zeroBased_; }

private:

    QRectF p_pageRect_;
    double
        p_marginLeft_[2],
        p_marginRight_[2],
        p_marginTop_[2],
        p_marginBottom_[2];
    bool p_zeroBased_;
};

} // namespace Sonot

#endif // SONOTESRC_PAGELAYOUT_H
