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

#include "io/JsonInterface.h"

class QFont;

namespace Sonot {


class ScoreLayout : public JsonInterface
{
public:
    ScoreLayout();
    void init();

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &);

    double noteSpacing() const { return p_noteSpacing_; }
    double rowSpacing() const { return p_rowSpacing_; }
    double lineSpacing() const { return p_lineSpacing_; }
    double minBarWidth() const { return p_minBarWidth_; }
    double maxBarWidth() const { return p_maxBarWidth_; }

    double noteSize() const { return p_noteSize_; }

    // --- convenience ---

    QFont font() const;

    double lineHeight(int numRows) const;

private:

    double
        p_noteSpacing_,
        p_rowSpacing_,
        p_lineSpacing_,
        p_minBarWidth_,
        p_maxBarWidth_,

        p_noteSize_;
};

} // namespace Sonot

#endif // SONOTSRC_SCORELAYOUT_H
