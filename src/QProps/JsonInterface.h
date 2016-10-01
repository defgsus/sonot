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

#ifndef QPROPS_SRC_JSONINTERFACE_H
#define QPROPS_SRC_JSONINTERFACE_H

#include <QString>

#include "qprops_global.h"

class QJsonObject;
class QJsonArray;
class QJsonValue;

namespace QProps {

/** Interface for loading/saving json data.
    Users of the interface need to implement the
    toJson() and fromJson() methods.
    This enables converting to/from json strings and file io.
*/
class //QPROPS_SHARED_EXPORT
        JsonInterface
{
public:

    virtual ~JsonInterface() { }

    // ----------- pure methods --------------

    /** Return a QJsonObject with all data inside.
        Should throw a descriptive Sonot::Exception on any errors. */
    virtual QJsonObject toJson() const = 0;

    /** Initializes the object from the QJsonObject.
        Should throw a descriptive Sonot::Exception on severe errors. */
    virtual void fromJson(const QJsonObject&) = 0;

    // ------------ convenience --------------

    /** Converts this object's data to a Json string.
        Uses toJson().
        @throws Sonot::Exception on any error */
    virtual QString toJsonString(bool compact = true) const;

    /** Initializes this objects's data from a json string.
        Uses fromJson().
        @throws Sonot::Exception on any error */
    virtual void fromJsonString(const QString&);

    /** Stores the json string to a file.
        Uses toJsonString().
        @throws Sonot::Exception on any error */
    virtual void saveJsonFile(
            const QString& filename, bool compact = true) const;

    /** Initializes this object's data from a json file.
        Uses fromJsonString().
        @throws Sonot::Exception on any error */
    virtual void loadJsonFile(const QString& filename);

};


} // namespace QProps

#endif // QPROPS_SRC_JSONINTERFACE_H
