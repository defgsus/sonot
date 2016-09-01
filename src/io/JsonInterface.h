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



/** Collection of helper functions for json value conversion.
    Most functions throw Sonot::Exception with the className
    defined in the constructor */
class JsonHelper
{
public:

    JsonHelper(const char* className)
        : p_classname_(className) { }
    JsonHelper(const QString& className)
        : p_classname_(className) { }

    // --- info ---

    /** The name given in constructor */
    const QString& className() const { return p_classname_; }

    /** Returns the name of the json type */
    static const char* typeName(const QJsonValue&);

    // -- convert to json --

    /** @{ */
    /** Wraps the type into a json value.
        Unwrap the value with expect() or expectChild() */
    static QJsonValue wrap(const QRectF&);
    static QJsonValue wrap(const QColor&);
    /** @} */

    /** Converts the vector of type T to a json array.
        Unwrap with fromArray() */
    template <typename T>
    static QJsonArray toArray(const std::vector<T>&);


    // -- throwing getters --

    /** Converts the json value to type T.
        @throws Sonot::Exception if not convertible. */
    template <typename T>
    T expect(const QJsonValue&);

    /** Gets the child value and converts it to type T.
        @throws Sonot::Exception if child not found or not convertible. */
    template <typename T>
    T expectChild(const QJsonObject& parent, const QString& key);

    /** Returns the child value.
        @throws Sonot::Exception if child not found. */
    QJsonValue expectChildValue(const QJsonObject& parent, const QString& key);

    /** Converts the QJsonValue to a QJsonArray.
        @throws Sonot::Exception if not an array. */
    QJsonArray expectArray(const QJsonValue&);

    /** Converts the QJsonValue to a QJsonObject.
        @throws Sonot::Exception if not an object. */
    QJsonObject expectObject(const QJsonValue&);

    /** Converts the json array to a vector of type T.
        Previous contents of @p dst are erased.
        @throws Sonot::Exception if an element is not convertible to T. */
    template <typename T>
    void fromArray(std::vector<T>& dst, const QJsonArray& src);

    /** Converts the json value to a vector of type T.
        Previous contents of @p dst are erased.
        @throws Sonot::Exception if @p src is not an array
        or an element is not convertible to T. */
    template <typename T>
    void fromArray(std::vector<T>& dst, const QJsonValue& src);

private:
    QString p_classname_;
};





} // namespace Sonot

#endif // SONOTSRC_JSONINTERFACE_H
