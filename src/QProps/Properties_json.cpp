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

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <QColor>

#include "Properties.h"
#include "JsonInterfaceHelper.h"
#include "error.h"

namespace QProps {



QJsonObject Properties::toJson() const
{
    JsonInterfaceHelper json("Properties:" + p_id_);

    QJsonObject main;
    main.insert("id", p_id_);

    QJsonArray array;
    for (const Property& p : p_map_)
    {
        QJsonObject o;
        o.insert("id", p.id());
        if (!p.hasNamedValues())
        {
            json.beginContext(QString("save value for %1:%2")
                              .arg(p_id_).arg(p.id()));
            // write value from QVariant
            o.insert("v", json.wrap(p.value(), p_explicitJsonTypes_));
            json.endContext();
        }
        else
        {
            // write value-id of NamedValue
            if (!p.namedValues().isFlags())
            {
                json.beginContext(QString("save named value for %1:%2")
                                  .arg(p_id_).arg(p.id()));
                auto nv = p.namedValues().getByValue(p.value());
                o.insert("nv", nv.id);
                json.endContext();
            }
            else
            // write each flag id
            {
                json.beginContext(QString("save named value flags for %1:%2")
                                  .arg(p_id_).arg(p.id()));
                QJsonArray jflags;
                auto flag = p.value().toLongLong();
                for (const NamedValues::Value& v : p.namedValues())
                {
                    if ((v.v.toLongLong() & flag) == v.v.toLongLong())
                        jflags.append(v.id);
                }
                o.insert("flags", jflags);

                json.endContext();
            }
        }

        array.append(o);
    }
    main.insert("values", array);

    return main;
}

void Properties::fromJson(const QJsonObject& main)
{
    JsonInterfaceHelper json("Properties:" + p_id_);
    Properties tmp(*this);

    QJsonArray array = json.expectChildArray(main, "values");

    for (int i=0; i<array.size(); ++i)
    {
        QJsonObject o = json.expectObject(array.at(i));

        QString id = json.expectChild<QString>(o, "id");
        Property p = tmp.getProperty(id);

        // read simple value
        if (!p.hasNamedValues() || o.contains("v"))
        {
            json.beginContext(QString("read value for %1:%2")
                                .arg(p_id_).arg(id));
            QVariant::Type type = p.isValid()
                    ? p.value().type() : QVariant::Invalid;
            QVariant v = json.expectChildQVariant(o, "v", type);
            tmp.set(id, v);

            json.endContext();
        }
        else
        // read named value
        {
            if (o.contains("nv"))
            {
                json.beginContext(
                    QString("read named value for %1:%2")
                            .arg(p_id_).arg(id));

                QString vid = json.expectChild<QString>(o, "nv");
                if (!p.namedValues().has(vid))
                    QPROPS_IO_ERROR("Unknown value-id '" << id << "' in json "
                                   "for property '" << id << "'");
                tmp.set(id, p.namedValues().get(vid).v);

                json.endContext();
            }
            else
            // read named value flags
            {
                json.beginContext(
                    QString("read named value flags for %1:%2")
                            .arg(p_id_).arg(id));
                QJsonArray jarray = json.expectChildArray(o, "flags");
                std::vector<QString> ids;
                json.fromArray(ids, jarray);
                tmp.clearFlags(id);
                tmp.setFlags(id, ids);

                json.endContext();
            }
        }

    }

    *this = tmp;
}



} // namespace QProps


