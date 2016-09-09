/********************* ******************************************************

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
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QVariant>
#include <QString>
#include <QColor>
#include <QRect>
#include <QRectF>
#include <QLine>
#include <QLineF>
#include <QFont>
#include <QTime>
#include <QDateTime>

#include "io/JsonInterface.h"
#include "io/error.h"

#define SONOT_JSON_ERROR(arg__) \
    { const QString ctx__ = context(); \
      if (ctx__.isEmpty()) \
        SONOT_IO_ERROR("[JSON: " << p_classname_ << "] " << arg__) \
      else \
        SONOT_IO_ERROR("[JSON: " << p_classname_ << " | " \
            << ctx__ << "] " << arg__); }

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



// #################### JSON object conversion ######################

QJsonArray JsonHelper::expectArray(const QJsonValue& v)
{
    if (!v.isArray())
        SONOT_JSON_ERROR("Expected json array, got " << typeName(v));
    return v.toArray();
}

QJsonObject JsonHelper::expectObject(const QJsonValue& v)
{
    if (!v.isObject())
        SONOT_JSON_ERROR("Expected json object, got " << typeName(v));
    return v.toObject();
}

QJsonValue JsonHelper::expectChildValue(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        SONOT_JSON_ERROR("Expected '" << key << "' value, not found");
    return parent.value(key);
}

QJsonArray JsonHelper::expectChildArray(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        SONOT_JSON_ERROR("Expected '" << key << "' array, not found");
    return expectArray(parent.value(key));
}

QJsonObject JsonHelper::expectChildObject(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        SONOT_JSON_ERROR("Expected '" << key << "' object, not found");
    return expectObject(parent.value(key));
}



// ########################## to JSON ################################

namespace
{
    /** Converter for arguments acceptable by QJsonValue constructor */
    template <typename T>
    QJsonValue to_json(T v) { return QJsonValue(v); }

    QJsonValue to_json(int8_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(uint8_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(int16_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(uint16_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(int32_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(uint32_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(int64_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(uint64_t v) { return QJsonValue((double)v); }

    QJsonValue to_json(const QRectF& r)
    {
        QJsonArray a;
        a << r.left() << r.top() << r.width() << r.height();
        return a;
    }

    QJsonValue to_json(const QRect& r)
    {
        QJsonArray a;
        a << r.left() << r.top() << r.width() << r.height();
        return a;
    }

    QJsonValue to_json(const QSizeF& r)
    {
        QJsonArray a;
        a << r.width() << r.height();
        return a;
    }

    QJsonValue to_json(const QSize& r)
    {
        QJsonArray a;
        a << r.width() << r.height();
        return a;
    }

    QJsonValue to_json(const QPointF& r)
    {
        QJsonArray a;
        a << r.x() << r.y();
        return a;
    }

    QJsonValue to_json(const QPoint& r)
    {
        QJsonArray a;
        a << r.x() << r.y();
        return a;
    }

    QJsonValue to_json(const QLineF& r)
    {
        QJsonArray a;
        a << r.x1() << r.y1() << r.x2() << r.y2();
        return a;
    }

    QJsonValue to_json(const QLine& r)
    {
        QJsonArray a;
        a << r.x1() << r.y1() << r.x2() << r.y2();
        return a;
    }

    QJsonValue to_json(const QColor& c)
    {
        //return QJsonValue(QString("#%1").arg(c.rgba(), 8, 16, QChar('0')));
        QJsonArray a;
        a << c.red() << c.green() << c.blue() << c.alpha();
        return a;
    }

    QJsonValue to_json(const QFont& f)
    {
        return QJsonValue(f.toString());
    }

    /** Need to implement this because QVariant's version
        does not store milliseconds */
    QJsonValue to_json(const QTime& t)
    {
        return QJsonValue(t.toString("hh:mm:ss.zzz"));
    }

    /** Need to implement this because QVariant's version
        does not store milliseconds */
    QJsonValue to_json(const QDateTime& t)
    {
        return QJsonValue(t.toString("yyyy-MM-dd hh:mm:ss.zzz"));
    }

}




template <typename T>
QJsonArray JsonHelper::toArray(const std::vector<T>& data)
{
    QJsonArray a;
    for (const T& v : data)
        a.append(to_json(v));
    return a;
}

template <typename T>
QJsonValue JsonHelper::wrap(const T& r)
{
    return to_json(r);
}

QJsonValue JsonHelper::wrap(const QVariant& v, bool explicitType)
{
    // store with QVariant conversion
    if (!explicitType)
    {
        auto jv = QJsonValue::fromVariant(v);
        if (!jv.isNull())
            return jv;
    }

    // handle compound types supported by QVariant
    QJsonObject o;
    o.insert("t", QString(v.typeName()));
    switch (v.type())
    {
        case QVariant::Color:
            o.insert("v", to_json(v.value<QColor>()));
        break;
        case QVariant::RectF:
            o.insert("v", to_json(v.value<QRectF>()));
        break;
        case QVariant::SizeF:
            o.insert("v", to_json(v.value<QSizeF>()));
        break;
        case QVariant::PointF:
            o.insert("v", to_json(v.value<QPointF>()));
        break;
        case QVariant::LineF:
            o.insert("v", to_json(v.value<QLineF>()));
        break;
        case QVariant::Rect:
            o.insert("v", to_json(v.value<QRect>()));
        break;
        case QVariant::Size:
            o.insert("v", to_json(v.value<QSize>()));
        break;
        case QVariant::Line:
            o.insert("v", to_json(v.value<QLine>()));
        break;
        case QVariant::Point:
            o.insert("v", to_json(v.value<QPoint>()));
        break;
        case QVariant::Font:
            o.insert("v", to_json(v.value<QFont>()));
        break;
        case QVariant::Time:
            o.insert("v", to_json(v.value<QTime>()));
        break;
        case QVariant::DateTime:
            o.insert("v", to_json(v.value<QDateTime>()));
        break;
        default:
        {
            QJsonValue jv = QJsonValue::fromVariant(v);
            if (jv.isNull())
                SONOT_JSON_ERROR("Can't save QVariant::" << v.typeName()
                             << "(" << v.type() << ") to json, "
                             "type not implemented!");
            o.insert("v", jv);
        }
        break;
    }

    return o;
}
















// ########################## from JSON #####################

/** Internal helper that expects an array of specific length */
template <typename T>
void JsonHelper::p_expectArray_(
        const QJsonValue &src, std::vector<T> &dst,
        size_t size, const QString &forType)
{
    if (!src.isArray())
        SONOT_JSON_ERROR("Expected json array (" << forType << "), got "
                         << typeName(src));
    auto ja = src.toArray();
    if (ja.size() != size)
        SONOT_JSON_ERROR("Expected json array of length 4, got length "
                         << ja.size());
    fromArray(dst, ja);
}


// expect specific types

template <>
double JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isDouble())
        SONOT_JSON_ERROR("Expected double value, got " << typeName(v));
    return v.toDouble();
}

template <>
float JsonHelper::expect(const QJsonValue& v)
{
    return expect<double>(v);
}

template <>
int64_t JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isDouble() && !v.isString())
        SONOT_JSON_ERROR("Expected int value, got " << typeName(v));
    if (v.isString())
    {
        bool ok;
        int k = v.toString().toLongLong(&ok);
        if (!ok)
            SONOT_JSON_ERROR("Expected int value, got non-int string '"
                        << v.toString() << "'");
        return k;
    }
    return v.toInt();
}

template <>
int8_t JsonHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
uint8_t JsonHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
int16_t JsonHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
uint16_t JsonHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
int32_t JsonHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
uint32_t JsonHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
uint64_t JsonHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }


template <>
QString JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        SONOT_JSON_ERROR("Expected string value, got " << typeName(v));
    return v.toString();
}

