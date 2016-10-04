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

#include <QMap>
#ifdef QT_GUI_LIB
#   include <QFileDialog>
#endif

#include "FileTypes.h"
#include "error.h"

namespace QProps {


struct FileTypes::Private
{
    static Private* instance()
    {
        static Private* p_instance_ = nullptr;
        if (!p_instance_)
            p_instance_ = new Private();
        return p_instance_;
    }

    QMap<QString, FileType> typeMap;
};



void FileTypes::addFileType(const FileType& t_)
{
    QPROPS_ASSERT(!t_.id.isEmpty(), "invalid id for FileTypes::addFileType");
    QPROPS_ASSERT(!t_.extensions.isEmpty(),
                  "no extensions given for FileTypes::addFileType("
                  << t_.id << ")");

    FileType t(t_);

    if (t.directory.isEmpty())
        t.directory = "./";
    else t.directory.replace("\\", "/");
    if (!t.directory.endsWith("/"))
        t.directory.append("/");

    for (QString& e : t.extensions)
    {
        QPROPS_ASSERT(!e.isEmpty(),
                      "empty extension in FileTypes::addFileType("
                      << t.id << ")");
        if (!e.startsWith("."))
            e.prepend(".");
    }

    Private::instance()->typeMap.insert(t.id, t);
}

QString FileTypes::getName(const QString& id)
{
    Private* p = Private::instance();
    auto i = p->typeMap.find(id);
    if (i == p->typeMap.end())
        return QObject::tr("file");

    return i.value().name;
}

QString FileTypes::getDirectory(const QString& id)
{
    Private* p = Private::instance();
    auto i = p->typeMap.find(id);
    if (i == p->typeMap.end())
        return "./";

    return i.value().directory;
}

QStringList FileTypes::getExtensions(const QString &id)
{
    Private* p = Private::instance();
    auto i = p->typeMap.find(id);
    if (i == p->typeMap.end())
        return QStringList() << ".*";

    return i.value().extensions;
}


#ifdef QT_GUI_LIB

QString FileTypes::getSaveFilename(const QString& id, QWidget* parent)
{
    QString dir = getDirectory(id);
    QStringList extensions = getExtensions(id);

    QFileDialog diag(parent);

    diag.setDirectory(dir);
    diag.setConfirmOverwrite(true);
    diag.setDefaultSuffix(extensions[0]);
    diag.setAcceptMode(QFileDialog::AcceptSave);
    diag.setWindowTitle(QFileDialog::tr("Save %1").arg(getName(id)));
    diag.setDirectory(dir);
    //diag.setNameFilters(fileTypeDialogFilters[ft]);
    //if (!isDir && !fn.isEmpty())
    //    diag.selectFile(fn);

    if (diag.exec() == QDialog::Rejected
        || diag.selectedFiles().isEmpty())
        return QString();

    QString fn = diag.selectedFiles()[0];

    if (!fn.isEmpty())
    {
        //if (updateFile)
        //    setFilename(ft, fn);

        //if (updateDirectory)
        //    setDirectory(ft, QFileInfo(fn).absolutePath());
    }

    return fn;
}

QString FileTypes::getOpenFilename(const QString& id, QWidget* parent)
{
    QString dir = getDirectory(id);
    QStringList extensions = getExtensions(id);

    QFileDialog diag(parent);

    diag.setDirectory(dir);
    diag.setConfirmOverwrite(false);
    diag.setFilter(QDir::AllEntries);
    diag.setFileMode(QFileDialog::ExistingFiles);
    diag.setAcceptMode(QFileDialog::AcceptOpen);
    diag.setWindowTitle(QFileDialog::tr("Open %1").arg(getName(id)));
    diag.setDirectory(dir);
    //diag.setNameFilters(fileTypeDialogFilters[ft]);
    //if (!isDir && !fn.isEmpty())
    //    diag.selectFile(fn);

    if (diag.exec() == QDialog::Rejected
        || diag.selectedFiles().isEmpty())
        return QString();

    QString fn = diag.selectedFiles()[0];

    if (!fn.isEmpty())
    {
        //if (updateFile)
        //    setFilename(ft, fn);

        //if (updateDirectory)
        //    setDirectory(ft, QFileInfo(fn).absolutePath());
    }

    return fn;
}

#endif



} // namespace QProps
