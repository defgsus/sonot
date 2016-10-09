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
#include "QProps/JsonInterfaceHelper.h"

namespace Sonot {


ScoreLayout::ScoreLayout()
    : p_props_      ("score-layout")
{
    p_props_.set("fixed-number-bars", tr("fixed number bars"),
                 tr("Puts the same amount of bars on every line"),
                 true);
    p_props_.set("bars-per-line", tr("bars per line"),
                 tr("The factor of the number of bars that should go "
                    "on one line"),
                 4);
    p_props_.setMin("bars-per-line", 1);

    p_props_.set("note-size", tr("note size"),
                 tr("The size of note items"),
                 4., 1.);

    p_props_.set("note-spacing", tr("note spacing"),
                 tr("The distance between adjacent notes"),
                 3., .1);
    p_props_.set("row-spacing", tr("row spacing"),
                 tr("The vertical distance between rows"),
                 5., .1);
    p_props_.set("line-spacing", tr("line spacing"),
                 tr("The vertical distance between lines of score"),
                 6., 1.);
    p_props_.set("min-bar-width", tr("bar width (minimum)"),
                 tr("The minimum width of a bar"),
                 24., 1.);
    p_props_.set("max-bar-width", tr("bar width (maximum)"),
                 tr("The maximum width of a bar"),
                 48., 1.);

    p_props_.setUpdateVisibilityCallback([](QProps::Properties& p)
    {
        bool fixed = p.get("fixed-number-bars").toBool();
        p.setVisible("bars-per-line", fixed);
        p.setVisible("note-spacing", !fixed);
        p.setVisible("min-bar-width", !fixed);
        p.setVisible("max-bar-width", !fixed);
    });
}


bool ScoreLayout::operator == (const ScoreLayout& o) const
{
    return p_props_ == o.p_props_;
}

QJsonObject ScoreLayout::toJson() const
{
    QJsonObject o;
    o.insert("props", p_props_.toJson());
    return o;
}

void ScoreLayout::fromJson(const QJsonObject& o)
{
    QProps::JsonInterfaceHelper json("ScoreLayout");
    ScoreLayout l;
    l.p_props_.fromJson(json.expectChildObject(o, "props"));

    *this = l;
}

QFont ScoreLayout::font() const
{
    QFont f;
    f.setPointSizeF(noteSize());
    f.setBold(true);
    return f;
}

double ScoreLayout::lineHeight(int numRows) const
{
    return rowSpacing() * numRows;
}


} // namespace Sonot
