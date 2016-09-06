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

#ifndef SONOTSRC_PAGEANNOTATION_H
#define SONOTSRC_PAGEANNOTATION_H

#include <vector>

#include "TextItem.h"
#include "io/JsonInterface.h"

namespace Sonot {


/** Collection of TextItems per page */
class PageAnnotation : public JsonInterface
{
public:
    PageAnnotation();

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // -- getter --

    const std::vector<TextItem>& textItems() const { return p_textItems; }

    bool operator == (const PageAnnotation& o) const;
    bool operator != (const PageAnnotation& o) const { return !(*this == o); }

    // -- setter --

    void clear() { p_textItems.clear(); }

    std::vector<TextItem>& textItems() { return p_textItems; }

private:

    std::vector<TextItem> p_textItems;
};

} // namespace Sonot

#endif // SONOTSRC_PAGEANNOTATION_H
