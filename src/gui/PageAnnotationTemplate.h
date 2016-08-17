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

#ifndef SONOTSRC_PAGEANNOTATIONTEMPLATE_H
#define SONOTSRC_PAGEANNOTATIONTEMPLATE_H

#include <QString>
#include <QMap>

#include "PageAnnotation.h"


namespace Sonot {

class PageAnnotationTemplate
{
public:
    PageAnnotationTemplate();

    /** Returns the page annotation for id, or empty annotation */
    PageAnnotation getPage(const QString& id) const;

    /** Returns the annotation for given page index. */
    PageAnnotation getPage(int pageIndex) const;

    /** Stores a page annotation for id */
    void setPage(const QString& id, const PageAnnotation&);

    const QMap<QString, PageAnnotation>& pages() const { return p_pages_; }
          QMap<QString, PageAnnotation>& pages()       { return p_pages_; }

    void clear() { p_pages_.clear(); }

    void init(const QString& templateId);

private:

    QMap<QString, PageAnnotation> p_pages_;
};

} // namespace Sonot

#endif // SONOTSRC_PAGEANNOTATIONTEMPLATE_H
