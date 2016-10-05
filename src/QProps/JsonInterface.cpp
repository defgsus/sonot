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

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include "JsonInterface.h"
#include "error.h"

namespace QProps {

QString JsonInterface::toJsonString(bool compact) const
{
    QJsonDocument doc(toJson());
    return QString::fromUtf8(doc.toJson(
                compact ? QJsonDocument::Compact : QJsonDocument::Indented));
}

QByteArray JsonInterface::toJsonByteArray(bool compact) const
{
    QJsonDocument doc(toJson());
    return doc.toJson( compact ? QJsonDocument::Compact
                               : QJsonDocument::Indented );
}

void JsonInterface::fromJsonString(const QString &jsonString)
{
    fromJsonByteArray(jsonString.toUtf8());
}

void JsonInterface::fromJsonByteArray(const QByteArray &jsonString)
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(jsonString, &error);
    if (error.error != QJsonParseError::NoError)
        QPROPS_IO_ERROR("Error parsing json: "
                        << error.errorString());
    auto main = doc.object();
    fromJson(main);
}

QByteArray JsonInterface::toJsonByteArrayZipped() const
{
    return qCompress( toJsonByteArray() );
}

void JsonInterface::fromJsonByteArrayZipped(const QByteArray& a)
{
    auto b = qUncompress(a);
    if (b.isEmpty())
        QPROPS_IO_ERROR("Json byte-array could not be uncompressed");
    fromJsonByteArray( b );
}

void JsonInterface::saveJsonFile(const QString& filename, bool compact) const
{
    QString js = toJsonString(compact);

    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
        QPROPS_IO_ERROR("Could not open '"
                      << filename.toStdString() << "' for writing,\n"
                      << file.errorString().toStdString());

    file.write(js.toUtf8());
}

void JsonInterface::loadJsonFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        QPROPS_IO_ERROR("Could not open '"
                      << filename.toStdString() << "' for reading,\n"
                      << file.errorString().toStdString());

    auto js = QString::fromUtf8(file.readAll());
    file.close();

    fromJsonString(js);
}



} // namespace QProps

