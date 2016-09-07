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

#ifndef SONOTSRC_PROPERTIESVIEW_H
#define SONOTSRC_PROPERTIESVIEW_H

#include <QScrollArea>

namespace Sonot {

class Properties;

/** Dynamic gui display/editor for Properties */
class PropertiesView : public QScrollArea
{
    Q_OBJECT
public:
    explicit PropertiesView(QWidget *parent = 0);
    ~PropertiesView();

    /** Returns read access to the current properties */
    const Properties& properties() const;

signals:

    /** Emitted when the user has changed a property value */
    void propertyChanged(const QString& id);

public slots:

    /** Destroys all property widgets */
    void clear();

    /** Assigns a new set of Properties to edit */
    void setProperties(const Properties& );

    /** Assigns multiple sets of Properties to edit.
        A widget is created for each unique id in all Properties. */
    void setProperties(const QList<Properties>& );

private:

    struct Private;
    Private * p_;
};

} // namespace Sonot

#endif // SONOTSRC_PROPERTIESVIEW_H