// -- compound types --
// (typically stored as array)

#define SONOT__EXPECT_COMPOUND_ARRAY2(QType__, Type__) \
    template <> \
    QType__ JsonHelper::expect(const QJsonValue& v) \
    { \
        std::vector<Type__> a; \
        p_expectArray_(v, a, 2, #QType__); \
        return QType__(a[0], a[1]); \
    }

#define SONOT__EXPECT_COMPOUND_ARRAY4(QType__, Type__) \
    template <> \
    QType__ JsonHelper::expect(const QJsonValue& v) \
    { \
        std::vector<Type__> a; \
        p_expectArray_(v, a, 4, #QType__); \
        return QType__(a[0], a[1], a[2], a[3]); \
    }

SONOT__EXPECT_COMPOUND_ARRAY2(QPoint, int)
SONOT__EXPECT_COMPOUND_ARRAY2(QSize,  int)
SONOT__EXPECT_COMPOUND_ARRAY4(QLine,  int)
SONOT__EXPECT_COMPOUND_ARRAY4(QRect,  int)
SONOT__EXPECT_COMPOUND_ARRAY4(QColor, int)
SONOT__EXPECT_COMPOUND_ARRAY2(QPointF, double)
SONOT__EXPECT_COMPOUND_ARRAY2(QSizeF,  double)
SONOT__EXPECT_COMPOUND_ARRAY4(QLineF,  double)
SONOT__EXPECT_COMPOUND_ARRAY4(QRectF,  double)


template <>
QFont JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        SONOT_JSON_ERROR("Expected font description, got " << typeName(v));

    QFont f;
    if (!f.fromString(v.toString()))
        SONOT_JSON_ERROR("Can not parse QFont description '"
                         << v.toString() << "'");
    return f;
}

template <>
QTime JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        SONOT_JSON_ERROR("Expected time string, got " << typeName(v));

    QTime t = QTime::fromString(v.toString(), "h:m:s.z");
    if (!t.isValid())
        SONOT_JSON_ERROR("Can not parse time string '"
                         << v.toString() << "'");
    return t;
}

