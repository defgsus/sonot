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

#ifndef SONOTSRC_PROPERTYWIDGET_H
#define SONOTSRC_PROPERTYWIDGET_H

#include <QWidget>

namespace Sonot {

class Properties;

/** Dynamic widget for a Properties::Property.
    Most common types supported. */
class PropertyWidget : public QWidget
{
    Q_OBJECT
public:

    /** Constructor for a Properties::Property with given @p id */
    explicit PropertyWidget(
            const QString& id, const Properties* p, QWidget *parent = 0);

    /** Returns the currently set value */
    const QVariant& value() const;

signals:

    /** Emitted when the user changed the value (and only then). */
    void valueChanged();

private slots:

    void onValueChanged_();

private:

    struct Private;
    Private * p_;
};

} // namespace Sonot

#endif // SONOTSRC_PROPERTYWIDGET_H
