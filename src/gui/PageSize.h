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

#ifndef SONOTESRC_PAGESIZE_H
#define SONOTESRC_PAGESIZE_H

#include <QSizeF>

namespace Sonot {

class PageSize
{
public:

    enum Format
    {
        F_CUSTOM,
        F_DIN_A4
    };

    /** Constructor with preset type */
    explicit PageSize(Format);

    /** Constructor for F_CUSTOM type */
    PageSize(double width_mm, double height_mm)
        : p_width_(width_mm), p_height_(height_mm)
        , p_format_(F_CUSTOM) { }

    // --- getter ---

    Format format() const { return p_format_; }
    double width() const { return p_width_; }
    double height() const { return p_height_; }
    QSizeF size() const { return QSize(p_width_, p_height_); }

    // --- setter ---

    void setFormat(Format f);
    void setWidth(double mm) { p_width_ = mm; p_format_ = F_CUSTOM; }
    void setHeight(double mm) { p_height_ = mm; p_format_ = F_CUSTOM; }
    void setSize(const QSize& s)
        { p_width_ = s.width(); p_height_ = s.height(); p_format_ = F_CUSTOM; }
    void setSize(double width_mm, double height_mm)
        { p_width_ = width_mm; p_height_ = height_mm; p_format_ = F_CUSTOM; }

private:

    double p_width_, p_height_;
    Format p_format_;
};

} // namespace Sonot

#endif // SONOTESRC_PAGESIZE_H
