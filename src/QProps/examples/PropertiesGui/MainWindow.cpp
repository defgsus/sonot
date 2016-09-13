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

#include <QLayout>
#include <QPlainTextEdit>

#include <QProps/Properties.h>
#include <QProps/PropertiesView.h>

#include "MainWindow.h"

struct MainWindow::Private
{
    Private(MainWindow* p)
        : p         (p)
        , props     ("gui-types")
    { }

    void createProperties();
    void createWidgets();

    MainWindow* p;

    QProps::Properties props;
    QProps::PropertiesView* propsView;
    QPlainTextEdit* textOut;
};


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow   (parent)
    , p_            (new Private(this))
{
    setWindowTitle(tr("Properties GUI"));
    setMinimumSize(480,640);

    p_->createProperties();
    p_->createWidgets();
}

MainWindow::~MainWindow()
{
    delete p_;
}

void MainWindow::Private::createProperties()
{
    // primitives
    props.set("bool",           true);
    props.set("char",           char(65));
    props.set("schar",          (signed char)(-65));
    props.set("uchar",          uchar(42));
    props.set("qchar",          QChar(65));
    props.set("short",          short(-42));
    props.set("ushort",         ushort(23));
    props.set("int",            int(23));
    props.set("uint",           uint(666));
    props.set("float",          42.f);
    props.set("double",         42.);
    props.set("long",           long(7777));
    props.set("ulong",          ulong(42));
    props.set("longlong",       qlonglong(7777));
    props.set("ulonglong",      qulonglong(7777));
    // compounds
    props.set("string",         QString("holladihoh"));
    props.set("color",          QColor(10,20,30,40));
    props.set("rect",           QRect(23, 42, 666, 7777));
    props.set("rectf",          QRectF(23, 42, 666, 7777));
    props.set("size",           QSize(23, 42));
    props.set("sizef",          QSizeF(23, 42));
    props.set("point",          QPoint(42, 23));
    props.set("pointf",         QPointF(42, 23));
    props.set("line",           QLine(10,20,30,40));
    props.set("linef",          QLineF(10,20,30,40));
    props.set("font",           QFont("family", 30., 2, true));
    props.set("date",           QDate::currentDate());
    props.set("time",           QTime::currentTime());
    props.set("datetime",       QDateTime::currentDateTime());
    // primitive QVector
    props.set("vec-bool",       QVector<bool>()       << false << true );
    props.set("vec-char",       QVector<char>()       << -1 << -2 << -3 );
    props.set("vec-schar",      QVector<signed char>()<< -1 << -2 << -3 );
    props.set("vec-uchar",      QVector<uchar>()      << 1 << 2 << 3 );
    props.set("vec-qchar",      QVector<QChar>()      << 1 << 2 << 3 );
    props.set("vec-short",      QVector<short>()      << -1 << -2 << -3 );
    props.set("vec-ushort",     QVector<ushort>()     << 1 << 2 << 3 );
    props.set("vec-int",        QVector<int>()        << -1 << -2 << -3 );
    props.set("vec-uint",       QVector<uint>()       << 1 << 2 << 3 );
    props.set("vec-float",      QVector<float>()      << 1.4 << 2.5 << 3.6 );
    props.set("vec-double",     QVector<double>()     << 1.4 << 2.5 << 3.6 );
    props.set("vec-long",       QVector<long>()       << -1 << -2 << -3 );
    props.set("vec-ulong",      QVector<ulong>()      << 1 << 2 << 3 );
    props.set("vec-longlong",   QVector<qlonglong>()  << -1 << -2 << -3 );
    props.set("vec-ulonglong",  QVector<qulonglong>() << 1 << 2 << 3 );
}

void MainWindow::Private::createWidgets()
{
    p->setCentralWidget( new QWidget(p) );
    auto lh = new QHBoxLayout(p->centralWidget());

        propsView = new QProps::PropertiesView(p);
        propsView->setProperties(props);
        lh->addWidget(propsView);

        textOut = new QPlainTextEdit(p);
        lh->addWidget(textOut);

        connect(propsView, &QProps::PropertiesView::propertyChanged,
                [=](const QString& id)
        {
            // copy the changed value from view to local Properties
            props.set(id, propsView->properties().get(id));

            textOut->appendPlainText(
                        tr("'%1' changed to '%2' (%3)")
                        .arg(id)
                        .arg(props.getProperty(id).toString())
                        .arg(props.get(id).typeName())
                        );
        });
}
