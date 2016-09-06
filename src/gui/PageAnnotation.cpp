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

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "PageAnnotation.h"

namespace Sonot {


PageAnnotation::PageAnnotation()
{

}

QJsonObject PageAnnotation::toJson() const
{
    QJsonArray a;
    for (const TextItem& t : p_textItems)
        a.append( t.toJson() );
    QJsonObject o;
    o.insert("text-items", a);
    return o;
}

void PageAnnotation::fromJson(const QJsonObject& o)
{
    JsonHelper json("PageAnnotation");
    QJsonArray a = json.expectArray(json.expectChildValue(o, "text-items"));

    std::vector<TextItem> items;
    for (int i=0; i<a.size(); ++i)
    {
        TextItem item;
        item.fromJson(json.expectObject(a.at(i)));
        items.push_back(item);
    }

    p_textItems.swap(items);
}

bool PageAnnotation::operator == (const PageAnnotation& o) const
{
    return p_textItems == o.p_textItems;
}



} // namespace Sonot
