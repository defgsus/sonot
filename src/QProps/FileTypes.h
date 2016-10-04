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

#ifndef QPROPS_SRC_FILETYPES_H
#define QPROPS_SRC_FILETYPES_H

#include <QtCore>
#include <QString>
#include <QStringList>

#ifdef QT_GUI_LIB
class QWidget;
#endif

namespace QProps {


struct FileType
{
public:
    FileType(const QString& id,
             const QString& name,
             const QString& directory,
             const QStringList& extensions,
             const QStringList& filters)
        : id(id), name(name), directory(directory),
          extensions(extensions), filters(filters)
    { }

    QString id, name, directory;
    QStringList extensions, filters;
};


/** Singleton instance for managing application's file types */
class FileTypes
{
    Q_DECLARE_TR_FUNCTIONS(FileTypes);

    FileTypes() { }
    FileTypes(const FileTypes&) = delete;
    void operator=(const FileTypes&) = delete;
    struct Private;
public:

    // -------------- getter -----------------

    /** Returns the name of the file type */
    static QString getName(const QString& id);
    /** Returns the default or current directory for the file type */
    static QString getDirectory(const QString& id);
    /** Returns a list of extensions for the file type */
    static QStringList getExtensions(const QString& id);
    /** Returns a list of filters for the file type */
    static QStringList getFilters(const QString& id);

#ifdef QT_GUI_LIB
    /** Returns a filename for saving the file type, or
        empty string */
    static QString getSaveFilename(const QString& id,
                                   QWidget* parent = nullptr,
                                   bool updateDirectory = true);
    static QString getOpenFilename(const QString& id,
                                   QWidget* parent = nullptr,
                                   bool updateDirectory = true);
#endif

    // -------------- setter -----------------

    static void addFileType(const FileType& t);

    static void setDirectory(const QString& id, const QString& dir);
};

} // namespace QProps

#endif // QPROPS_SRC_FILETYPES_H
