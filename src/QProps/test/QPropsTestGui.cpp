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
#include <QCoreApplication>

#include <QtWidgets>

#include "Properties.h"
#include "PropertyWidget.h"
#include "PropertiesView.h"

class QPropsGuiTest : public QObject
{
    Q_OBJECT

public:
    QPropsGuiTest() { }

    QProps::Properties createGuiTypeProperties();

private Q_SLOTS:

    void testWidgetTypes();
};


QProps::Properties QPropsGuiTest::createGuiTypeProperties()
{
    QProps::Properties p("gui-types");

    p.set("bool",           true);
    p.set("char",           char(65));
    p.set("schar",          (signed char)(-65));
    p.set("uchar",          uchar(42));
    p.set("qchar",          QChar(65));
    p.set("short",          short(-42));
    p.set("ushort",         ushort(23));
    p.set("int",            int(23));
    p.set("uint",           uint(666));
    p.set("float",          42.f);
    p.set("long",           long(7777));
    p.set("ulong",          ulong(42));
    p.set("longlong",       qlonglong(7777));
    p.set("ulonglong",      qulonglong(7777));
    p.set("double",         42.);

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

    return p;
}


void QPropsGuiTest::testWidgetTypes()
{
    QProps::Properties props = createGuiTypeProperties();

#define QPROPS__TEST_SIMPLE(id__, Widget__) \
    { auto w = new QProps::PropertyWidget(id__, &props); \
      QVERIFY(dynamic_cast<Widget__*>(w->editWidget()) != 0); \
      delete w; }

    QPROPS__TEST_SIMPLE("bool",     QCheckBox);
    //QPROPS__TEST_SIMPLE("char",
    //QPROPS__TEST_SIMPLE("schar",
    //QPROPS__TEST_SIMPLE("uchar",
    //QPROPS__TEST_SIMPLE("qchar",
    QPROPS__TEST_SIMPLE("short",    QSpinBox);
    QPROPS__TEST_SIMPLE("ushort",   QSpinBox);
    QPROPS__TEST_SIMPLE("int",      QSpinBox);
    QPROPS__TEST_SIMPLE("uint",     QSpinBox);
    QPROPS__TEST_SIMPLE("float",    QDoubleSpinBox);
    QPROPS__TEST_SIMPLE("long",     QSpinBox);
    QPROPS__TEST_SIMPLE("ulong",    QSpinBox);
    QPROPS__TEST_SIMPLE("longlong", QSpinBox);
    QPROPS__TEST_SIMPLE("ulonglong",QSpinBox);
    QPROPS__TEST_SIMPLE("double",   QDoubleSpinBox);

    QPROPS__TEST_SIMPLE("string",   QLineEdit);
    QPROPS__TEST_SIMPLE("font",     QFontComboBox);

    //QPROPS__TEST_SIMPLE("color",    );
    //QPROPS__TEST_SIMPLE("rect",
    //QPROPS__TEST_SIMPLE("rectf",
    //QPROPS__TEST_SIMPLE("size",
    //QPROPS__TEST_SIMPLE("sizef",
    //QPROPS__TEST_SIMPLE("point",
    //QPROPS__TEST_SIMPLE("pointf",
    //QPROPS__TEST_SIMPLE("line",
    //QPROPS__TEST_SIMPLE("linef",
    //QPROPS__TEST_SIMPLE("date",
    //QPROPS__TEST_SIMPLE("time",
    //QPROPS__TEST_SIMPLE("datetime",
}


QTEST_MAIN(QPropsGuiTest)

#include "QPropsTestGui.moc"
