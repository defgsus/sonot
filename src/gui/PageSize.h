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

#ifndef SONOTSRC_PAGESIZE_H
#define SONOTSRC_PAGESIZE_H

#include <QSizeF>
#include <QRectF>
#include <QStringList>

#include "io/JsonInterface.h"

namespace Sonot {

class PageSize : public JsonInterface
{
public:

    enum Format
    {
        F_CUSTOM,
        F_ISO_A4
    };
    static const QStringList formatIds;
    static Format formatFromId(const QString& id);

    /** Constructor with preset type */
    explicit PageSize(Format);

    /** Constructor for F_CUSTOM type */
    PageSize(double width_mm, double height_mm)
        : p_size_   (width_mm, height_mm)
        , p_format_ (F_CUSTOM) { }

    /** Constructor for F_CUSTOM type */
    PageSize(const QSizeF& s)
        : p_size_   (s)
        , p_format_ (F_CUSTOM) { }

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // --- getter ---

    Format format() const { return p_format_; }
    double width() const { return p_size_.width(); }
    double height() const { return p_size_.height(); }
    QSizeF size() const { return p_size_; }
    QRectF rect() const { return QRectF(0,0, width(), height()); }
    QString formatId() const;

    bool operator == (const PageSize& o) const;
    bool operator != (const PageSize& o) const { return !(*this == o); }

    // --- setter ---

    void setFormat(Format f);
    void setWidth(double mm) { p_size_.setWidth(mm); p_format_ = F_CUSTOM; }
    void setHeight(double mm) { p_size_.setHeight(mm); p_format_ = F_CUSTOM; }
    void setSize(const QSize& s) { p_size_ = s; p_format_ = F_CUSTOM; }
    void setSize(double width_mm, double height_mm)
        { p_size_.setWidth(width_mm); p_size_.setHeight(height_mm);
          p_format_ = F_CUSTOM; }

private:

    QSizeF p_size_;
    Format p_format_;
};

} // namespace Sonot

#endif // SONOTSRC_PAGESIZE_H
