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

#ifndef SONOTESRC_TEXTITEM_H
#define SONOTESRC_TEXTITEM_H

#include <QString>
#include <QRectF>

#include "io/JsonInterface.h"

class QPainter;

namespace Sonote {

/** @brief Class for storing/displaying a text on the score sheet */
class TextItem : public JsonInterface
{
public:
    TextItem();

    // -- getter --

    QString text() const { return p_text_; }
    Qt::Alignment boxAlignment() const { return p_boxAlign_; }
    Qt::Alignment textAlignment() const { return p_textAlign_; }
    const QRectF& boundingRect() const { return p_boundingRect_; }

    // -- setter --

    void setText(const QString& t) { p_text_ = t; }
    void setBoxAlignment(Qt::Alignment a) { p_boxAlign_ = a; }
    void setTextAlignment(Qt::Alignment a) { p_textAlign_ = a; }
    void setBoundingRect(const QRectF& rect) { p_boundingRect_ = rect; }

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // --- render ---

    //void paint(QPainter*, QRect);

private:
    Qt::Alignment p_boxAlign_, p_textAlign_;
    QRectF p_boundingRect_;
    QString p_text_;
};

} // namespace Sonote

#endif // SONOTESRC_TEXTITEM_H
