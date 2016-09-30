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
        , xml    (&xmlString)
    { }

    ExportMusicXML* p;

    void exportAll();

    void exportHeader();
    void exportFooter();
    void exportWork();
    void exportPart(const NoteStream& xml);

    Score score;
    QString xmlString;
    QTextStream xml;
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
    p_->exportAll();
    return p_->xmlString;
}

void ExportMusicXML::Private::exportAll()
{
    exportHeader();
    exportWork();
    for (const NoteStream& s : score.noteStreams())
        exportPart(s);
}

void ExportMusicXML::Private::exportHeader()
{
    xml <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
    "<!DOCTYPE score-partwise PUBLIC\n"
    "    \"-//Recordare//DTD MusicXML 3.0 Partwise//EN\n"
    "    \"http://www.musicxml.org/dtds/partwise.dtd\">\n"
    "<score-partwise>\n";
}

void ExportMusicXML::Private::exportFooter()
{
    xml <<
    "</score-partwise>\n";
}

void ExportMusicXML::Private::exportWork()
{
    xml << "<work>\n"
        << "  <work-number>1</work-number>\n"
        << "  <work-title>" << score.title() << "</work-title>\n"
        << "</work>\n"
        //<movement-number>22</movement-number>
        //<movement-title>Mut</movement-title>
        << "<identification>\n"
        << "  <creator type=\"composer\">"
                << score.author() << "</creator>\n";

    if (score.props().contains("poet"))
        xml
        << "  <creator type=\"poet\">"
        << score.props().get("poet").toString() << "</creator>\n";

    xml << "  <rights>" << score.copyright() << "</rights>\n"
        << "  <encoding>\n"
        //<< "    <encoding-date>2002-02-16</encoding-date>\n"
        //<< "    <encoder>Michael Good</encoder>\n"
        << "    <software>Sonot</software>\n"
        << "    <encoding-description>Sonot export"
                "</encoding-description>\n"
        << "  </encoding>\n"
        //<< "  <source></source>\n"
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

void ExportMusicXML::Private::exportPart(const NoteStream &stream)
{
    xml << "<part>\n"
        << "  <attributes>\n"
        << "    <divisions>24</divisions>\n";

/*
        << "    <key>\n"
        << "      <fifths>-3</fifths>\n"
        << "      <mode>minor</mode>\n"
        << "    </key>\n"
*/
    xml << "    <time>\n"
        << "      <beats>4</beats>\n"
        << "      <beat-type>4</beat-type>\n"
        << "    </time>\n";

    xml << "  </attributes>\n";

    //xml << "  <note><pitch>"

    xml << "</part>\n";
}

} // namespace Sonot
