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

#ifndef SONOTSRC_SCOREITEM_H
#define SONOTSRC_SCOREITEM_H

#include <QRectF>

#include "core/Note.h"
#include "core/Score.h"

class QPainter;

namespace Sonot {

class ScoreItem
{
public:
    enum Type
    {
        T_NOTE,
        T_BAR_SLASH
    };

    ScoreItem(const Score::Index& i, const QRectF& rect);
    ScoreItem(const Score::Index& i, const QLineF& rect);

    // ---- getter ----

    Score* score() const { return p_index_.score(); }
    Score::Index index() const { return p_index_; }
    Type type() const { return p_type_; }

    const Note& note() const { return p_index_.getNote(); }

    QRectF boundingBox() const { return p_rect_; }

    // ---- render ----

    void paint(QPainter& p);


private:

    Score::Index p_index_;
    Type p_type_;
    QRectF p_rect_;
};

} // namespace Sonot

#endif // SONOTSRC_SCOREITEM_H
