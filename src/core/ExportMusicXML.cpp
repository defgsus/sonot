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

#include <QTextStream>

#include "ExportMusicXML.h"
#include "Score.h"
#include "NoteStream.h"
#include "Notes.h"
#include "Bar.h"

namespace Sonot {

struct ExportMusicXML::Private
{
    Private(ExportMusicXML* p)
        : p         (p)
        , stream    (&xmlString)
    { }

    ExportMusicXML* p;

    void exportWork();

    Score score;
    QString xmlString;
    QTextStream stream;
};

ExportMusicXML::ExportMusicXML(const Score& s)
    : p_        (new Private(this))
{
    p_->score = s;
}

ExportMusicXML::~ExportMusicXML()
{
    delete p_;
}

QString ExportMusicXML::toString()
{
    p_->xmlString.clear();
    p_->exportWork();
    return p_->xmlString;
}

void ExportMusicXML::Private::exportWork()
{
    stream
    << "<work>\n"
    << "  <work-number>D. 911</work-number>\n"
    << "  <work-title>Winterreise</work-title>\n"
    << "</work>\n"
    //<movement-number>22</movement-number>
    //<movement-title>Mut</movement-title>
    << "<identification>\n"
    << "  <creator type=\"composer\">Franz Schubert</creator>\n"
    << "  <creator type=\"poet\">Wilhelm Müller</creator>\n"
    << "  <rights>Copyright © 2001 Recordare LLC</rights>\n"
    << "  <encoding>\n"
    << "    <encoding-date>2002-02-16</encoding-date>\n"
    << "    <encoder>Michael Good</encoder>\n"
    << "    <software>Finale 2002 for Windows</software>\n"
    << "    <encoding-description>MusicXML 1.0 example</encoding-description>\n"
    << "  </encoding>\n"
    << "  <source>Based on Breitkopf &amp; Härtel edition of 1895</source>\n"
    << "</identification>\n"
    << "<part-list>\n"
    << "  <score-part id=\"P1\">\n"
    << "    <part-name>Singstimme.</part-name>\n"
    << "  </score-part>\n"
    << "  <score-part id=\"P2\">\n"
    << "    <part-name>Pianoforte.</part-name>\n"
    << "  </score-part>\n"
    << "</part-list>\n"
    ;
}

} // namespace Sonot
