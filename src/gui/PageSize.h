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

#include <QtCore>
#include <QSizeF>
#include <QRectF>
#include <QStringList>

#include "QProps/JsonInterface.h"
#include "QProps/Properties.h"

namespace Sonot {

class PageSize : public QProps::JsonInterface
{
    Q_DECLARE_TR_FUNCTIONS(PageSize)

    /** Default initializer is private */
    PageSize();
public:

    // --- types ---

    /** The paper formats */
    enum Format
    {
        F_CUSTOM,
        F_ISO_A4
    };
    static QProps::Properties::NamedValues formatNamedValues();

    /** QSizeF for Format */
    static QSizeF formatSize(Format);

    // --- ctor ---

    /** Constructor with preset type */
    explicit PageSize(Format);

    /** Constructor for F_CUSTOM type */
    PageSize(double width_mm, double height_mm);

    /** Constructor for F_CUSTOM type */
    PageSize(const QSizeF& size_in_mm);

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // --- getter ---

    Format format() const { return (Format)p_props_.get("format").toInt(); }
    double width() const { return p_props_.get("size").toSizeF().width(); }
    double height() const { return p_props_.get("size").toSizeF().height(); }
    QSizeF size() const { return p_props_.get("size").toSizeF(); }
    QRectF rect() const { return QRectF(0,0, width(), height()); }

    bool operator == (const PageSize& o) const;
    bool operator != (const PageSize& o) const { return !(*this == o); }

    const QProps::Properties& props() const { return p_props_; }

    // --- setter ---

    void setFormat(Format f);
    void setWidth(double mm);
    void setHeight(double mm);
    void setSize(const QSizeF& s);
    void setSize(double width_mm, double height_mm);

private:
    QProps::Properties p_props_;
};

} // namespace Sonot

#endif // SONOTSRC_PAGESIZE_H
