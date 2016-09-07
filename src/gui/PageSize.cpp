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

PageSize::PageSize()
    : p_props_("page-size")
{
    p_props_.set("format", tr("format"),
                 tr("The format of the paper"),
                 formatNamedValues(),
                 int(F_ISO_A4));
    p_props_.set("size", tr("custom size"),
                 tr("Custom size of paper in mm"),
                 formatSize(F_ISO_A4));
}

Properties::NamedValues PageSize::formatNamedValues()
{
    Properties::NamedValues nv;
    nv.set("custom", tr("custom"),
           tr("user defined format"),
           int(F_CUSTOM));
    nv.set("iso-a4", tr("ISO A4"),
           tr("A4 format of ISO-..."),
           F_ISO_A4);
    return nv;
}

QSizeF PageSize::formatSize(Format f)
{
    switch (f)
    {
        case F_CUSTOM: return QSizeF();
        case F_ISO_A4: return QSizeF(210, 297);
    }
    return QSizeF();
}

PageSize::PageSize(Format f)
    : PageSize()
{
    setFormat(f);
}

PageSize::PageSize(double width_mm, double height_mm)
    : PageSize()
{
    setSize(width_mm, height_mm);
}

PageSize::PageSize(const QSizeF& size_in_mm)
    : PageSize()
{
    setSize(size_in_mm);
}

bool PageSize::operator == (const PageSize& o) const
{
    return p_props_ == o.p_props_;
}

void PageSize::setFormat(Format f)
{
    if (f == F_CUSTOM)
        return;

    p_props_.set("format", int(f));
    p_props_.set("size", formatSize(f));
}

void PageSize::setWidth(double mm)
{
    p_props_.set("format", int(F_CUSTOM));
    p_props_.set("size", QSizeF(mm, height()));
}

void PageSize::setHeight(double mm)
{
    p_props_.set("format", int(F_CUSTOM));
    p_props_.set("size", QSizeF(width(), mm));
}

void PageSize::setSize(const QSizeF& s)
{
    p_props_.set("format", int(F_CUSTOM));
    p_props_.set("size", s);
}

void PageSize::setSize(double width_mm, double height_mm)
{
    setSize(QSizeF(width_mm, height_mm));
}



QJsonObject PageSize::toJson() const
{
    QJsonObject o;
    o.insert("props", p_props_.toJson());
    return o;
}

void PageSize::fromJson(const QJsonObject& o)
{
    JsonHelper json("PageSize");
    Properties p(p_props_);
    p.fromJson(json.expectChildObject(o, "props"));
    p_props_ = p;
}


} // namespace Sonot
