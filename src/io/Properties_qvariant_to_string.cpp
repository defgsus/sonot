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

#ifdef QT_CORE_LIB
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
#endif

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

namespace Sonot {

QString Properties::qvariant_to_string(const QVariant& v)
{
    QString s = v.toString();
    if (!s.isEmpty())
        return s;

    // use QDebug for a string representation
    QDebug deb(&s);

#define SONOT__TO_STRING(Typename__, Enum__, Type__) \
    case QMetaType::Typename__: deb.nospace() << v.value<Type__>(); break;

    switch (QMetaType::Type(v.type()))
    {
#ifdef QT_CORE_LIB
        QT_FOR_EACH_STATIC_CORE_CLASS(SONOT__TO_STRING)
#endif
#ifdef QT_GUI_LIB
        QT_FOR_EACH_STATIC_GUI_CLASS(SONOT__TO_STRING)
#endif
        default: break;
    }
    return s;
#undef SONOT__TO_STRING
}


} // namespace Sonot
