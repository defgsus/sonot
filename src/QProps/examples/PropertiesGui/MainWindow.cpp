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

#include <QProps/PropertiesView.h>

#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow   (parent)
    , p_props       ("gui-types")
{
    setWindowTitle(tr("Properties GUI"));
    setMinimumSize(320,400);

    createProperties();
    createWidgets();
}

MainWindow::~MainWindow()
{

}

void MainWindow::createProperties()
{
    // primitives
    p_props.set("bool",           true);
    p_props.set("char",           char(65));
    p_props.set("schar",          (signed char)(-65));
    p_props.set("uchar",          uchar(42));
    p_props.set("qchar",          QChar(65));
    p_props.set("short",          short(-42));
    p_props.set("ushort",         ushort(23));
    p_props.set("int",            int(23));
    p_props.set("uint",           uint(666));
    p_props.set("float",          42.f);
    p_props.set("double",         42.);
    p_props.set("long",           long(7777));
    p_props.set("ulong",          ulong(42));
    p_props.set("longlong",       qlonglong(7777));
    p_props.set("ulonglong",      qulonglong(7777));
    // compounds
    p_props.set("string",         QString("holladihoh"));
    p_props.set("color",          QColor(10,20,30,40));
    p_props.set("rect",           QRect(23, 42, 666, 7777));
    p_props.set("rectf",          QRectF(23, 42, 666, 7777));
    p_props.set("size",           QSize(23, 42));
    p_props.set("sizef",          QSizeF(23, 42));
    p_props.set("point",          QPoint(42, 23));
    p_props.set("pointf",         QPointF(42, 23));
    p_props.set("line",           QLine(10,20,30,40));
    p_props.set("linef",          QLineF(10,20,30,40));
    p_props.set("font",           QFont("family", 30., 2, true));
    p_props.set("date",           QDate::currentDate());
    p_props.set("time",           QTime::currentTime());
    p_props.set("datetime",       QDateTime::currentDateTime());
    // primitive QVector
    p_props.set("vec-bool",       QVector<bool>()       << false << true );
    p_props.set("vec-char",       QVector<char>()       << -1 << -2 << -3 );
    p_props.set("vec-schar",      QVector<signed char>()<< -1 << -2 << -3 );
    p_props.set("vec-uchar",      QVector<uchar>()      << 1 << 2 << 3 );
    p_props.set("vec-qchar",      QVector<QChar>()      << 1 << 2 << 3 );
    p_props.set("vec-short",      QVector<short>()      << 1 << 2 << 3 );
    p_props.set("vec-ushort",     QVector<ushort>()     << -1 << -2 << -3 );
    p_props.set("vec-int",        QVector<int>()        << 1 << 2 << 3 );
    p_props.set("vec-uint",       QVector<uint>()       << -1 << -2 << -3 );
    p_props.set("vec-float",      QVector<float>()      << 1 << 2 << 3 );
    p_props.set("vec-double",     QVector<double>()     << 1 << 2 << 3 );
    p_props.set("vec-long",       QVector<long>()       << -1 << -2 << -3 );
    p_props.set("vec-ulong",      QVector<ulong>()      << 1 << 2 << 3 );
    p_props.set("vec-longlong",   QVector<qlonglong>()  << -1 << -2 << -3 );
    p_props.set("vec-ulonglong",  QVector<qulonglong>() << 1 << 2 << 3 );
}

void MainWindow::createWidgets()
{
    p_view = new QProps::PropertiesView(this);
    p_view->setProperties(p_props);

    setCentralWidget(p_view);

}
