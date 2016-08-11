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

#include "TextItem.h"

namespace Sonote {

TextItem::TextItem()
    : JsonInterface     ("TextItem")
    , p_boxAlign_       (Qt::AlignAbsolute)
    , p_textAlign_      (Qt::AlignLeft)
{

}

QJsonObject TextItem::toJson() const
{
    QJsonObject o;
    o.insert("text", QJsonValue(p_text_));
    o.insert("box", json_wrap(p_boundingRect_));
    o.insert("box-align", QJsonValue((int)p_boxAlign_));
    o.insert("text-align", QJsonValue((int)p_textAlign_));

    return o;
}

void TextItem::fromJson(const QJsonObject& o)
{
    p_text_ = json_expectChild(o, "text").toString();
    p_boundingRect_ = json_expect<QRectF>(json_expectChild(o, "box"));
    p_boxAlign_ = (Qt::Alignment)json_expect<int>(json_expectChild(o, "box-align"));
    p_textAlign_ = (Qt::Alignment)json_expect<int>(json_expectChild(o, "text-align"));
}


} // namespace Sonote
