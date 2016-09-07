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

#ifndef SONOTSRC_TEXTITEM_H
#define SONOTSRC_TEXTITEM_H

#include <QtCore>
#include <QString>
#include <QRectF>
#include <QColor>

#include "io/JsonInterface.h"
#include "io/Properties.h"

class QFont;

namespace Sonot {

/** @brief Class for storing/displaying a text on the score sheet */
class TextItem : public JsonInterface
{
    Q_DECLARE_TR_FUNCTIONS(TextItem)
public:
    /** Flags for font */
    enum FontFlag
    {
        F_NONE = 0,
        F_ITALIC = 1,
        F_BOLD = 2,
        F_UNDERLINE = 4
    };
    static Properties::NamedValues fontFlagNamedValues();

    TextItem();

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // -- getter --

    QString text() const { return p_props_.get("text").toString(); }
    Qt::Alignment boxAlignment() const
        { return (Qt::Alignment)p_props_.get("align-box").toInt(); }
    Qt::Alignment textAlignment() const
        { return (Qt::Alignment)p_props_.get("align-text").toInt(); }
    Qt::TextFlag textFlags() const
        { return (Qt::TextFlag)p_props_.get("text-flags").toInt(); }
    FontFlag fontFlags() const
        { return (FontFlag)p_props_.get("font-flags").toInt(); }
    double fontSize() const { return p_props_.get("font-size").toDouble(); }
    QColor color() const
        { return p_props_.get("color").value<QColor>(); }

    /** The bounding rectangle */
    QRectF boundingBox() const
        { return p_props_.get("box").value<QRectF>(); }

    /** Returns the boundingRect() aligned to enclosing @p parent rectangle. */
    QRectF alignedBoundingBox(const QRectF& parent) const;

    QFont font() const;

    bool operator == (const TextItem& o) const;
    bool operator != (const TextItem& o) const { return !(*this == o); }

    const Properties& props() const { return p_props_; }

    // -- setter --

    void setProperties(const Properties& p) { p_props_ = p; }

    void setBoundingBox(const QRectF& rect) { p_props_.set("box", rect); }
    void setBoxAlignment(Qt::Alignment a) { p_props_.set("align-box", int(a)); }

    void setText(const QString& t) { p_props_.set("text", t); }
    void setTextAlignment(Qt::Alignment a)
        { p_props_.set("align-text", int(a)); }
    /** Accepts or-combinations of
            Qt::TextDontClip
            Qt::TextSingleLine
            Qt::TextExpandTabs
            Qt::TextShowMnemonic
            Qt::TextWordWrap
            Qt::TextIncludeTrailingSpaces */
    void setTextFlags(int a) { p_props_.set("text-flags", int(a)); }
    /** Accepts or-combinations of FontFlag */
    void setFontFlags(int a) { p_props_.set("font-flags", int(a)); }
    void setFontSize(double pointSize) { p_props_.set("font-size", pointSize); }
    void setColor(const QColor& c) { p_props_.set("color", c); }

private:

    Properties p_props_;
};

} // namespace Sonot

#endif // SONOTSRC_TEXTITEM_H
