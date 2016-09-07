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

namespace Sonot {

TextItem::TextItem()
    : p_props_      ("text-item")
{
    p_props_.set("text", tr("text"),
                 tr("The text to print"),
                 QString(""));
    p_props_.set("font-size", tr("font size"),
                 tr("The size of the font in mm"),
                 8.);
    p_props_.set("color", tr("color"),
                 tr("The color of the text"),
                 QColor(Qt::black));
    p_props_.set("box", tr("bounding box"),
                 tr("The box surrounding and aligning the text"),
                 QRectF(0,0,10,10));
    p_props_.set("align-box", tr("box alignment"),
                 tr("Alignment of the containing box"),
                 Properties::namedValuesQtAlignment(),
                 int(Qt::AlignAbsolute));
    p_props_.set("align-text", tr("text alignment"),
                 tr("Alignment of the text within containing box"),
                 Properties::namedValuesQtAlignment(),
                 int(Qt::AlignCenter));
    p_props_.set("text-flags", tr("text options"),
                 tr("Options regarding the flow of text"),
                 Properties::namedValuesQtTextFlag(),
                 int(Qt::TextDontClip));
    p_props_.set("font-flags", tr("font options"),
                 tr("Options on the used font"),
                 fontFlagNamedValues(),
                 int(0));
}

Properties::NamedValues TextItem::fontFlagNamedValues()
{
    Properties::NamedValues f;
    f.setIsFlags(true);
    f.set("italic", tr("italic"), int(F_ITALIC));
    f.set("bold", tr("bold"), int(F_BOLD));
    f.set("underline", tr("underline"), int(F_UNDERLINE));
    return f;
}

bool TextItem::operator == (const TextItem& o) const
{
    return p_props_ == o.p_props_;
}


QJsonObject TextItem::toJson() const
{
    QJsonObject o;
    o.insert("props", p_props_.toJson());

    return o;
}

void TextItem::fromJson(const QJsonObject& o)
{
    JsonHelper json("TextItem");

    TextItem t;
    t.p_props_.fromJson( json.expectChildObject(o, "props") );

    *this = t;
}


QRectF TextItem::alignedBoundingBox(const QRectF& p) const
{
    auto r = boundingBox();

    auto boxAlign = boxAlignment();

    if (boxAlign.testFlag(Qt::AlignVCenter))
    {
        r.moveTop(p.top() + (p.height() - r.height()) / 2.);
    }
    else if (boxAlign.testFlag(Qt::AlignTop))
    {
        r.moveTop(p.top() + r.top());
    }
    else if (boxAlign.testFlag(Qt::AlignBottom))
    {
        r.moveBottom(p.bottom());
    }

    if (boxAlign.testFlag(Qt::AlignHCenter))
    {
        r.moveLeft(p.left() + (p.width() - r.width()) / 2.);
    }
    else if (boxAlign.testFlag(Qt::AlignLeft))
    {
        r.moveLeft(p.left() + r.left());
    }
    else if (boxAlign.testFlag(Qt::AlignRight))
    {
        r.moveRight(p.right());
    }

    return r;
}

QFont TextItem::font() const
{
    QFont f;

    //f.setFamily("Verdana");
    f.setPointSizeF(fontSize());

    auto flags = fontFlags();
    f.setItalic(flags & F_ITALIC);
    f.setBold(flags & F_BOLD);
    f.setUnderline(flags & F_UNDERLINE);

    return f;
}

} // namespace Sonot
