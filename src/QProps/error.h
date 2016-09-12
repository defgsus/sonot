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

#ifndef QPROPS_SRC_ERROR_H
#define QPROPS_SRC_ERROR_H

#include <exception>

#include <QString>
#include <QDebug>


#ifdef QT_DEBUG
    #define QPROPS_ERROR_IMPL_(arg__) \
        { throw ::QProps::Exception() << arg__ \
            << "\n" << __FILE__ << ":" << (int)__LINE__; }
#else
    #define QPROPS_ERROR_IMPL_(arg__) \
        { throw ::QProps::Exception() << arg__; }
#endif


#define QPROPS_ERROR(arg__) QPROPS_ERROR_IMPL_(arg__)

#define QPROPS_IO_ERROR(arg__) QPROPS_ERROR_IMPL_("IO: " << arg__)

#define QPROPS_PROG_ERROR(cond__, arg__) \
    QPROPS_ERROR_IMPL_("ASSERTION (" #cond__ ") FAILED;\n" << arg__)


#ifdef QT_DEBUG
    #define QPROPS_ASSERT(cond__, arg__) \
        { if (!(cond__)) QPROPS_PROG_ERROR(cond__, arg__); }

    #define QPROPS_ASSERT_LT(a__, b__, arg__) \
        { if (!((a__) < (b__))) QPROPS_PROG_ERROR(cond__, \
            "'" #a__ "' (" << (long long)(a__) \
            << ") expected to be less than '" #b__ "' (" \
            << (long long)(b__) << ") " << arg__); }

    #define QPROPS_ASSERT_LTE(a__, b__, arg__) \
        { if (!((a__) <= (b__))) QPROPS_PROG_ERROR(cond__, \
            "'" #a__ "' (" << (long long)(a__) \
            << ") expected to be less than or equal to '" #b__ "' (" \
            << (long long)(b__) << ") " << arg__); }
#else
    #define QPROPS_ASSERT(cond__, arg__) { }
    #define QPROPS_ASSERT_LT(a__, b__, arg__) { }
    #define QPROPS_ASSERT_LTE(a__, b__, arg__) { }
#endif

namespace QProps {

/** Exception base class.
    Use like, e.g.:
    @code
    throw Exception() << "my info " << error_code;
    @endcode
    The content of the text stream is returned from classic const char* what().
    Encouraged use is:
    @code
    QPROPS_ERROR("information is " << whatever_helps);
    QPROPS_IO_ERROR("wrong version, only " << ver << " supported");
    @endcode
    */
class Exception : public std::exception
{
public:

    Exception() throw() { }

    virtual ~Exception() throw() { }

    virtual const char * what() const throw()
        { return text_.toStdString().c_str(); }

    const QString& text() const throw() { return text_; }

    template <class T>
    Exception& operator << (const T& value) throw()
        { addToText(value); return *this; }

    template <class T>
    void addToText(const T& value) throw()
        { QDebug deb(&text_); deb.noquote() << value; }

    void addToText(const std::string& value) throw()
        { text_ += QString::fromStdString(value); }

protected:

    QString text_;
};


} // namespace QProps


#endif // QPROPS_SRC_ERROR_H

