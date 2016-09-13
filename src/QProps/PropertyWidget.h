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

#ifdef QT_WIDGETS_LIB

#ifndef QPROPS_SRC_PROPERTYWIDGET_H
#define QPROPS_SRC_PROPERTYWIDGET_H

#include <QWidget>

#include "error.h"

namespace QProps {

class Properties;

/** Dynamic widget for a Properties::Property.
    Supports all primitive types, QVector of primitive types and most
    compound types that QVariant supports.

    @note uint32_t, int64_t, uint64_t are bounded by the
          range of QSpinBox's int32_t
    */
class PropertyWidget : public QWidget
{
    Q_OBJECT
public:

    /** Constructor for a Properties::Property with given @p id */
    explicit PropertyWidget(const QString& id, const Properties* p,
                            QWidget *parent = 0) throw(Exception);

    /** Returns the currently set value */
    const QVariant& value() const;

    QWidget* editWidget() const;

signals:

    /** Emitted when the user changed the value (and only then). */
    void valueChanged();

private slots:

    void onValueChanged_();

private:

    struct Private;
    Private * p_;
};

} // namespace QProps

#endif // QPROPS_SRC_PROPERTYWIDGET_H

#endif
