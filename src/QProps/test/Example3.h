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

#ifndef QPROPSTEST_SRC_EXAMPLE3_H
#define QPROPSTEST_SRC_EXAMPLE3_H

#include <QtCore>
#include <QPoint>
#include <QColor>

#include "Properties.h"
#include "JsonInterface.h"
#include "JsonInterfaceHelper.h"

class Example3 : public QProps::JsonInterface
{
    Q_DECLARE_TR_FUNCTIONS(Example3)
public:

    Example3(const QString& name = QString())
        : p_props("example3")
    {
        // create the properties
        p_initProps();
    }

    // --- getter ---

    const QProps::Properties& properties() const { return p_props; }

    // --- setter ---

    void setProperties(const QProps::Properties& p) { p_props = p; }

    // --- io ---

    // interface to serialize to json
    QJsonObject toJson() const override
    {
        QJsonObject o;
        // create the properties as child object
        // so we are on the save side if we want to add
        // other stuff as well
        o.insert("props", p_props.toJson());
        return o;
    }

    // interface to deserialize from json
    void fromJson(const QJsonObject& o) override
    {
        // construct the helper class
        QProps::JsonInterfaceHelper json("Example3");
        // tell helper what we want to do
        // (this is optional and simply creates better error descriptions)
        json.beginContext("Deserializing properties");
            // deserialize properties
            p_props.fromJson(
                    // ask helper to get a child object
                    // this will throw if "props" is not
                    // found or is not a QJsonObject
                    json.expectChildObject(o, "props")
            );
        json.endContext();
    }


private:
    void p_initProps()
    {
        // primitives
        p_props.set("bool",           true);
        p_props.set("int",            int(23));
        p_props.set("uint",           uint(666));
        p_props.set("longlong",       qlonglong(7777));
        p_props.set("ulonglong",      qulonglong(7777));
        p_props.set("double",         42.);
        p_props.set("long",           long(7777));
        p_props.set("short",          short(-42));
        p_props.set("char",           char(65));
        p_props.set("ulong",          ulong(42));
        p_props.set("ushort",         ushort(23));
        p_props.set("uchar",          uchar(42));
        p_props.set("float",          42.f);
        p_props.set("schar",          (signed char)(-65));
        p_props.set("qchar",          QChar(65));
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
    }

    QProps::Properties p_props;
};



#endif // QPROPSTEST_SRC_EXAMPLE2_H