template <>
QDateTime JsonHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        SONOT_JSON_ERROR("Expected date-time string, got " << typeName(v));

    QDateTime t = QDateTime::fromString(v.toString(), "yyyy-M-d h:m:s.z");
    if (!t.isValid())
        SONOT_JSON_ERROR("Can not parse date-time string '"
                         << v.toString() << "'");
    return t;
}



QVariant JsonHelper::expectQVariant(
        const QJsonValue& v, QVariant::Type expected)
{
    QVariant ret;

    if (!v.isObject())
    {
        // use built-in QJsonValue->QVariant
        ret = v.toVariant();
        if (!ret.isValid())
            SONOT_JSON_ERROR("Expected QVariant compatible json value, "
                             "got '" << typeName(v) << "'");
        // explicit type conversion
        if (expected != QVariant::Invalid)
        {
            if (expected != ret.type() && !ret.convert(expected))
            {
                SONOT_IO_ERROR("Can't convert value type '" << ret.typeName()
                               << "' in json to '"
                               << QVariant::typeToName(expected) << "'");
            }
        }
    }
    // convert special JsonObject->QVariant
    else
    {
        QJsonObject o = v.toObject();
        const QString typeName = expectChild<QString>(o, "t");
        QVariant::Type type =
                QVariant::nameToType(typeName.toStdString().c_str());
        if (type == QVariant::Invalid)
            SONOT_JSON_ERROR("Illegal QVariant type '" << typeName << "' in "
                             "json object");
        switch (type)
        {
            case QVariant::Color:  ret = expectChild<QColor>(o, "v"); break;
            case QVariant::Rect:   ret = expectChild<QRect>(o, "v"); break;
            case QVariant::Size:   ret = expectChild<QSize>(o, "v"); break;
            case QVariant::Point:  ret = expectChild<QPoint>(o, "v"); break;
            case QVariant::Line:   ret = expectChild<QLine>(o, "v"); break;
            case QVariant::RectF:  ret = expectChild<QRectF>(o, "v"); break;
            case QVariant::SizeF:  ret = expectChild<QSizeF>(o, "v"); break;
            case QVariant::PointF: ret = expectChild<QPointF>(o, "v"); break;
            case QVariant::LineF:  ret = expectChild<QLineF>(o, "v"); break;
            case QVariant::Font:   ret = expectChild<QFont>(o, "v"); break;
            case QVariant::Time:   ret = expectChild<QTime>(o, "v"); break;
            case QVariant::DateTime:ret = expectChild<QDateTime>(o, "v"); break;
            default:
                // check for Qt's QVariant conversion
                ret = expectChildValue(o, "v").toVariant();
                if (ret.isNull())
                    SONOT_JSON_ERROR("Unsupported QVariant type '" << typeName
                                 << "' in json object");
                //qDebug() << "EXPECT" << typeName << "GOT" << ret.typeName();

                // convert to explicit type (e.g. double -> int)
                if (ret.type() != type && !ret.convert(type))
                    SONOT_JSON_ERROR("Could not convert QVariant type '"
                                     << ret.typeName() << "' to '"
                                     << typeName << "'");
            break;
        }
    }

    // explicit type conversion
    if (expected != QVariant::Invalid)
    {
        if (expected != ret.type() && !ret.convert(expected))
        {
            SONOT_IO_ERROR("Can't convert value type '" << ret.typeName()
                           << "' in json to '"
                           << QVariant::typeToName(expected) << "'");
        }
    }

    return ret;
}

