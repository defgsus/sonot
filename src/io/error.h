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

#ifndef SONOTSRC_ERROR_H
#define SONOTSRC_ERROR_H

#include <exception>
#include <string>
#include <sstream>

#include <QString>

#define SONOT_ERROR(arg__) \
    { throw ::Sonot::Exception() << arg__; }

#define SONOT_IO_ERROR(arg__) \
    { throw ::Sonot::Exception() << "IO: " << arg__; }

#define SONOT_PROG_ERROR(arg__) \
    { throw ::Sonot::Exception() << __FILE__ << ":" << __LINE__ << " " << arg__; }

#define SONOT_ASSERT(cond__, arg__) \
    { if (!(cond__)) SONOT_PROG_ERROR(arg__); }

#define SONOT_ASSERT_LT(a__, b__, arg__) \
    { if (!((a__) < (b__))) SONOT_PROG_ERROR( \
        "'" #a__ "' (" << (long long)(a__) \
        << ") expected to be less than '" #b__ "' (" \
        << (long long)(b__) << ") " << arg__); }

#define SONOT_ASSERT_LTE(a__, b__, arg__) \
    { if (!((a__) <= (b__))) SONOT_PROG_ERROR( \
        "'" #a__ "' (" << (long long)(a__) \
        << ") expected to be less than or equal to '" #b__ "' (" \
        << (long long)(b__) << ") " << arg__); }


namespace Sonot {

/** Exception base class.
    Use like, e.g.:
    @code
    throw Exception() << "my info " << error_code;
    @endcode
    The content of the text stream is returned from classic const char* what().
    Encouraged use is:
    @code
    SONOT_ERROR("information is " << whatever_helps);
    SONOT_IO_ERROR("wrong version, only " << ver << " supported");
    @endcode
    Especially io errors with IoException() will be reported to the user.
    @todo Should be internationalized with tr()
    */
class Exception : public std::exception
{
public:

    Exception() throw() { }

    virtual ~Exception() throw() { }

    virtual const char * what() const throw() { return text_.c_str(); }

    template <class T>
    Exception& operator << (const T& value) { addToStream(value); return *this; }

    template <class T>
    void addToStream(const T& value)
    { std::stringstream s; s << value; text_ += s.str(); }

    void addToStream(const QString& value)
    { text_ += value.toStdString(); }

protected:

    std::string text_;
};


} // namespace Sonot

#endif // SONOTSRC_ERROR_H

