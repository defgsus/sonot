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

#include "JsonInterfaceHelper.h"
#include "error.h"

#define QPROPS_JSON_ERROR(arg__) \
    { const QString ctx__ = context(); \
      if (ctx__.isEmpty()) \
        QPROPS_IO_ERROR("[JSON: " << p_classname_ << "] " << arg__) \
      else \
        QPROPS_IO_ERROR("[JSON: " << p_classname_ << " | " \
            << ctx__ << "] " << arg__); }

namespace QProps {


const char* JsonInterfaceHelper::typeName(const QJsonValue& v)
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

QJsonArray JsonInterfaceHelper::expectArray(const QJsonValue& v)
{
    if (!v.isArray())
        QPROPS_JSON_ERROR("Expected json array, got " << typeName(v));
    return v.toArray();
}

QJsonObject JsonInterfaceHelper::expectObject(const QJsonValue& v)
{
    if (!v.isObject())
        QPROPS_JSON_ERROR("Expected json object, got " << typeName(v));
    return v.toObject();
}

QJsonValue JsonInterfaceHelper::expectChildValue(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        QPROPS_JSON_ERROR("Expected '" << key << "' value, not found");
    return parent.value(key);
}

QJsonArray JsonInterfaceHelper::expectChildArray(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        QPROPS_JSON_ERROR("Expected '" << key << "' array, not found");
    return expectArray(parent.value(key));
}

QJsonObject JsonInterfaceHelper::expectChildObject(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        QPROPS_JSON_ERROR("Expected '" << key << "' object, not found");
    return expectObject(parent.value(key));
}


// ###################### supported types ############################

/// @note this is copied from qmetatype.h
#define QPROPS__FOR_EACH_PRIMITIVE_TYPE(F__) \
    F__(Bool, 1, bool) \
    F__(Int, 2, int) \
    F__(UInt, 3, uint) \
    F__(LongLong, 4, qlonglong) \
    F__(ULongLong, 5, qulonglong) \
    F__(Double, 6, double) \
    F__(Long, 32, long) \
    F__(Short, 33, short) \
    F__(Char, 34, char) \
    F__(ULong, 35, ulong) \
    F__(UShort, 36, ushort) \
    F__(UChar, 37, uchar) \
    F__(Float, 38, float) \
    F__(SChar, 40, signed char) \

// All compound types supported by QVariant
// that are excplicitly handled in code
#define QPROPS__FOR_EACH_COMPOUND_TYPE(F__) \
    F__(QColor) \
    F__(QRect) \
    F__(QRectF) \
    F__(QSize) \
    F__(QSizeF) \
    F__(QPoint) \
    F__(QPointF) \
    F__(QLine) \
    F__(QLineF) \
    F__(QFont) \
    F__(QTime) \
    F__(QDateTime)





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

    template <typename T>
    QJsonValue to_json(const QVector<T>& v)
    {
        QJsonArray a;
        for (int i=0; i<v.size(); ++i)
            a << to_json(v[i]);
        return a;
    }

}




template <typename T>
QJsonArray JsonInterfaceHelper::toArray(const std::vector<T>& data)
{
    QJsonArray a;
    for (const T& v : data)
        a.append(to_json(v));
    return a;
}

template <typename T>
QJsonValue JsonInterfaceHelper::wrap(const T& r)
{
    return to_json(r);
}

