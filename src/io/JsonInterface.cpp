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

#include <cstddef>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QFile>
#include <QRectF>

#include "io/JsonInterface.h"
#include "io/error.h"

#define SONOTE_JSON_ERROR(arg__) \
    SONOTE_IO_ERROR("[JSON: " << p_json_iface_classname_ << "] " << arg__)

namespace Sonote {

QString JsonInterface::toJsonString() const
{
    QJsonDocument doc(toJson());
    return QString::fromUtf8(doc.toJson());
}

void JsonInterface::fromJsonString(const QString &jsonString)
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError)
        SONOTE_JSON_ERROR("Error parsing json string: "
                        << error.errorString().toStdString());
    auto main = doc.object();
    fromJson(main);
}


void JsonInterface::saveJsonFile(const QString& filename) const
{
    QString js = toJsonString();

    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
        SONOTE_JSON_ERROR("Could not open '"
                      << filename.toStdString() << "' for writing,\n"
                      << file.errorString().toStdString());

    file.write(js.toUtf8());
}

void JsonInterface::loadJsonFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        SONOTE_JSON_ERROR("Could not open '"
                      << filename.toStdString() << "' for reading,\n"
                      << file.errorString().toStdString());

    auto js = QString::fromUtf8(file.readAll());
    file.close();

    fromJsonString(js);
}



// ---------------------------- HELPER -------------------------------

namespace
{
    /** Converter for arguments acceptable by QJsonValue constructor */
    template <typename T>
    struct JsonValueTraits
    {
        static QJsonValue to(T v) { return QJsonValue(v); }
    };

    template <>
    struct JsonValueTraits<size_t>
    {
        static QJsonValue to(size_t v) { return QJsonValue((qint64)v); }
    };
}




const char* JsonInterface::json_typeName(const QJsonValue& v)
{
    switch (v.type())
    {
        case QJsonValue::Null: return "NULL";
        case QJsonValue::Bool: return "bool";
        case QJsonValue::Double: return "double";
        case QJsonValue::String: return "string";
        case QJsonValue::Array: return "array";
        case QJsonValue::Object: return "object";
        case QJsonValue::Undefined: return "undefined";
    }
    return "*undefined*";
}

template <>
double JsonInterface::json_expect(const QJsonValue& v)
{
    if (!v.isDouble())
        SONOTE_JSON_ERROR("Expected double value, got " << json_typeName(v));
    return v.toDouble();
}

template <>
int JsonInterface::json_expect(const QJsonValue& v)
{
    if (!v.isDouble() && !v.isString())
        SONOTE_JSON_ERROR("Expected int value, got " << json_typeName(v));
    if (v.isString())
    {
        bool ok;
        int k = v.toString().toInt(&ok);
        if (!ok)
            SONOTE_JSON_ERROR("Expected int value, got non-int string '"
                        << v.toString() << "'");
        return k;
    }
    return v.toInt();
}

template <>
float JsonInterface::json_expect(const QJsonValue& v)
{
    return json_expect<double>(v);
}

template <>
size_t JsonInterface::json_expect(const QJsonValue& v)
{
    return json_expect<int>(v);
}

template <>
QRectF JsonInterface::json_expect(const QJsonValue& v)
{
    if (!v.isArray())
        SONOTE_JSON_ERROR("Expected json array (of rect), got " << json_typeName(v));
    auto ja = v.toArray();
    if (ja.size() < 4)
        SONOTE_JSON_ERROR("Expected json array of length 4, got length " << ja.size());
    std::vector<double> a;
    json_fromArray(a, ja);
    return QRectF(a[0], a[1], a[2], a[3]);
}


QJsonArray JsonInterface::json_expectArray(const QJsonValue& v)
{
    if (!v.isArray())
        SONOTE_JSON_ERROR("Expected json array, got " << json_typeName(v));
    return v.toArray();
}

QJsonValue JsonInterface::json_expectChild(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        SONOTE_JSON_ERROR("Expected '" << key << "' object, not found");
    return parent.value(key);
}





template <typename T>
QJsonArray JsonInterface::json_toArray(const std::vector<T>& data)
{
    QJsonArray a;
    for (T v : data)
        a.append(JsonValueTraits<T>::to(v));
    return a;
}

template <typename T>
void JsonInterface::json_fromArray(std::vector<T>& dst, const QJsonArray& src)
{
    dst.resize(src.size());
    for (size_t i=0; i<dst.size(); ++i)
        dst[i] = json_expect<T>(src.at(i));
}

template <typename T>
void JsonInterface::json_fromArray(std::vector<T>& dst, const QJsonValue& src)
{
    json_fromArray(dst, json_expectArray(src));
}

QJsonValue JsonInterface::json_wrap(const QRectF& r)
{
    QJsonArray a;
    a.append(r.left());
    a.append(r.top());
    a.append(r.width());
    a.append(r.height());
    return a;
}


// --- template instantiation ---

#define SONOTE__INSTANTIATE(type__) \
template QJsonArray JsonInterface::json_toArray<type__>( const std::vector<type__>&); \
template void JsonInterface::json_fromArray<type__>(std::vector<type__>& dst, const QJsonArray& src); \
template void JsonInterface::json_fromArray<type__>(std::vector<type__>& dst, const QJsonValue& src);

SONOTE__INSTANTIATE(int)
SONOTE__INSTANTIATE(float)
SONOTE__INSTANTIATE(double)
SONOTE__INSTANTIATE(size_t)


#undef SONOTE__INSTANTIATE

} // namespace Sonote
