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
#include <QFileInfo>

#ifdef QT_WIDGETS_LIB
#   include <QFileDialog>
#   include <QMessageBox>
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

    static void fixDirectory(QString& dir);

    QMap<QString, FileType> typeMap;
};


void FileTypes::Private::fixDirectory(QString& dir)
{
    if (dir.isEmpty())
        dir = "./";
    else
        dir.replace("\\", "/");

    if (!dir.endsWith("/"))
        dir.append("/");
}


void FileTypes::addFileType(const FileType& t_)
{
    QPROPS_ASSERT(!t_.id.isEmpty(), "invalid id for FileTypes::addFileType");
    QPROPS_ASSERT(!t_.extensions.isEmpty(),
                  "no extensions given for FileTypes::addFileType("
                  << t_.id << ")");

    FileType t(t_);

    Private::fixDirectory(t.directory);

    for (QString& e : t.extensions)
    {
        QPROPS_ASSERT(!e.isEmpty(),
                      "empty extension in FileTypes::addFileType("
                      << t.id << ")");
        if (!e.startsWith("."))
            e.prepend(".");
    }

    if (t.filters.isEmpty())
        t.filters << (tr("Any file") + " (*)");

    Private::instance()->typeMap.insert(t.id, t);
}

void FileTypes::setDirectory(const QString &id, const QString &dir)
{
    QMap<QString,FileType>::iterator
            i = Private::instance()->typeMap.find(id);
    if (i == Private::instance()->typeMap.end())
        return;
    i.value().directory = dir;
    Private::fixDirectory(i.value().directory);
}



QString FileTypes::getName(const QString& id)
{
    Private* p = Private::instance();
    auto i = p->typeMap.find(id);
    if (i == p->typeMap.end())
        return tr("file");

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

QStringList FileTypes::getFilters(const QString &id)
{
    Private* p = Private::instance();
    auto i = p->typeMap.find(id);
    if (i == p->typeMap.end())
        return QStringList() << (tr("Any file") + " (*)");

    return i.value().filters;
}



#ifdef QT_WIDGETS_LIB

QString FileTypes::getSaveFilename(const QString& id, QWidget* parent, bool updateDirectory)
{
    QString dir = getDirectory(id),
            name = getName(id);
    QStringList extensions = getExtensions(id);

    QFileDialog diag(parent);

    diag.setDirectory(dir);
    diag.setConfirmOverwrite(false);
    diag.setDefaultSuffix(extensions[0]);
    diag.setAcceptMode(QFileDialog::AcceptSave);
    diag.setWindowTitle(QFileDialog::tr("Save %1").arg(name));
    diag.setDirectory(dir);
    diag.setNameFilters(getFilters(id));
    //if (!isDir && !fn.isEmpty())
    //    diag.selectFile(fn);

    if (diag.exec() == QDialog::Rejected
        || diag.selectedFiles().isEmpty())
        return QString();

    QString fn = diag.selectedFiles()[0];

    if (!fn.isEmpty())
    {
        if (QFileInfo(fn).exists())
        {
            int r = QMessageBox::question(parent,
                    tr("Replace %1").arg(name),
                    tr("The file %1 already exists\n").arg(fn),
                    tr("Change name"), tr("Overwrite"), tr("Cancel"));
            if (r == 0)
                return getSaveFilename(id, parent);
            if (r == 2)
                return QString();
        }

        //if (updateFile)
        //    setFilename(ft, fn);

        if (updateDirectory)
            setDirectory(id, QFileInfo(fn).absolutePath());
    }

    return fn;
}

QString FileTypes::getOpenFilename(const QString& id, QWidget* parent, bool updateDirectory)
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
    diag.setNameFilters(getFilters(id));
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

        if (updateDirectory)
            setDirectory(id, QFileInfo(fn).absolutePath());
    }

    return fn;
}

#endif



} // namespace QProps
