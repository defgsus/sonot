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

#include <QDebug>

//#ifdef QT_CORE_LIB
#   include <QChar>
#   include <QString>
#   include <QStringList>
#   include <QByteArray>
#   include <QBitArray>
#   include <QDate>
#   include <QTime>
#   include <QDateTime>
#   include <QUrl>
#   include <QLocale>
#   include <QRect>
#   include <QRectF>
#   include <QSize>
#   include <QSizeF>
#   include <QLine>
#   include <QLineF>
#   include <QPoint>
#   include <QPointF>
#   include <QRegExp>
#   include <QEasingCurve>
#   include <QUuid>
#   include <QVariant>
#   include <QModelIndex>
#   include <QRegularExpression>
#   include <QJsonValue>
#   include <QJsonObject>
#   include <QJsonArray>
#   include <QJsonDocument>
#   include <QPersistentModelIndex>
//#endif

#ifdef QT_GUI_LIB
#   include <QPixmap>
#   include <QMatrix4x2>
#   include <QMatrix4x3>
#   include <QMatrix4x4>
#   include <QVector2D>
#   include <QVector3D>
#   include <QVector4D>
#   include <QFont>
#   include <QBrush>
#   include <QPalette>
#   include <QIcon>
#   include <QBitmap>
#   include <QCursor>
#   include <QKeySequence>
#   include <QPen>
#   include <QTextLength>
#endif

#include "Properties.h"


// F is a tuple: (QMetaType::TypeName, QMetaType::TypeNameID, RealType)
// ### Qt6: reorder the types to match the C++ integral type ranking
/// @note this is copied from qmetatype.h
#define QT_FOR_EACH_STATIC_PRIMITIVE_TYPE_WITHOUT_VOID(F)\
    F(Bool, 1, bool) \
    F(Int, 2, int) \
    F(UInt, 3, uint) \
    F(LongLong, 4, qlonglong) \
    F(ULongLong, 5, qulonglong) \
    F(Double, 6, double) \
    F(Long, 32, long) \
    F(Short, 33, short) \
    F(Char, 34, char) \
    F(ULong, 35, ulong) \
    F(UShort, 36, ushort) \
    F(UChar, 37, uchar) \
    F(Float, 38, float) \
    F(SChar, 40, signed char) \

#ifdef QT_GUI_LIB
    /// @note this is copied from qmetatype.h
    /// removed types that have no operator ==
    #define QT_FOR_EACH_STATIC_COMPAREABLE_GUI_CLASS(F)\
        F(QFont, 64, QFont) \
        F(QBrush, 66, QBrush) \
        F(QColor, 67, QColor) \
        F(QPalette, 68, QPalette) \
        F(QImage, 70, QImage) \
        F(QPolygon, 71, QPolygon) \
        F(QRegion, 72, QRegion) \
        F(QKeySequence, 75, QKeySequence) \
        F(QPen, 76, QPen) \
        F(QTextLength, 77, QTextLength) \
        F(QTextFormat, 78, QTextFormat) \
        F(QMatrix, 79, QMatrix) \
        F(QTransform, 80, QTransform) \
        F(QMatrix4x4, 81, QMatrix4x4) \
        F(QVector2D, 82, QVector2D) \
        F(QVector3D, 83, QVector3D) \
        F(QVector4D, 84, QVector4D) \
        F(QQuaternion, 85, QQuaternion) \
        F(QPolygonF, 86, QPolygonF)
#else
    #define QT_FOR_EACH_STATIC_COMPAREABLE_GUI_CLASS(F)
#endif


#define QT_FOR_EACH_STATIC_COMPAREABLE_QT_TYPE(F) \
        QT_FOR_EACH_STATIC_PRIMITIVE_TYPE_WITHOUT_VOID( F ) \
        QT_FOR_EACH_STATIC_CORE_CLASS( F ) \
        QT_FOR_EACH_STATIC_COMPAREABLE_GUI_CLASS( F )



namespace QProps {

QString Properties::qvariant_to_string(const QVariant& v)
{
    QString s = v.toString();
    if (!s.isEmpty())
        return s;

    // use QDebug for a string representation
    QDebug deb(&s);

#define QPROPS__TO_STRING(Typename__, Enum__, Type__) \
    case QMetaType::Typename__: deb.nospace() << v.value<Type__>(); break;

    switch (QMetaType::Type(v.type()))
    {
        QT_FOR_EACH_STATIC_CORE_CLASS( QPROPS__TO_STRING )
#ifdef QT_GUI_LIB
        QT_FOR_EACH_STATIC_GUI_CLASS( QPROPS__TO_STRING )
#endif
        default: break;
    }
    return s;
#undef QPROPS__TO_STRING
}


bool Properties::qvariant_compare(const QVariant& v1, const QVariant& v2)
{
    bool ret;

    // fuzzy compare numbers?
    if (v1.canConvert(QMetaType::Double))
    {
        double d1 = v1.toDouble(),
               d2 = v2.toDouble();
        ret = (std::abs(d1 - d2) < 0.00000001);
    }
    else

#define QPROPS__VEC_COMAPRE_IMPL(Vector__, Type__) \
    if (0==strcmp(v1.typeName(), #Vector__ "<" #Type__ ">") \
     && 0==strcmp(v2.typeName(), #Vector__ "<" #Type__ ">")) \
    { \
        ret = v1.value<Vector__<Type__>>() == v2.value<Vector__<Type__>>(); \
    } else

    // QVector<T>::operator==
#define QPROPS__VEC_COMPARE(Typename__, Enum__, Type__) \
    QPROPS__VEC_COMAPRE_IMPL(QVector, Type__)
    QT_FOR_EACH_STATIC_COMPAREABLE_QT_TYPE( QPROPS__VEC_COMPARE )
#undef QPROPS__VEC_COMPARE

    // QList<T>::operator==
#define QPROPS__VEC_COMPARE(Typename__, Enum__, Type__) \
    QPROPS__VEC_COMAPRE_IMPL(QList, Type__)
    QT_FOR_EACH_STATIC_COMPAREABLE_QT_TYPE( QPROPS__VEC_COMPARE )
#undef QPROPS__VEC_COMPARE

#undef QPROPS__VEC_COMAPRE_IMPL

    // else QVariant::operator==
    ret = (v1 == v2);

    if (!ret) qDebug() << "COMPARE " << ret << v1 << v2;
    return ret;
}




} // namespace QProps


