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
#include <QJsonValue>

#include "PerPage.h"
#include "PageAnnotation.h"
#include "PageLayout.h"
#include "ScoreLayout.h"
#include "QProps/JsonInterfaceHelper.h"

namespace Sonot {


template <class T>
QJsonObject PerPage<T>::toJson() const
{
    QJsonObject o;
    for (auto i = p_data_.begin(); i!=p_data_.end(); ++i)
        o.insert(i.key(), i.value().toJson());
    return o;
}

template <class T>
void PerPage<T>::fromJson(const QJsonObject& o)
{
    QProps::JsonInterfaceHelper json(QString("PerPage<%1>").arg(typeid(T).name()));
    QMap<QString, T> map;
    for (auto key : o.keys())
    {
        T t;
        t.fromJson(json.expectChildObject(o, key));
        map.insert(key, t);
    }
    p_data_.swap(map);
}

template <class T>
const T& PerPage<T>::get(const QString &key) const
{
    auto i = p_data_.find(key);
    return i == p_data_.end() ? p_empty_ : i.value();
}

// -- instantiate for layout types --

template class PerPage<PageAnnotation>;
template class PerPage<PageLayout>;
template class PerPage<ScoreLayout>;


} // namespace Sonot
