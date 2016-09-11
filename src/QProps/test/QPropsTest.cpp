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

#include <QString>
#include <QtTest>
#include <QFont>

#include "Properties.h"

class QPropsTest : public QObject
{
    Q_OBJECT

public:
    QPropsTest() { }

    void addPrimitiveTypes(QProps::Properties& p);
    void addCompoundTypes(QProps::Properties& p);
    void addPrimitiveVectorTypes(QProps::Properties& p);
    void addCompoundVectorTypes(QProps::Properties& p);

private slots:

    void testQVariantCompare();
    void testJsonProperties();
    void testJsonPropertiesExplicitTypes();
};


namespace QTest {

    template <>
    char* toString(const QVariant::Type& t)
    {
        return toString(QVariant::typeToName(t));
    }

    template <>
    char* toString(const QProps::Properties& p)
    {
        return toString(p.toCompactString());
    }

} // namespace QTest


void QPropsTest::testQVariantCompare()
{
#define QPROPS__COMPARE(val__) \
    QProps::Properties::qvariant_compare( \
                QVariant::fromValue(val__), QVariant::fromValue(val__))

    QVERIFY( QPROPS__COMPARE(23) );
    QVERIFY( QPROPS__COMPARE(23.) );
    QVERIFY( QPROPS__COMPARE(23ULL) );
    QVERIFY( QPROPS__COMPARE(QVector<double>() << 1. << 2.) );
    QVERIFY( QPROPS__COMPARE(QVector<qulonglong>() << 1ULL << 2ULL) );
    QVERIFY( QPROPS__COMPARE(QVector<bool>() << false << true) );
    QVERIFY( QPROPS__COMPARE(QVector<QSizeF>() << QSizeF(1,2) << QSizeF(2,3)) );

}

void QPropsTest::addPrimitiveTypes(QProps::Properties& p)
{
    p.set("bool",           true);
    p.set("int",            int(23));
    p.set("uint",           uint(666));
    p.set("longlong",       qlonglong(7777));
    p.set("ulonglong",      qulonglong(7777));
    p.set("double",         42.);
    p.set("long",           long(7777));
    p.set("short",          short(-42));
    p.set("char",           char(65));
    p.set("ulong",          ulong(42));
    p.set("ushort",         ushort(23));
    p.set("uchar",          uchar(42));
    p.set("float",          42.f);
    p.set("schar",          (signed char)(-65));
    p.set("qchar",          QChar(65));
}

void QPropsTest::addCompoundTypes(QProps::Properties& p)
{
    p.set("string",         QString("holladihoh"));
    p.set("color",          QColor(10,20,30,40));
    p.set("rect",           QRect(23, 42, 666, 7777));
    p.set("rectf",          QRectF(23, 42, 666, 7777));
    p.set("size",           QSize(23, 42));
    p.set("sizef",          QSizeF(23, 42));
    p.set("point",          QPoint(42, 23));
    p.set("pointf",         QPointF(42, 23));
    p.set("line",           QLine(10,20,30,40));
    p.set("linef",          QLineF(10,20,30,40));
    p.set("font",           QFont("family", 30., 2, true));
    p.set("date",           QDate::currentDate());
    p.set("time",           QTime::currentTime());
    p.set("datetime",       QDateTime::currentDateTime());
}

void QPropsTest::addPrimitiveVectorTypes(QProps::Properties& p)
{
    p.set("vec-bool",       QVector<bool>() << true);
    p.set("vec-int",        QVector<int>() << int(23));
    p.set("vec-uint",       QVector<uint>() << uint(666));
    p.set("vec-longlong",   QVector<qlonglong>() << qlonglong(7777));
    p.set("vec-ulonglong",  QVector<qulonglong>() << qulonglong(7777));
    p.set("vec-double",     QVector<double>() << 42.);
    p.set("vec-long",       QVector<long>() << long(7777));
    p.set("vec-short",      QVector<short>() << short(-42));
    p.set("vec-char",       QVector<char>() << char(65));
    p.set("vec-ulong",      QVector<ulong>() << ulong(42));
    p.set("vec-ushort",     QVector<ushort>() << ushort(23));
    p.set("vec-uchar",      QVector<uchar>() << uchar(42));
    p.set("vec-float",      QVector<float>() << 42.f);
    p.set("vec-schar",      QVector<signed char>() << (signed char)(-65));
    p.set("vec-qchar",      QVector<QChar>() << QChar(65));
}

