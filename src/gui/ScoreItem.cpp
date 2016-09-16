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

#include <QPainter>

#include "ScoreItem.h"

namespace Sonot {

ScoreItem::ScoreItem(const Score::Index& i,
                     const ScoreDocument::Index& j, const QRectF& rect)
    : p_index_      (i)
    , p_docIndex_   (j)
    , p_type_       (T_NOTE)
    , p_rect_       (rect)
{

}

ScoreItem::ScoreItem(const Score::Index& i, const ScoreDocument::Index& j, const QLineF& line)
    : p_index_      (i)
    , p_docIndex_   (j)
    , p_type_       (T_BAR_SLASH)
    , p_rect_       (line.x1(), line.y1(),
                     line.x2()-line.x1(), line.y2()-line.y1())
{

}

void ScoreItem::paint(QPainter& p)
{
    if (p_type_ == T_NOTE)
    {
        QString t = note().toSpanishString();

        QFont f(p.font());
        f.setPixelSize(p_rect_.height());
        f.setBold(true);
        p.setFont(f);
        p.drawText(p_rect_, Qt::AlignCenter | Qt::TextDontClip, t);
        //p.drawRect(p_rect_);

        //p.drawLine(p_rect_.center().x(), p_rect_.top(),
        //           p_rect_.center().x(), p_rect_.bottom());
    }
    else
    {
        p.drawLine(p_rect_.x(), p_rect_.y(),
                   p_rect_.bottomRight().x(), p_rect_.bottomRight().y());
    }


}

} // namespace Sonot
