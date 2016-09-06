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

#include "PageAnnotationTemplate.h"

namespace Sonot {


PageAnnotationTemplate::PageAnnotationTemplate()
{

}

QJsonObject PageAnnotationTemplate::toJson() const
{
    QJsonObject o;
    for (auto i = p_pages_.begin(); i != p_pages_.end(); ++i)
    {
        o.insert(i.key(), i.value().toJson());
    }
    return o;
}

void PageAnnotationTemplate::fromJson(const QJsonObject& o)
{
    JsonHelper json("PageAnnotationTemplate");
    QMap<QString, PageAnnotation> map;
    for (auto i = o.begin(); i != o.end(); ++i)
    {
        PageAnnotation p;
        p.fromJson(json.expectObject(i.value()));
        map.insert(i.key(), p);
    }
    p_pages_.swap(map);
}

bool PageAnnotationTemplate::operator == (const PageAnnotationTemplate& o) const
{
    return p_pages_ == o.p_pages_;
}

PageAnnotation PageAnnotationTemplate::getPage(const QString& id) const
{
    if (!p_pages_.contains(id))
        return PageAnnotation();

    return p_pages_[id];
}

PageAnnotation PageAnnotationTemplate::getPage(int pageIndex) const
{
    const QString pageId = pageIndex == 0 ? "title"
                                          : (pageIndex & 1) == 1 ? "left"
                                                                 : "right";
    return getPage(pageId);
}

void PageAnnotationTemplate::setPage(
        const QString& id, const PageAnnotation& page)
{
    p_pages_[id] = page;
}

void PageAnnotationTemplate::init(const QString& )
{
    clear();

    for (int k = 0; k < 3; ++k)
    {
        const bool titlePage = k == 0;
        const bool leftPage = k == 1;
        const QString pageId = titlePage ? "title" : leftPage ? "left" : "right";

        PageAnnotation page;
        TextItem ti;

        if (titlePage)
        {
            ti = TextItem();
            ti.setBoundingBox(QRectF(0,0,100,10));
            ti.setBoxAlignment(Qt::AlignHCenter | Qt::AlignTop);
            ti.setText("#title");
            ti.setFontFlags(TextItem::F_ITALIC);
            page.textItems().push_back(ti);
        }

        if (!titlePage)
        {
            ti = TextItem();
            ti.setBoundingBox(QRectF(0,0,100,10));
            ti.setBoxAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            ti.setText("#copyright");
            ti.setFontFlags(TextItem::F_ITALIC);
            ti.setFontSize(4.);
            page.textItems().push_back(ti);
        }

        if (!titlePage)
        {
            ti = TextItem();
            ti.setBoundingBox(QRectF(0,0,20,10));
            ti.setBoxAlignment(Qt::AlignBottom |
                               (leftPage ? Qt::AlignLeft : Qt::AlignRight));
            ti.setTextAlignment(Qt::AlignBottom |
                                (leftPage ? Qt::AlignLeft : Qt::AlignRight));
            ti.setText("#page");
            ti.setFontSize(5.);
            page.textItems().push_back(ti);
        }

        p_pages_.insert(pageId, page);
    }

}


} // namespace Sonot
