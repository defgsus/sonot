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

#include "PageSize.h"

namespace Sonot {

const QStringList PageSize::formatIds =
{
    "custom",
    "a4"
};

PageSize::PageSize(Format f)
{
    setFormat(f);
}

QString PageSize::formatId() const
{
    return p_format_ < 0 || p_format_ >= formatIds.size()
            ? "*unkown*" : formatIds[p_format_];
}

PageSize::Format PageSize::formatFromId(const QString &id)
{
    for (int i=0; i<formatIds.size(); ++i)
        if (id == formatIds.at(i))
            return Format(i);
    return F_CUSTOM;
}

void PageSize::setFormat(Format f)
{
    p_format_ = f;

    switch (p_format_)
    {
        case F_CUSTOM: break;
        case F_ISO_A4: p_size_ = QSizeF(210, 297); break;
    }
}

bool PageSize::operator == (const PageSize& o) const
{
    return p_size_ == o.p_size_
        && p_format_ == o.p_format_;
}

QJsonObject PageSize::toJson() const
{
    JsonHelper json("PageSize");
    QJsonObject o;
    o.insert("size", json.wrap(p_size_));
    o.insert("format", formatId());
    return o;
}

void PageSize::fromJson(const QJsonObject& o)
{
    JsonHelper json("PageSize");
    QSizeF size = json.expectChild<QSizeF>(o, "size");
    Format fmt = formatFromId(json.expectChild<QString>(o, "format"));

    p_size_ = size;
    p_format_ = fmt;
}


} // namespace Sonot