// wrapper for expect()
template <typename T>
T JsonHelper::expectChild(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        SONOT_JSON_ERROR("Expected '" << key << "' value ("
                          << typeid(T).name() << "), not found");
    return expect<T>( parent.value(key) );
}



QVariant JsonHelper::expectChildQVariant(
        const QJsonObject& parent, const QString& key,
        QVariant::Type expectedType)
{
    if (!parent.contains(key))
        SONOT_JSON_ERROR("Expected '" << key << "' qvariant, not found");
    return expectQVariant(parent.value(key), expectedType);
}


// -- wrapper of expect() for arrays --

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



// --- template instantiation ---

// All compound types supported by QVariant
// that are excplicitly handled in code above
#define SONOT__FOR_EACH_SUPPORTED_QT_TYPE(F__) \
    F__(QRect) \
    F__(QRectF) \
    F__(QSize) \
    F__(QSizeF) \
    F__(QPoint) \
    F__(QPointF) \
    F__(QLine) \
    F__(QLineF) \
    F__(QColor) \
    F__(QTime) \
    F__(QDateTime)



// -- inst. JsonHelper::wrap() --

#define SONOT__INSTANTIATE(T__) \
    template QJsonValue JsonHelper::wrap<T__>(const T__&);

    SONOT__FOR_EACH_SUPPORTED_QT_TYPE( SONOT__INSTANTIATE )

#undef SONOT__INSTANTIATE


// -- inst. JsonHelper::expectChild(), toArray, fromArray() --

#define SONOT__INSTANTIATE(T__) \
    template T__ JsonHelper::expectChild<T__>(const QJsonObject& parent, const QString& key); \
    template QJsonArray JsonHelper::toArray<T__>( const std::vector<T__>&); \
    template void JsonHelper::fromArray<T__>(std::vector<T__>& dst, const QJsonArray& src); \
    template void JsonHelper::fromArray<T__>(std::vector<T__>& dst, const QJsonValue& src);

    SONOT__INSTANTIATE(float)
    SONOT__INSTANTIATE(double)
    SONOT__INSTANTIATE(int8_t)
    SONOT__INSTANTIATE(uint8_t)
    SONOT__INSTANTIATE(int16_t)
    SONOT__INSTANTIATE(uint16_t)
    SONOT__INSTANTIATE(int32_t)
    SONOT__INSTANTIATE(uint32_t)
    SONOT__INSTANTIATE(int64_t)
    SONOT__INSTANTIATE(uint64_t)
    SONOT__INSTANTIATE(QString)

    SONOT__FOR_EACH_SUPPORTED_QT_TYPE( SONOT__INSTANTIATE )

#undef SONOT__INSTANTIATE



} // namespace Sonot
