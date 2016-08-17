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

#ifndef SONOTSRC_JSONINTERFACE_H
#define SONOTSRC_JSONINTERFACE_H

#include <vector>
#include <QString>

class QJsonObject;
class QJsonArray;
class QJsonValue;
class QRectF;
class QColor;

namespace Sonot {

/** Interface for loading/saving json data.
    Users of the interface need to implement the
    toJson() and fromJson() methods.
    This enables converting to/from json strings and file io.
*/
class JsonInterface
{
public:

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

};

/** Collection of helper functions for json parsing.
    Most functions throw Exception with the className defined in the constructor */
class JsonHelper
{
public:

    JsonHelper(const char* className)
        : p_json_helper_classname_(className) { }
    JsonHelper(const QString& className)
        : p_json_helper_classname_(className) { }

    // --- info ---

    static const char* typeName(const QJsonValue&);

    // -- convert to json --

    static QJsonValue wrap(const QRectF&);
    static QJsonValue wrap(const QColor&);

    template <typename T>
    static QJsonArray toArray(const std::vector<T>&);


    // -- throwing getters --

    template <typename T>
    T expect(const QJsonValue&);

    template <typename T>
    T expectChild(const QJsonObject& parent, const QString& key);

    QJsonValue expectChildValue(const QJsonObject& parent, const QString& key);
    QJsonArray expectArray(const QJsonValue&);

    template <typename T>
    void fromArray(std::vector<T>& dst, const QJsonArray& src);

    template <typename T>
    void fromArray(std::vector<T>& dst, const QJsonValue& src);

private:
    QString p_json_helper_classname_;
};





} // namespace Sonot

#endif // SONOTSRC_JSONINTERFACE_H
