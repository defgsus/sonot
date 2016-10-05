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
class QByteArray;

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
        Should throw a descriptive QProps::Exception on any errors. */
    virtual QJsonObject toJson() const = 0;

    /** Initializes the object from the QJsonObject.
        Should throw a descriptive QProps::Exception on severe errors. */
    virtual void fromJson(const QJsonObject&) = 0;

    // ------------ convenience --------------

    /** Converts this object's data to a Json string.
        Uses toJson().
        @throws QProps::Exception on any error */
    virtual QString toJsonString(bool compact = true) const;

    /** Converts this object's data to a Json string.
        Uses toJson().
        @throws QProps::Exception on any error */
    virtual QByteArray toJsonByteArray(bool compact = true) const;

    /** Converts this object's data to a zipped Json string.
        Uses toJsonByteArray().
        @throws QProps::Exception on any error */
    virtual QByteArray toJsonByteArrayZipped() const;

    /** Initializes this objects's data from a json string.
        Uses fromJsonByteArray().
        @throws QProps::Exception on any error */
    virtual void fromJsonString(const QString&);

    /** Initializes this objects's data from a json string.
        Uses fromJson().
        @throws QProps::Exception on any error */
    virtual void fromJsonByteArray(const QByteArray&);

    /** Initializes this objects's data from a zipped json string.
        Uses fromJsonByteArray().
        @throws QProps::Exception on any error */
    virtual void fromJsonByteArrayZipped(const QByteArray&);

    /** Stores the json string to a file.
        Uses toJsonString().
        @throws QProps::Exception on any error */
    virtual void saveJsonFile(
            const QString& filename, bool compact = true) const;

    /** Initializes this object's data from a json file.
        Uses fromJsonString().
        @throws QProps::Exception on any error */
    virtual void loadJsonFile(const QString& filename);

};


} // namespace QProps

#endif // QPROPS_SRC_JSONINTERFACE_H
