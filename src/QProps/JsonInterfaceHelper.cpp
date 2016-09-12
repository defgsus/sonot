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

/** Calls QPROPS_IO_ERROR() and adds the
    QProps::JsonInterfaceHelper::className() and current
    QProps::JsonInterfaceHelper::contextStack() */
#define QPROPS_JSON_ERROR(arg__) \
    { const QString ctx__ = contextTrace(); \
      if (ctx__.isEmpty()) \
        QPROPS_IO_ERROR("[JSON: " << className() << "]" << arg__) \
      else \
        QPROPS_IO_ERROR("[JSON: " << className() << "]" << arg__ \
                        << "\nin " << ctx__); }


// ###################### supported types ############################

/** Tuples of C-type, Qt-type and QMetaType::Type */
#define QPROPS__FOR_EACH_PRIMITIVE_TYPE(F__) \
    F__(bool        , bool,           Bool       ) \
    F__(char        , char,           Char       ) \
    F__(int8_t      , signed char,    SChar      ) \
    F__(uint8_t     , uchar,          UChar      ) \
    F__(int16_t     , short,          Short      ) \
    F__(uint16_t    , ushort,         UShort     ) \
    F__(int32_t     , int,            Int        ) \
    F__(uint32_t    , uint,           UInt       ) \
    F__(float       , float,          Float      ) \
    F__(int64_t     , long,           Long       ) \
    F__(uint64_t    , ulong,          ULong      ) \
    F__(int64_t     , qlonglong,      LongLong   ) \
    F__(uint64_t    , qulonglong,     ULongLong  ) \
    F__(double      , double,         Double     )

// All compound types supported by QVariant
// that are excplicitly handled in code
#define QPROPS__FOR_EACH_COMPOUND_TYPE(F__) \
    F__(QChar       , QChar       , QChar       ) \
    F__(QString     , QString     , QString     ) \
    F__(QColor      , QColor      , QColor      ) \
    F__(QRect       , QRect       , QRect       ) \
    F__(QRectF      , QRectF      , QRectF      ) \
    F__(QSize       , QSize       , QSize       ) \
    F__(QSizeF      , QSizeF      , QSizeF      ) \
    F__(QPoint      , QPoint      , QPoint      ) \
    F__(QPointF     , QPointF     , QPointF     ) \
    F__(QLine       , QLine       , QLine       ) \
    F__(QLineF      , QLineF      , QLineF      ) \
    F__(QFont       , QFont       , QFont       ) \
    F__(QTime       , QTime       , QTime       ) \
    F__(QDate       , QDate       , QDate       ) \
    F__(QDateTime   , QDateTime   , QDateTime   )

#define QPROPS__FOR_EACH_TYPE(F__) \
    QPROPS__FOR_EACH_PRIMITIVE_TYPE(F__) \
    QPROPS__FOR_EACH_COMPOUND_TYPE(F__)

/*
#define QPROPS__FOR_EACH_NUMBER_TYPE(F__) \
    F__(double) \
    F__(float) \
    F__(char) \
    F__(int8_t) \
    F__(uint8_t) \
    F__(int16_t) \
    F__(uint16_t) \
    F__(int32_t) \
    F__(uint32_t) \
    F__(int64_t) \
    F__(uint64_t) \
    F__(qlonglong) \
    F__(qulonglong)
*/

