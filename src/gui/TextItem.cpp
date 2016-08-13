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
#include <QJsonArray>
#include <QFont>

#include "TextItem.h"

namespace Sonote {

TextItem::TextItem()
    : JsonInterface     ("TextItem")
    , p_boxAlign_       (Qt::AlignAbsolute)
    , p_textAlign_      (Qt::AlignCenter)
    , p_textFlags_      (Qt::TextDontClip)
    , p_pointSize_      (8.)
    , p_boundingBox_    (0,0,10,10)
{

}

QJsonObject TextItem::toJson() const
{
    QJsonObject o;
    o.insert("ver", 1);
    o.insert("text", QJsonValue(p_text_));
    o.insert("box", json_wrap(p_boundingBox_));
    o.insert("point-size", p_pointSize_);
    /** @todo supposing Qt::AlignmentFlag values never change */
    o.insert("box-align", QJsonValue((int)p_boxAlign_));
    o.insert("text-align", QJsonValue((int)p_textAlign_));
    /** @todo supposing Qt::TextFlag values never change */
    o.insert("text-flags", QJsonValue((int)p_textFlags_));

    return o;
}

void TextItem::fromJson(const QJsonObject& o)
{
    p_text_ = json_expectChild<QString>(o, "text");
    p_boundingBox_ = json_expectChild<QRectF>(o, "box");
    p_pointSize_ = json_expectChild<double>(o, "point-size");
    p_boxAlign_ = (Qt::Alignment)json_expectChild<int>(o, "box-align");
    p_textAlign_ = (Qt::Alignment)json_expectChild<int>(o, "text-align");
    p_textFlags_ = (Qt::TextFlag)json_expectChild<int>(o, "text-flags");
}


QRectF TextItem::alignedBoundingBox(const QRectF& p) const
{
    auto r = boundingBox();

    if (p_boxAlign_.testFlag(Qt::AlignVCenter))
    {
        r.moveTop(p.top() + (p.height() - r.height()) / 2.);
    }
    else if (p_boxAlign_.testFlag(Qt::AlignTop))
    {
        r.moveTop(p.top() + r.top());
    }
    else if (p_boxAlign_.testFlag(Qt::AlignBottom))
    {
        r.moveBottom(p.bottom());
    }

    if (p_boxAlign_.testFlag(Qt::AlignHCenter))
    {
        r.moveLeft(p.left() + (p.width() - r.width()) / 2.);
    }
    else if (p_boxAlign_.testFlag(Qt::AlignLeft))
    {
        r.moveLeft(p.left() + r.left());
    }
    else if (p_boxAlign_.testFlag(Qt::AlignRight))
    {
        r.moveRight(p.right());
    }

    return r;
}

QFont TextItem::font() const
{
    QFont f;

    f.setPointSizeF(p_pointSize_);

    return f;
}

} // namespace Sonote
