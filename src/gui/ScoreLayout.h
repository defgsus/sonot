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

#ifndef SONOTSRC_SCORELAYOUT_H
#define SONOTSRC_SCORELAYOUT_H

#include <QtCore>

#include "QProps/JsonInterface.h"
#include "QProps/Properties.h"

class QFont;

namespace Sonot {


class ScoreLayout : public QProps::JsonInterface
{
    Q_DECLARE_TR_FUNCTIONS(ScoreLayout)
public:
    ScoreLayout();

    // ---- io ----

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &);

    // ---- getter ----

    const QProps::Properties& props() const { return p_props_; }

    bool isFixedBarWidth() const
                    { return p_props_.get("fixed-bar-width").toBool(); }

    double noteSpacing() const { return p_props_.get("note-spacing").toDouble(); }
    double rowSpacing() const { return p_props_.get("row-spacing").toDouble(); }
    double lineSpacing() const { return p_props_.get("line-spacing").toDouble(); }
    double minBarWidth() const { return p_props_.get("min-bar-width").toDouble(); }
    double maxBarWidth() const { return p_props_.get("max-bar-width").toDouble(); }
    double noteSize() const { return p_props_.get("note-size").toDouble(); }
    int barsPerLine() const { return p_props_.get("bars-per-line").toInt(); }

    bool operator == (const ScoreLayout& o) const;
    bool operator != (const ScoreLayout& o) const { return !(*this == o); }

    // --- convenience ---

    QFont font() const;

    double lineHeight(int numRows) const;

    // --- setter ---

    void setProperties(QProps::Properties& p) { p_props_ = p; }

private:
    QProps::Properties p_props_;
};

} // namespace Sonot

#endif // SONOTSRC_SCORELAYOUT_H
