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

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QString>
#include <QColor>
#include <QRectF>

#include "io/JsonInterface.h"
#include "io/error.h"

#define SONOTE_JSON_ERROR(arg__) \
    SONOT_IO_ERROR("[JSON: " << p_classname_ << "] " << arg__)

namespace Sonot {

QString JsonInterface::toJsonString(bool compact) const
{
    QJsonDocument doc(toJson());
    return QString::fromUtf8(doc.toJson(
                compact ? QJsonDocument::Compact : QJsonDocument::Indented));
}

void JsonInterface::fromJsonString(const QString &jsonString)
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError)
        SONOT_IO_ERROR("Error parsing json string: "
                        << error.errorString().toStdString());
    auto main = doc.object();
    fromJson(main);
}


void JsonInterface::saveJsonFile(const QString& filename, bool compact) const
{
    QString js = toJsonString(compact);

    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
        SONOT_IO_ERROR("Could not open '"
                      << filename.toStdString() << "' for writing,\n"
                      << file.errorString().toStdString());

    file.write(js.toUtf8());
}

void JsonInterface::loadJsonFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        SONOT_IO_ERROR("Could not open '"
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
        //static T from(const QJsonValue& v) { }
    };

    template <>
    struct JsonValueTraits<size_t>
    {
        static QJsonValue to(size_t v) { return QJsonValue((qint64)v); }
        //static size_t from(const QJsonValue& v) { return v.toInt(); }
    };

    template <>
    struct JsonValueTraits<QRectF>
    {
        static QJsonValue to(const QRectF& r)
        {
            QJsonArray a;
            a.append(r.left());
            a.append(r.top());
            a.append(r.width());
            a.append(r.height());
            return a;
        }
    };

    template <>
    struct JsonValueTraits<QSizeF>
    {
        static QJsonValue to(const QSizeF& r)
        {
            QJsonArray a;
            a.append(r.width());
            a.append(r.height());
            return a;
        }
    };

    template <>
    struct JsonValueTraits<QColor>
    {
        static QJsonValue to(const QColor& c)
        {
            return QJsonValue(QString("#%1").arg(c.rgba(), 8, 16, QChar('0')));
        }
    };
}




const char* JsonHelper::typeName(const QJsonValue& v)
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
double JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isDouble())
        SONOTE_JSON_ERROR("Expected double value, got " << typeName(v));
    return v.toDouble();
}

template <>
float JsonHelper::expect(const QJsonValue& v)
{
    return expect<double>(v);
}

template <>
int JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isDouble() && !v.isString())
        SONOTE_JSON_ERROR("Expected int value, got " << typeName(v));
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
size_t JsonHelper::expect(const QJsonValue& v)
{
    return expect<int>(v);
}

template <>
QString JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        SONOTE_JSON_ERROR("Expected string value, got " << typeName(v));
    return v.toString();
}

template <>
QRectF JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isArray())
        SONOTE_JSON_ERROR("Expected json array (of rect), got " << typeName(v));
    auto ja = v.toArray();
    if (ja.size() < 4)
        SONOTE_JSON_ERROR("Expected json array of length 4, got length " << ja.size());
    std::vector<double> a;
    fromArray(a, ja);
    return QRectF(a[0], a[1], a[2], a[3]);
}

template <>
QSizeF JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isArray())
        SONOTE_JSON_ERROR("Expected json array (of size), got " << typeName(v));
    auto ja = v.toArray();
    if (ja.size() < 2)
        SONOTE_JSON_ERROR("Expected json array of length 2, got length " << ja.size());
    std::vector<double> a;
    fromArray(a, ja);
    return QSizeF(a[0], a[1]);
}


template <>
QColor JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        SONOTE_JSON_ERROR("Expected color string, got " << typeName(v));
    QString s = expect<QString>(v);
    bool ok;
    unsigned int c = s.mid(1).toUInt(&ok, 16);
    if (!ok)
        SONOTE_JSON_ERROR("Error parsing color string '" << s << "'");
    return QColor::fromRgba(c);
}



template <typename T>
T JsonHelper::expectChild(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        SONOTE_JSON_ERROR("Expected '" << key << "' value ("
                          << typeid(T).name() << "), not found");
    return expect<T>( parent.value(key) );
}


QJsonArray JsonHelper::expectArray(const QJsonValue& v)
{
    if (!v.isArray())
        SONOTE_JSON_ERROR("Expected json array, got " << typeName(v));
    return v.toArray();
}

QJsonObject JsonHelper::expectObject(const QJsonValue& v)
{
    if (!v.isObject())
        SONOTE_JSON_ERROR("Expected json object, got " << typeName(v));
    return v.toObject();
}

QJsonValue JsonHelper::expectChildValue(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        SONOTE_JSON_ERROR("Expected '" << key << "' value, not found");
    return parent.value(key);
}

QJsonObject JsonHelper::expectChildObject(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        SONOTE_JSON_ERROR("Expected '" << key << "' object, not found");
    return expectObject(parent.value(key));
}




template <typename T>
QJsonArray JsonHelper::toArray(const std::vector<T>& data)
{
    QJsonArray a;
    for (T v : data)
        a.append(JsonValueTraits<T>::to(v));
    return a;
}

template <typename T>
void JsonHelper::fromArray(std::vector<T>& dst, const QJsonArray& src)
{
    dst.resize(src.size());
    for (size_t i=0; i<dst.size(); ++i)
        dst[i] = expect<T>(src.at(i));
}

template <typename T>
void JsonHelper::fromArray(std::vector<T>& dst, const QJsonValue& src)
{
    fromArray(dst, expectArray(src));
}

template <typename T>
QJsonValue JsonHelper::wrap(const T& r)
{
    return JsonValueTraits<T>::to(r);
}


// --- template instantiation ---

#define SONOT__INSTANTIATE(T__) \
template QJsonValue JsonHelper::wrap<T__>(const T__&);

SONOT__INSTANTIATE(QRectF);
SONOT__INSTANTIATE(QSizeF);
SONOT__INSTANTIATE(QColor);

#undef SONOT__INSTANTIATE
#define SONOT__INSTANTIATE(T__) \
template T__ JsonHelper::expectChild<T__>(const QJsonObject& parent, const QString& key); \
template QJsonArray JsonHelper::toArray<T__>( const std::vector<T__>&); \
template void JsonHelper::fromArray<T__>(std::vector<T__>& dst, const QJsonArray& src); \
template void JsonHelper::fromArray<T__>(std::vector<T__>& dst, const QJsonValue& src);

SONOT__INSTANTIATE(int)
SONOT__INSTANTIATE(float)
SONOT__INSTANTIATE(double)
SONOT__INSTANTIATE(size_t)
SONOT__INSTANTIATE(QString)
SONOT__INSTANTIATE(QRectF)
SONOT__INSTANTIATE(QSizeF)
SONOT__INSTANTIATE(QColor)


#undef SONOT__INSTANTIATE

} // namespace Sonot
