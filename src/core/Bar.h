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

#ifndef SONOTSRC_BAR_H
#define SONOTSRC_BAR_H

#include <vector>

#include "QProps/JsonInterface.h"

#include "Notes.h"

namespace Sonot {

/** Rows of Notes combined in a Bar */
class Bar : public QProps::JsonInterface
{
public:
    Bar();
    ~Bar();

    Bar(const Bar& o);
    Bar& operator = (const Bar& o);

    typedef std::vector<Notes>::iterator Iter;
    typedef std::vector<Notes>::const_iterator ConstIter;

    // --- io ---

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // -- getter --

    bool operator == (const Bar& o) const;
    bool operator != (const Bar& o) const { return !(*this == o); }

    bool isEmpty() const;
    size_t numRows() const;
    size_t maxNumberNotes() const;

    ConstIter begin() const;
    ConstIter end() const;

    const Notes& notes(size_t i) const { return (*this)[i]; }
    const Notes& operator[](size_t i) const;

    QString toString() const;

    // -- setter --

    Iter begin();
    Iter end();

    void clear() { resize(0); }
    void resize(size_t numRows);
    void setNotes(size_t row, const Notes& n);
    void append(const Notes& n);
    void insert(size_t idx, const Notes& n);
    void remove(size_t idx);

    Notes& notes(size_t i) { return (*this)[i]; }
    Notes& operator[](size_t i);

private:
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_BAR_H