namespace QProps {


// ########################### info #############################

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

QMap<int, JsonInterfaceHelper::VariantType> JsonInterfaceHelper::p_imap_;
QMap<QString, JsonInterfaceHelper::VariantType> JsonInterfaceHelper::p_vmap_;
QMap<QString, JsonInterfaceHelper::VariantType> JsonInterfaceHelper::p_cmap_;
JsonInterfaceHelper::VariantType JsonInterfaceHelper::p_invalid_;

void JsonInterfaceHelper::p_createMap_()
{
    p_invalid_.metaType = QMetaType::UnknownType;
    VariantType v;
    // standard QVariant types
#define QPROPS__CREATE(CType__, QType__, Meta__) \
        v.metaType = QMetaType::Meta__; \
        v.isVector = false; \
        v.typeName = #CType__; \
        v.variantName = #QType__; \
        p_imap_.insert((int)v.metaType, v); \
        p_vmap_.insert(v.variantName, v); \
        p_cmap_.insert(v.typeName, v);

    QPROPS__FOR_EACH_TYPE( QPROPS__CREATE )
#undef QPROPS__CREATE

    // QVector of standard QVariant types
#define QPROPS__CREATE(CType__, QType__, Meta__) \
        v.metaType = QMetaType::Meta__; \
        v.isVector = true; \
        v.typeName = "QVector<" #CType__ ">"; \
        v.variantName = "QVector<" #QType__ ">"; \
        p_vmap_.insert(v.variantName, v); \
        p_cmap_.insert(v.typeName, v);

    QPROPS__FOR_EACH_TYPE( QPROPS__CREATE )
#undef QPROPS__CREATE
}

const JsonInterfaceHelper::VariantType&
JsonInterfaceHelper::typeFromQMeta(QMetaType::Type t)
{
    if (p_imap_.isEmpty())
        p_createMap_();

    auto i = p_imap_.find((int)t);
    return i == p_imap_.end() ? p_invalid_ : i.value();
}

const JsonInterfaceHelper::VariantType&
JsonInterfaceHelper::typeFromQVariantName(const QString& name)
{
    if (p_vmap_.isEmpty())
        p_createMap_();

    auto i = p_vmap_.find(name);
    return i == p_vmap_.end() ? p_invalid_ : i.value();
}



QString JsonInterfaceHelper::contextTrace() const
{
    QString s;
    for (int i=p_context_.size(); i > 0; --i)
    {
        if (i != p_context_.size())
            s += "\n";
        s += p_context_[i-1];
    }
    return s;
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






// ########################## to JSON ################################

namespace
{
    static_assert(sizeof(int64_t) == sizeof(qlonglong), "");
    static_assert(sizeof(uint64_t) == sizeof(qulonglong), "");

    /** Converter for arguments acceptable by QJsonValue constructor */
    template <typename T>
    QJsonValue to_json(T v) { return QJsonValue(v); }

    // -- explicit primitives conversion --

    QJsonValue to_json(char v) { return QJsonValue(QString::number((int)v)); }
    QJsonValue to_json(int8_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(uint8_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(int16_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(uint16_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(int32_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(uint32_t v) { return QJsonValue((double)v); }
    QJsonValue to_json(int64_t v) { return QJsonValue(QString::number(v)); }
    QJsonValue to_json(uint64_t v) { return QJsonValue(QString::number(v)); }
    QJsonValue to_json(qlonglong v) { return QJsonValue(QString::number(v)); }
    QJsonValue to_json(qulonglong v) { return QJsonValue(QString::number(v)); }

    // -- conversion of QVariant's compound types --

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

    QJsonValue to_json(const QDate& t)
    {
        return QJsonValue(t.toString());
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
    // try QVariant -> QJsonValue conversion
    if (!explicitType)
    {
        auto jv = QJsonValue::fromVariant(v);
        if (!jv.isNull())
            return jv;
    }

    // -- handle all supported QVariant types --

    const VariantType vtype = typeFromQVariant(v);
    if (!vtype.isValid())
        QPROPS_JSON_ERROR("Can't save QVariant::" << v.typeName()
                     << "(" << (int)v.type() << ") to json, "
                     "type not implemented!");

    QJsonObject o;
    o.insert("t", vtype.variantName);

    switch (vtype.metaType)
    {
        // calls to_json(QType__)
        //    or to_json(QVector<QType__>)
#define QPROPS__ARRAY_WRITE(CType__, QType__, Meta__) \
        case QMetaType::Meta__: \
            if (vtype.isVector) \
                o.insert("v", to_json(v.value<QVector<QType__>>())); \
            else \
                o.insert("v", to_json(v.value<QType__>())); \
        break;
        // run for each supported type
        QPROPS__FOR_EACH_TYPE( QPROPS__ARRAY_WRITE )

#undef QPROPS__ARRAY_WRITE

        default:
        {
            // try QVariant's inbuilt conversion
            QJsonValue jv = QJsonValue::fromVariant(v);
            if (jv.isNull())
                QPROPS_JSON_ERROR("Can't save QVariant::" << v.typeName()
                             << "(" << (int)v.type() << ") to json, "
                             "base type not implemented!");
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


template <>
bool JsonInterfaceHelper::expect(const QJsonValue& v)
{
    if (!v.isBool())
        QPROPS_JSON_ERROR("Expected bool, got " << typeName(v));
    return v.toBool();
}


// expect number, either as json-double or convertible string
#define QPROPS__EXPECT_NUMBER(Type__, fromStr__) \
    template <> \
    Type__ JsonInterfaceHelper::expect(const QJsonValue& v) \
    { \
        if (!v.isDouble() && !v.isString()) \
            QPROPS_JSON_ERROR("Expected " #Type__ " value, got " \
                              << typeName(v)); \
        if (v.isString()) \
        { \
            bool ok; \
            Type__ k = v.toString().fromStr__(&ok); \
            if (!ok) \
                QPROPS_JSON_ERROR("Expected " #Type__ " value, " \
                                  "got non-" #Type__ " string '" \
                                  << v.toString() << "'"); \
            return k; \
        } \
        return v.toDouble(); \
    }

QPROPS__EXPECT_NUMBER(double, toDouble)
QPROPS__EXPECT_NUMBER(float, toFloat)
QPROPS__EXPECT_NUMBER(char, toInt)
//QPROPS__EXPECT_NUMBER(uchar, toUInt)
QPROPS__EXPECT_NUMBER(int8_t, toInt)
QPROPS__EXPECT_NUMBER(uint8_t, toFloat)
QPROPS__EXPECT_NUMBER(int16_t, toShort)
QPROPS__EXPECT_NUMBER(uint16_t, toUShort)
QPROPS__EXPECT_NUMBER(int32_t, toInt)
QPROPS__EXPECT_NUMBER(uint32_t, toUInt)
QPROPS__EXPECT_NUMBER(int64_t, toLongLong)
QPROPS__EXPECT_NUMBER(uint64_t, toULongLong)
//QPROPS__EXPECT_NUMBER(long, toLong)
//QPROPS__EXPECT_NUMBER(ulong, toULong)
QPROPS__EXPECT_NUMBER(qlonglong, toLongLong)
QPROPS__EXPECT_NUMBER(qulonglong, toULongLong)

#undef QPROPS__EXPECT_NUMBER


template <>
QString JsonInterfaceHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        QPROPS_JSON_ERROR("Expected string value, got " << typeName(v));
    return v.toString();
}

template <>
QChar JsonInterfaceHelper::expect(const QJsonValue& v)
{
    QString s = expect<QString>(v);
    if (s.isEmpty())
        QPROPS_JSON_ERROR("Expected QChar, got empty string");
    if (s.size() > 1)
        QPROPS_JSON_ERROR("Expected QChar, got string of size" << s.size());
    return s[0];
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
QDate JsonInterfaceHelper::expect(const QJsonValue& v)
{
    if (!v.isString())
        QPROPS_JSON_ERROR("Expected date string, got " << typeName(v));

    QDate t = QDate::fromString(v.toString());
    if (!t.isValid())
        QPROPS_JSON_ERROR("Can not parse date string '"
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
        const QJsonValue& v, QMetaType::Type expected)
{
    QVariant ret;

    QPROPS_ASSERT( expected != QMetaType::User,
                   "Can not convert to QMetaType::User" );

    // no special json object
    if (!v.isObject())
    {
        // use built-in QJsonValue->QVariant
        ret = v.toVariant();
        if (!ret.isValid())
            QPROPS_JSON_ERROR("Expected QVariant compatible json value, "
                             "got '" << typeName(v) << "'");
    }
    // convert special JsonObject->QVariant
    else
    {
        QJsonObject o = v.toObject();
        VariantType vtype = typeFromQVariantName(
                        expectChild<QString>(o, "t") );
        if (!vtype.isValid())
            QPROPS_JSON_ERROR("Unsupported QVariant type '"
                              << typeName << "'");

        switch (vtype.metaType)
        {
            // calls expect<QType__>()
            // for single and QVector values
#define QPROPS__QVARIANT_CASE(CType__, QType__, Meta__) \
            case QMetaType::Meta__: \
                if (!vtype.isVector) \
                    ret = QVariant::fromValue(expectChild<QType__>(o, "v")); \
                else \
                { \
                    QJsonArray ar = expectChildArray(o, "v"); \
                    QVector<QType__> vec; \
                    for (int i=0; i<ar.size(); ++i) \
                        vec << expect<QType__>(ar.at(i)); \
                    ret = QVariant::fromValue(vec); \
                } \
            break;

            QPROPS__FOR_EACH_TYPE( QPROPS__QVARIANT_CASE )

#undef QPROPS__QVARIANT_CASE

            default:
                // check for Qt's QVariant conversion
                ret = expectChildValue(o, "v").toVariant();
                if (ret.isNull())
                    QPROPS_JSON_ERROR("Unsupported QVariant type '"
                                      << typeName << "'");

                // convert to explicit type (e.g. double -> int)
                if (QMetaType::Type(ret.type()) != vtype.metaType)
                    ret = p_convert_(ret, vtype.metaType);
            break;
        }
    }

    // explicit type conversion
    if (expected != QMetaType::UnknownType
     && expected != QMetaType::Type(ret.type()))
    {
        ret = p_convert_(ret, expected);
    }

    return ret;
}

QVariant JsonInterfaceHelper::p_convert_(
        const QVariant &v, QMetaType::Type newType)
{
    /** @note need to handle String -> char specially
        There are some inconsistencies between
        QVariant::Type and QMetaType::Type */
    if (v.type() == QVariant::String)
    {
        QString s = v.toString();
        switch (newType)
        {
            case QMetaType::Char:
                return QVariant::fromValue( s[0].toLatin1() );
            case QMetaType::SChar:
                return QVariant::fromValue( (signed char)s[0].toLatin1() );
            case QMetaType::UChar:
                return QVariant::fromValue( (unsigned char)s[0].toLatin1() );
            case QMetaType::QChar:
                return QVariant::fromValue( QChar(s[0]) );
            default: break;
        }
    }

    QVariant conv = v;
    if (!conv.convert(newType))
        QPROPS_JSON_ERROR("Could not convert QVariant " << v
                     << " of type '"
                     << v.typeName() << "'(" << (int)v.type()
                     << ") to '" << QMetaType::typeName(newType) << "'("
                     << (int)newType << ")");
    return conv;
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
        QMetaType::Type expectedType)
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

#define QPROPS__INSTANTIATE(CType__, QType__, Meta__) \
    template QJsonValue JsonInterfaceHelper::wrap<QType__>(const QType__&);

    QPROPS__FOR_EACH_TYPE( QPROPS__INSTANTIATE )

#undef QPROPS__INSTANTIATE


// -- inst. JsonInterfaceHelper::expectChild(), toArray, fromArray() --

#define QPROPS__INSTANTIATE_IMPL(CType__, T__, Meta__) \
    template T__ JsonInterfaceHelper::expectChild<T__>(const QJsonObject& parent, const QString& key); \
    template QJsonArray JsonInterfaceHelper::toArray<T__>( const std::vector<T__>&); \
    template void JsonInterfaceHelper::fromArray<T__>(std::vector<T__>& dst, const QJsonArray& src); \
    template void JsonInterfaceHelper::fromArray<T__>(std::vector<T__>& dst, const QJsonValue& src);
#define QPROPS__INSTANTIATE(T__) QPROPS__INSTANTIATE_IMPL(T__, T__, T__)

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

    QPROPS__FOR_EACH_COMPOUND_TYPE( QPROPS__INSTANTIATE_IMPL )

#undef QPROPS__INSTANTIATE



} // namespace QProps
