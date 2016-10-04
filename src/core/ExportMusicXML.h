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

#ifndef SONOTSRC_CORE_EXPORTMUSICXML_H
#define SONOTSRC_CORE_EXPORTMUSICXML_H

#include <QString>

namespace Sonot {

class Score;

class ExportMusicXML
{
    ExportMusicXML(const ExportMusicXML&) = delete;
    void operator=(const ExportMusicXML&) = delete;
public:

    ExportMusicXML(const Score& s);
    ~ExportMusicXML();

    QString toString();

    void saveFile(const QString& fn);

private:
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_CORE_EXPORTMUSICXML_H
