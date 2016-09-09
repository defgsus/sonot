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

#ifndef SONOTSRC_PERPAGE_H
#define SONOTSRC_PERPAGE_H

#include <QString>
#include <QMap>

#include "QProps/JsonInterface.h"

namespace Sonot {

template <class T>
class PerPage : public QProps::JsonInterface
{
public:

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // ----- getter -----

    bool operator == (const PerPage& o) const { return p_data_ == o.p_data_; }
    bool operator != (const PerPage& o) const { return !(*this == o); }

    bool contains(const QString& key) const { return p_data_.contains(key); }

    const T& get(const QString& key) const;
    const T& operator[](const QString& key) const { return get(key); }

    // ----- setter -----

    void clear() { p_data_.clear(); }

    void insert(const QString& key, const T& data) { p_data_.insert(key, data); }
    void remove(const QString& key) { p_data_.remove(key); }

private:
    QMap<QString, T> p_data_;
    T p_empty_;
};

} // namespace Sonot

#endif // SONOTSRC_PERPAGE_H

