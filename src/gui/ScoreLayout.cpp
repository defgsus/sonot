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
#include <QFont>

#include "ScoreLayout.h"

namespace Sonot {


ScoreLayout::ScoreLayout()
{
    init();
}

void ScoreLayout::init()
{
    p_noteSpacing_ = 3.;
    p_rowSpacing_ = 7.;
    p_lineSpacing_ = 12.;

    p_minBarWidth_ = 24.;
    p_maxBarWidth_ = 24.*2.;

    p_noteSize_ = 4.;
}

QJsonObject ScoreLayout::toJson() const
{
    QJsonObject o;
    o.insert("note-spacing", QJsonValue(p_noteSpacing_));
    o.insert("row-spacing", QJsonValue(p_rowSpacing_));
    o.insert("line-spacing", QJsonValue(p_lineSpacing_));

    o.insert("min-bar-width", QJsonValue(p_minBarWidth_));
    o.insert("max-bar-width", QJsonValue(p_maxBarWidth_));

    o.insert("note-size", QJsonValue(p_noteSize_));
    return o;
}

void ScoreLayout::fromJson(const QJsonObject& o)
{
    JsonHelper json("ScoreLayout");
    ScoreLayout l;
    l.p_noteSpacing_ = json.expectChild<double>(o, "note-spacing");
    l.p_rowSpacing_ = json.expectChild<double>(o, "row-spacing");
    l.p_lineSpacing_ = json.expectChild<double>(o, "line-spacing");

    l.p_minBarWidth_ = json.expectChild<double>(o, "min-bar-width");
    l.p_maxBarWidth_ = json.expectChild<double>(o, "max-bar-width");

    l.p_noteSize_ = json.expectChild<double>(o, "note-size");

    *this = l;
}

QFont ScoreLayout::font() const
{
    QFont f;
    f.setPointSizeF(p_noteSize_);
    f.setBold(true);
    return f;
}

double ScoreLayout::lineHeight(int numRows) const
{
    return rowSpacing() * numRows;
}


} // namespace Sonot
