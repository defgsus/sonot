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

#include <QString>
#include <QRectF>
#include <QColor>

#include "io/JsonInterface.h"

class QFont;

namespace Sonot {

/** @brief Class for storing/displaying a text on the score sheet */
class TextItem : public JsonInterface
{
public:
    /** Flags for font selection.
        order MUST NOT change for json persistence! */
    enum FontFlag
    {
        F_NONE = 0,
        F_ITALIC = 1,
        F_BOLD = 2,
        F_UNDERLINE = 4
    };

    TextItem();

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // -- getter --

    QString text() const { return p_text_; }
    Qt::Alignment boxAlignment() const { return p_boxAlign_; }
    Qt::Alignment textAlignment() const { return p_textAlign_; }
    Qt::TextFlag textFlags() const { return p_textFlags_; }
    FontFlag fontFlags() const { return p_fontFlags_; }
    double fontSize() const { return p_pointSize_; }
    const QColor& color() const { return p_color_; }

    /** The bounding rectangle */
    const QRectF& boundingBox() const { return p_boundingBox_; }

    /** Returns the boundingRect() aligned to enclosing @p parent rectangle. */
    QRectF alignedBoundingBox(const QRectF& parent) const;

    QFont font() const;

    // -- setter --

    void setBoundingBox(const QRectF& rect) { p_boundingBox_ = rect; }
    void setBoxAlignment(Qt::Alignment a) { p_boxAlign_ = a; }

    void setText(const QString& t) { p_text_ = t; }
    void setTextAlignment(Qt::Alignment a) { p_textAlign_ = a; }
    /** Accepts or-combinations of
            Qt::TextDontClip
            Qt::TextSingleLine
            Qt::TextExpandTabs
            Qt::TextShowMnemonic
            Qt::TextWordWrap
            Qt::TextIncludeTrailingSpaces */
    void setTextFlags(int a) { p_textFlags_ = Qt::TextFlag(a); }
    /** Accepts or-combinations of FontFlag */
    void setFontFlags(int a) { p_fontFlags_ = FontFlag(a); }
    void setFontSize(double pointSize) { p_pointSize_ = pointSize; }
    void setColor(const QColor& c) { p_color_ = c; }

private:

    Qt::Alignment p_boxAlign_, p_textAlign_;
    Qt::TextFlag p_textFlags_;
    FontFlag p_fontFlags_;
    double p_pointSize_;
    QColor p_color_;
    QRectF p_boundingBox_;
    QString p_text_;
};

} // namespace Sonot

#endif // SONOTSRC_TEXTITEM_H