QJsonValue JsonInterfaceHelper::wrap(const QVariant& v, bool explicitType)
{
    // store with QVariant conversion
    if (!explicitType)
    {
        auto jv = QJsonValue::fromVariant(v);
        if (!jv.isNull())
            return jv;
    }

    // -- handle compound types supported by QVariant --

    const QString typeName = QString(v.typeName());
    QJsonObject o;
    o.insert("t", typeName);

    // - store QVariant containing QVector -

#define QPROPS__ARRAY_WRITE_IMPL(Array__, Type__) \
    if (typeName == #Array__ "<" #Type__ ">") \
    { \
        o.insert("v", to_json(v.value<Array__<Type__>>())); \
    } else

#define QPROPS__ARRAY_WRITE(Type__) QPROPS__ARRAY_WRITE_IMPL(QVector, Type__)
    QPROPS__FOR_EACH_COMPOUND_TYPE( QPROPS__ARRAY_WRITE )
#undef QPROPS__ARRAY_WRITE
#define QPROPS__ARRAY_WRITE(Typename__, Enum__, Type__) \
    QPROPS__ARRAY_WRITE_IMPL(QVector, Type__)
    QPROPS__FOR_EACH_PRIMITIVE_TYPE( QPROPS__ARRAY_WRITE )
#undef QPROPS__ARRAY_WRITE

#undef QPROPS__ARRAY_WRITE_IMPL

    switch (QMetaType::Type(v.type()))
    {
#define QPROPS__QVARIANT_CASE(Type__) \
        case QMetaType::Type__: \
            o.insert("v", to_json(v.value<Type__>())); \
        break;

        QPROPS__FOR_EACH_COMPOUND_TYPE( QPROPS__QVARIANT_CASE )

#undef QPROPS__QVARIANT_CASE

        default:
        {
            // use QVariant's inbuilt conversion
            QJsonValue jv = QJsonValue::fromVariant(v);
            if (jv.isNull())
                QPROPS_JSON_ERROR("Can't save QVariant::" << v.typeName()
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
void JsonInterfaceHelper::p_expectArray_(
        const QJsonValue &src, std::vector<T> &dst,
        size_t size, const QString &forType)
{
    if (!src.isArray())
        QPROPS_JSON_ERROR("Expected json array (" << forType << "), got "
                         << typeName(src));
    auto ja = src.toArray();
    if (ja.size() != size)
        QPROPS_JSON_ERROR("Expected json array of length 4, got length "
                         << ja.size());
    fromArray(dst, ja);
}


// expect specific types

template <>
double JsonInterfaceHelper::expect(const QJsonValue& v)
{
    if (!v.isDouble())
        QPROPS_JSON_ERROR("Expected double value, got " << typeName(v));
    return v.toDouble();
}

template <>
float JsonInterfaceHelper::expect(const QJsonValue& v)
{
    return expect<double>(v);
}

template <>
int64_t JsonInterfaceHelper::expect(const QJsonValue& v)
{
    if (!v.isDouble() && !v.isString())
        QPROPS_JSON_ERROR("Expected int value, got " << typeName(v));
    if (v.isString())
    {
        bool ok;
        int k = v.toString().toLongLong(&ok);
        if (!ok)
            QPROPS_JSON_ERROR("Expected int value, got non-int string '"
                        << v.toString() << "'");
        return k;
    }
    return v.toInt();
}

template <>
int8_t JsonInterfaceHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
uint8_t JsonInterfaceHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
int16_t JsonInterfaceHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
uint16_t JsonInterfaceHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
int32_t JsonInterfaceHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
uint32_t JsonInterfaceHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }
template <>
uint64_t JsonInterfaceHelper::expect(const QJsonValue& v) { return expect<int64_t>(v); }


template <>
QString JsonInterfaceHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        QPROPS_JSON_ERROR("Expected string value, got " << typeName(v));
    return v.toString();
}

// -- compound types --
// (typically stored as array)

#define QPROPS__EXPECT_COMPOUND_ARRAY2(QType__, Type__) \
    template <> \
    QType__ JsonInterfaceHelper::expect(const QJsonValue& v) \
    { \
        std::vector<Type__> a; \
        p_expectArray_(v, a, 2, #QType__); \
        return QType__(a[0], a[1]); \
    }

#define QPROPS__EXPECT_COMPOUND_ARRAY4(QType__, Type__) \
    template <> \
    QType__ JsonInterfaceHelper::expect(const QJsonValue& v) \
    { \
        std::vector<Type__> a; \
        p_expectArray_(v, a, 4, #QType__); \
        return QType__(a[0], a[1], a[2], a[3]); \
    }

QPROPS__EXPECT_COMPOUND_ARRAY2(QPoint, int)
QPROPS__EXPECT_COMPOUND_ARRAY2(QSize,  int)
QPROPS__EXPECT_COMPOUND_ARRAY4(QLine,  int)
QPROPS__EXPECT_COMPOUND_ARRAY4(QRect,  int)
QPROPS__EXPECT_COMPOUND_ARRAY4(QColor, int)
QPROPS__EXPECT_COMPOUND_ARRAY2(QPointF, double)
QPROPS__EXPECT_COMPOUND_ARRAY2(QSizeF,  double)
QPROPS__EXPECT_COMPOUND_ARRAY4(QLineF,  double)
QPROPS__EXPECT_COMPOUND_ARRAY4(QRectF,  double)

#undef QPROPS__EXPECT_COMPOUND_ARRAY2
#undef QPROPS__EXPECT_COMPOUND_ARRAY4

template <>
QFont JsonInterfaceHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        QPROPS_JSON_ERROR("Expected font description, got " << typeName(v));

    QFont f;
    if (!f.fromString(v.toString()))
        QPROPS_JSON_ERROR("Can not parse QFont description '"
                         << v.toString() << "'");
    return f;
}

template <>
QTime JsonInterfaceHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        QPROPS_JSON_ERROR("Expected time string, got " << typeName(v));

    QTime t = QTime::fromString(v.toString(), "h:m:s.z");
    if (!t.isValid())
        QPROPS_JSON_ERROR("Can not parse time string '"
                         << v.toString() << "'");
    return t;
}

template <>
QDateTime JsonInterfaceHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        QPROPS_JSON_ERROR("Expected date-time string, got " << typeName(v));

    QDateTime t = QDateTime::fromString(v.toString(), "yyyy-M-d h:m:s.z");
    if (!t.isValid())
        QPROPS_JSON_ERROR("Can not parse date-time string '"
                         << v.toString() << "'");
    return t;
}



QVariant JsonInterfaceHelper::expectQVariant(
        const QJsonValue& v, QVariant::Type expected)
{
    QVariant ret;

    if (!v.isObject())
    {
        // use built-in QJsonValue->QVariant
        ret = v.toVariant();
        if (!ret.isValid())
            QPROPS_JSON_ERROR("Expected QVariant compatible json value, "
                             "got '" << typeName(v) << "'");
        // explicit type conversion
        if (expected != QVariant::Invalid)
        {
            if (expected != ret.type() && !ret.convert(expected))
            {
                QPROPS_IO_ERROR("Can't convert value type '" << ret.typeName()
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
            QPROPS_JSON_ERROR("Illegal QVariant type '" << typeName << "' in "
                             "json object");

#define QPROPS__EXPECT_VEC(Vector__, Type__) \
        if (typeName == #Vector__ "<" #Type__ ">") \
        { \
            QJsonArray ar = expectChildArray(o, "v"); \
            Vector__<Type__> vec; \
            for (int i=0; i<ar.size(); ++i) \
                vec << expect<Type__>(ar.at(i)); \
            ret = QVariant::fromValue(vec); \
        } else

        QPROPS__EXPECT_VEC(QVector, double)
        QPROPS__EXPECT_VEC(QVector, QSizeF)

        switch (QMetaType::Type(type))
        {
#define QPROPS__QVARIANT_CASE(Type__) \
            case QMetaType::Type__: ret = expectChild<Type__>(o, "v"); break;

            QPROPS__FOR_EACH_COMPOUND_TYPE( QPROPS__QVARIANT_CASE )

#undef QPROPS__QVARIANT_CASE

            default:
                // check for Qt's QVariant conversion
                ret = expectChildValue(o, "v").toVariant();
                if (ret.isNull())
                    QPROPS_JSON_ERROR("Unsupported QVariant type '" << typeName
                                 << "' in json object");
                //qDebug() << "EXPECT" << typeName << "GOT" << ret.typeName();

                // convert to explicit type (e.g. double -> int)
                if (ret.type() != type && !ret.convert(type))
                    QPROPS_JSON_ERROR("Could not convert QVariant type '"
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
            QPROPS_IO_ERROR("Can't convert value type '" << ret.typeName()
                           << "' in json to '"
                           << QVariant::typeToName(expected) << "'");
        }
    }

    return ret;
}

// wrapper for expect()
template <typename T>
T JsonInterfaceHelper::expectChild(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        QPROPS_JSON_ERROR("Expected '" << key << "' value ("
                          << typeid(T).name() << "), not found");
    return expect<T>( parent.value(key) );
}



QVariant JsonInterfaceHelper::expectChildQVariant(
        const QJsonObject& parent, const QString& key,
        QVariant::Type expectedType)
{
    if (!parent.contains(key))
        QPROPS_JSON_ERROR("Expected '" << key << "' qvariant, not found");
    return expectQVariant(parent.value(key), expectedType);
}


// -- wrapper of expect() for arrays --

template <typename T>
void JsonInterfaceHelper::fromArray(std::vector<T>& dst, const QJsonArray& src)
{
    dst.resize(src.size());
    for (size_t i=0; i<dst.size(); ++i)
        dst[i] = expect<T>(src.at(i));
}

template <typename T>
void JsonInterfaceHelper::fromArray(std::vector<T>& dst, const QJsonValue& src)
{
    fromArray(dst, expectArray(src));
}



// --- template instantiation ---



// -- inst. JsonInterfaceHelper::wrap() --

#define QPROPS__INSTANTIATE(T__) \
    template QJsonValue JsonInterfaceHelper::wrap<T__>(const T__&);

    QPROPS__FOR_EACH_COMPOUND_TYPE( QPROPS__INSTANTIATE )

#undef QPROPS__INSTANTIATE


// -- inst. JsonInterfaceHelper::expectChild(), toArray, fromArray() --

#define QPROPS__INSTANTIATE(T__) \
    template T__ JsonInterfaceHelper::expectChild<T__>(const QJsonObject& parent, const QString& key); \
    template QJsonArray JsonInterfaceHelper::toArray<T__>( const std::vector<T__>&); \
    template void JsonInterfaceHelper::fromArray<T__>(std::vector<T__>& dst, const QJsonArray& src); \
    template void JsonInterfaceHelper::fromArray<T__>(std::vector<T__>& dst, const QJsonValue& src);

    QPROPS__INSTANTIATE(float)
    QPROPS__INSTANTIATE(double)
    QPROPS__INSTANTIATE(int8_t)
    QPROPS__INSTANTIATE(uint8_t)
    QPROPS__INSTANTIATE(int16_t)
    QPROPS__INSTANTIATE(uint16_t)
    QPROPS__INSTANTIATE(int32_t)
    QPROPS__INSTANTIATE(uint32_t)
    QPROPS__INSTANTIATE(int64_t)
    QPROPS__INSTANTIATE(uint64_t)
    QPROPS__INSTANTIATE(QString)

    QPROPS__FOR_EACH_COMPOUND_TYPE( QPROPS__INSTANTIATE )

#undef QPROPS__INSTANTIATE



} // namespace QProps
