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
#include <QFile>

#include "QProps/error.h"

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
    void exportNotes();
    void exportPart(size_t row);

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

void ExportMusicXML::saveFile(const QString& fn)
{
    QFile f(fn);
    if (!f.open(QFile::WriteOnly))
        QPROPS_ERROR("Could not open file " << fn
                     << " for writing.\n" << f.errorString());
    QByteArray a = toString().toUtf8();
    f.write(a);
}

void ExportMusicXML::Private::exportAll()
{
    exportHeader();
    exportWork();
    exportNotes();
    exportFooter();
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
        << "  <work-title>" << score.stringTitle() << "</work-title>\n"
        << "</work>\n"
        //<movement-number>22</movement-number>
        //<movement-title>Mut</movement-title>
        << "<identification>\n"
        << "  <creator type=\"composer\">"
                << score.stringAuthor() << "</creator>\n";

    if (score.props().contains("poet"))
        xml << "  <creator type=\"poet\">"
            << score.props().get("poet").toString() << "</creator>\n";

    xml << "  <rights>" << score.stringCopyright() << "</rights>\n"
        << "  <encoding>\n";
        //<< "    <encoding-date>2002-02-16</encoding-date>\n"

    if (!score.stringTranscriber().isEmpty())
        xml << "    <encoder>" << score.stringTranscriber()
            << "</encoder>\n";

    xml << "    <software>Sonot</software>\n"
        << "    <encoding-description>Sonot export"
                "</encoding-description>\n"
        << "  </encoding>\n";
    if (!score.stringSource().isEmpty())
        xml << "  <source>" << score.stringSource() << "</source>\n";

    xml << "</identification>\n";
}

void ExportMusicXML::Private::exportNotes()
{
    xml << "<part-list>\n"
        << "  <score-part id=\"P1\">\n"
        << "    <part-name>Mal sehen</part-name>\n"
        << "  </score-part>\n"
        << "</part-list>\n"
    ;

    exportPart(0);
}

void ExportMusicXML::Private::exportPart(size_t row)
{
    xml << "<part id=\"P" << (row+1) << "\">\n";

    const NoteStream& stream = score.noteStream(0);
    for (size_t barIdx=0; barIdx<stream.numBars(); ++barIdx)
    {
        xml << "  <messure number = \"" << barIdx << "\">\n";
        xml << "    <attributes>\n";

        const Bar& bar = stream.bar(barIdx);
        int beats = bar.maxNumberNotes();
        int beatType = 4;

        xml << "      <divisions>" << (beats*beatType) << "</divisions>\n";

/*
        << "    <key>\n"
        << "      <fifths>-3</fifths>\n"
        << "      <mode>minor</mode>\n"
        << "    </key>\n"
*/
        xml << "      <time>\n"
            << "        <beats>" << beats << "</beats>\n"
            << "        <beat-type>" << beatType << "</beat-type>\n"
            << "      </time>\n";

        xml << "    </attributes>\n";

        xml << "    <sound tempo=\"" << stream.beatsPerMinute(0) << "\"/>\n";

        const Notes& notes = bar.notes(row);
        for (size_t noteIdx = 0; noteIdx < notes.length(); ++noteIdx)
        {
            const Note& note = notes[noteIdx];
            xml << "    <note><pitch><step>" << note.noteName() << "</step>"
                    << "<octave>" << note.octave() << "</octave></pitch>\n";
            xml << "      <duration>" << 1 << "</duration>\n";
            xml << "      <type>" << "eight" << "</type>\n";
            xml << "    </note>\n";
        }

        xml << "  </messure>";
    }

    //xml << "  <note><pitch>"

    xml << "</part>\n";
}

} // namespace Sonot