void QPropsTest::addCompoundVectorTypes(QProps::Properties& p)
{
    p.set("vec-string",     QVector<QString>() << QString("holladihoh"));
    p.set("vec-color",      QVector<QColor>() << QColor(10,20,30,40));
    p.set("vec-rect",       QVector<QRect>() << QRect(23, 42, 666, 7777));
    p.set("vec-rectf",      QVector<QRectF>() << QRectF(23, 42, 666, 7777));
    p.set("vec-size",       QVector<QSize>() << QSize(23, 42));
    p.set("vec-sizef",      QVector<QSizeF>() << QSizeF(23, 42));
    p.set("vec-point",      QVector<QPoint>() << QPoint(42, 23));
    p.set("vec-pointf",     QVector<QPointF>() << QPointF(42, 23));
    p.set("vec-line",       QVector<QLine>() << QLine(10,20,30,40));
    p.set("vec-linef",      QVector<QLineF>() << QLineF(10,20,30,40));
    p.set("vec-font",       QVector<QFont>() << QFont("family", 30., 2, true));
    p.set("vec-date",       QVector<QDate>() << QDate::currentDate());
    p.set("vec-time",       QVector<QTime>() << QTime::currentTime());
    p.set("vec-datetime",   QVector<QDateTime>() << QDateTime::currentDateTime());
}

void QPropsTest::testJsonProperties()
{
    QProps::Properties p("props-test");

    addPrimitiveTypes(p);
    addCompoundTypes(p);
    addPrimitiveVectorTypes(p);
    addCompoundVectorTypes(p);

    qDebug() << "ORG" << p.toJsonString();
    QProps::Properties p2("copy");
    p2.fromJson(p.toJson());

    //qDebug() << "ORG" << p.toString();
    //qDebug() << "CPY" << p2.toString();
    //qDebug() << "CPY" << p2.toJsonString();

    // Property::operator==
    for (const QProps::Properties::Property& prop : p)
        QCOMPARE(p2.getProperty(prop.id()), prop);

    // Properties::operator==
    QCOMPARE(p2, p);

    // compare types
    for (const QProps::Properties::Property& prop : p)
    {
        if (prop.value().type() != p2.get(prop.id()).type())
            qDebug() << "type mismatch for '" << prop.id() << "'";
        QCOMPARE(p2.get(prop.id()).type(), prop.value().type());
    }
}

void QPropsTest::testJsonPropertiesExplicitTypes()
{
    QProps::Properties p("type-test");
    p.set("int",    23);
    p.set("long",   -7777LL);
    p.set("ulong",  7777ULL);

    QProps::Properties p2("copy");
    p2.fromJson(p.toJson());
    // see if exact type is kept
    QCOMPARE(p2.get("int").type(),  QVariant::Int);
    QCOMPARE(p2.get("long").type(), QVariant::LongLong);
    QCOMPARE(p2.get("ulong").type(), QVariant::ULongLong);

    p.setExplicitJsonTypes(false);
    QProps::Properties p3("copy");
    p3.fromJson(p.toJson());
    // see if exact type is discarded
    QCOMPARE(p3.get("int").type(),  QVariant::Double);
    QCOMPARE(p3.get("long").type(), QVariant::Double);
}





QTEST_APPLESS_MAIN(QPropsTest)

#include "QPropsTest.moc"
