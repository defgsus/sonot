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

#ifndef SONOTESRC_JSONINTERFACE_H
#define SONOTESRC_JSONINTERFACE_H

#include <vector>

class QJsonObject;
class QJsonArray;
class QJsonValue;
class QString;
class QRectF;

namespace Sonote {

/** Interface for loading/saving json data.
    Users of the interface need to implement the
    toJson() and fromJson() methods.
    This enables converting to/from json strings and file io.
*/
class JsonInterface
{
public:

    JsonInterface(const char* className)
        : p_json_iface_classname_(className) { }

    virtual ~JsonInterface() { }

    // ----------- pure methods --------------

    /** Return a QJsonObject with all data inside.
        Should throw a descriptive IoException on any errors. */
    virtual QJsonObject toJson() const = 0;

    /** Initializes the object from the QJsonObject.
        Should throw a descriptive IoException on severe errors. */
    virtual void fromJson(const QJsonObject&) = 0;

    // ------------ convenience --------------

    /** Converts this object's data to a Json string.
        Uses toJson().
        @throws IoException on any error */
    virtual QString toJsonString() const;

    /** Initializes this objects's data from a json string.
        Uses fromJson().
        @throws IoException on any error */
    virtual void fromJsonString(const QString&);

    /** Stores the json string to a file.
        Uses toJsonString().
        @throws IoException on any error */
    virtual void saveJsonFile(const QString& filename) const;

    /** Initializes this object's data from a json file.
        Uses fromJsonString().
        @throws IoException on any error */
    virtual void loadJsonFile(const QString& filename);

    /* ---- (static) helper functions ----- */

    static const char* json_typeName(const QJsonValue&);

    template <typename T>
    T json_expect(const QJsonValue&);

    QJsonValue json_expectChild(const QJsonObject& parent, const QString& key);
    QJsonArray json_expectArray(const QJsonValue&);

    template <typename T>
    static QJsonArray json_toArray(const std::vector<T>&);

    template <typename T>
    void json_fromArray(std::vector<T>& dst, const QJsonArray& src);

    template <typename T>
    void json_fromArray(std::vector<T>& dst, const QJsonValue& src);

    static QJsonValue json_wrap(const QRectF&);

private:
    const char* p_json_iface_classname_;
};

} // namespace Sonote

#endif // SONOTESRC_JSONINTERFACE_H
