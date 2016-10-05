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
    {
        noteTypeNames << "whole"
                  << "half"
                  << "quarter"
                  << "eighth"
                  << "16th"
                  << "32nd"
                  << "64th";
    }

    struct MeasureNote
    {
        QString noteName;
        bool isRest;
        int octave, alter, noteType, duration, length;
    };

    struct Measure
    {
        int beat, beatType, divisions;
        std::vector<MeasureNote> notes;
    };

    ExportMusicXML* p;

    void exportAll();

    void exportHeader();
    void exportFooter();
    void exportIdentification();
    void exportNotes();
    void exportRow(size_t row);
    //void exportChords(size_t row, size_t rowCount);

    int getOctaveChange(size_t row);
    void notesToMeasure(Measure& m, const Notes& n);
    void dumpMeasure(const Measure& m);

    Score score;
    QString xmlString;
    QTextStream xml;
    QStringList noteTypeNames;
    KeySignature curKeySig;
    int curOctaveChange;
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


void ExportMusicXML::Private::notesToMeasure(Measure& m, const Notes& n)
{
    switch (n.length())
    {
        case 1:
        case 2:
        case 4:
        case 8:
        case 16:
        case 32:
        case 64:
            m.beat = 4; m.beatType = 4;
        break;

        case 3:
        case 6:
        case 12:
        case 24:
        case 48:
            m.beat = 3; m.beatType = 4;
        break;

        default:
            QPROPS_ERROR("Bar length " << n.length() << " not yet implemented");
    }

    int noteType = 0, dur = 0;
    m.divisions = m.beat * m.beatType;

    switch (n.length())
    {
        case 1:  noteType = 0; dur = m.divisions*4; break;
        case 2:  noteType = 1; dur = m.divisions*2; break;
        case 4:  noteType = 2; dur = m.divisions; break;
        case 8:  noteType = 3; dur = m.divisions/2; break;
        case 16: noteType = 4; dur = m.divisions/4; break;
        case 32: noteType = 5; dur = m.divisions/8; break;
        case 64: noteType = 6; dur = m.divisions/16; break;

        case 3:  noteType = 2; dur = m.divisions; break;
        case 6:  noteType = 3; dur = m.divisions/2; break;
        case 9:  noteType = 4; dur = m.divisions/3; break; // ???
        case 12: noteType = 4; dur = m.divisions/4; break;
        case 24: noteType = 5; dur = m.divisions/8; break;
        case 48: noteType = 6; dur = m.divisions/16; break;

        default:
            QPROPS_ERROR("Bar length " << n.length() << " not yet implemented");
    }

    m.notes.clear();

    bool restWritten = false,
         noteWritten = false;
    for (size_t i = 0; i < n.length(); ++i)
    {
        MeasureNote note;
        note.length = n.length() - i;

        // find actual length
        for (size_t j = i+1; j < n.length(); ++j)
        {
            if (n[j].isNote() || n[j].isRest())
            {
                note.length = j - i;
                break;
            }
        }

        note.duration = dur * note.length;
        note.noteType = noteType;

        int restLength = 0, restType = -1;
        if (note.length > 1)
        {
            // determine note type for length
            note.noteType = std::max(0, note.noteType
                    - (int)std::log2(note.length));
            // see if length fits into time sig
            //restLength = note.length % m.beatType;
            //restType = note.noteType - (int)std::log2(restLength);
        }

        if (n[i].isNote())
        {
            Note tn = curKeySig.transform(n[i]);
            note.isRest = false;
            note.noteName = tn.noteName();
            note.octave = tn.octave() - curOctaveChange;
            note.alter = tn.accidental();

            restWritten = false;
            noteWritten = true;
            m.notes.push_back(note);
        }
        else if (n[i].isRest() && !restWritten)
        {
            note.isRest = true;

            restWritten = true;
            m.notes.push_back(note);
        }
        else if (n[i].isSpace() && !noteWritten && !restWritten)
        {
            note.isRest = true;

            restWritten = true;
            m.notes.push_back(note);
        }

        if (restLength > 0)
        {
            MeasureNote note;
            note.isRest = true;
            note.length = restLength;
            note.duration = dur * note.length;
            note.noteType = restType;

            //restWritten = true;
            m.notes.push_back(note);
        }
    }
}

void ExportMusicXML::Private::dumpMeasure(const Measure& m)
{
    qDebug().noquote().nospace()
        << "measure: "
        << m.beat << "/" << m.beatType << " div=" << m.divisions;
    for (const MeasureNote& n : m.notes)
    {
        QString name = n.isRest ? QString("rest") : n.noteName;
        qDebug().noquote().nospace()
                << name << " " << noteTypeNames[n.noteType]
                << " dur=" << n.duration << " len=" << n.length;
    }
}


void ExportMusicXML::Private::exportAll()
{
    exportHeader();
    exportIdentification();
    exportNotes();
    exportFooter();
}

void ExportMusicXML::Private::exportHeader()
{
    xml <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
    "<!DOCTYPE score-partwise PUBLIC\n"
    "    \"-//Recordare//DTD MusicXML 3.0 Partwise//EN\" "
    "    \"http://www.musicxml.org/dtds/partwise.dtd\">\n"
    "<score-partwise>\n";
}

void ExportMusicXML::Private::exportFooter()
{
    xml <<
    "</score-partwise>\n";
}

void ExportMusicXML::Private::exportIdentification()
{
    xml << "<work>\n"
        //<< "  <work-number>1</work-number>\n"
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
#if 0
    xml << "<part-list>\n"
        << "  <score-part id=\"P1\">\n"
        << "    <part-name>Organ</part-name>\n"
        << "    <part-abbreviation>Org.</part-abbreviation>\n"
        << "    <score-instrument id=\"P1-I3\">\n"
        << "      <instrument-name>Organ</instrument-name>\n"
        << "    </score-instrument>\n"
        << "    <midi-instrument id=\"P1-I3\">\n"
        << "      <midi-channel>1</midi-channel>\n"
        << "      <midi-program>20</midi-program>\n"
        << "      <volume>78.7402</volume>\n"
        << "      <pan>0</pan>\n"
        << "    </midi-instrument>\n"
        << "  </score-part>\n"
        << "</part-list>\n"
    ;

    exportChords(0, score.noteStream(0).numRows());
#else
    xml << "<part-list>\n";
    for (size_t row=0; row<score.noteStream(0).numRows(); ++row)
    {
        QString partName = QString("P%1").arg(row+1);
        xml << "  <score-part id=\"" << partName << "\">\n"
            << "    <part-name>Organ</part-name>\n"
            << "    <part-abbreviation>Org.</part-abbreviation>\n"
            << "    <score-instrument id=\"" << partName << "-I3\">\n"
            << "      <instrument-name>Organ</instrument-name>\n"
            << "    </score-instrument>\n"
            << "    <midi-instrument id=\"" << partName << "-I3\">\n"
            << "      <midi-channel>1</midi-channel>\n"
            << "      <midi-program>20</midi-program>\n"
            << "      <volume>80.</volume>\n"
            << "      <pan>0</pan>\n"
            << "    </midi-instrument>\n"
            << "  </score-part>\n";
    }
    xml << "</part-list>\n";
    for (size_t row=0; row<score.noteStream(0).numRows(); ++row)
        exportRow(row);
#endif
}

int ExportMusicXML::Private::getOctaveChange(size_t row)
{
    const int defaultOctave = 4;

    int oct = 0, count = 0;
    auto idx = score.index(0,0,row,0);
    while (true)
    {
        if (idx.getNote().isNote())
        {
            oct += idx.getNote().octave() - defaultOctave;
            ++count;
        }
        if (!idx.nextNote())
            break;
    }
    return count ? oct / count : 0;
}

void ExportMusicXML::Private::exportRow(size_t row)
{
    xml << "<part id=\"P" << (row+1) << "\">\n";

    Measure prevMeasure;
    prevMeasure.beat = -1;
    prevMeasure.beatType = -1;

    const NoteStream& stream = score.noteStream(0);
    curOctaveChange = getOctaveChange(row);

    for (size_t barIdx=0; barIdx<stream.numBars(); ++barIdx)
    {
        curKeySig = stream.keySignature();
        const Bar& bar = stream.bar(barIdx);
        const Notes& notes = bar.notes(row);

        /*
        if (barIdx > 0 && !notes.containsNotes())
        {
            xml << "  <measure number=\"" << (barIdx+1) << "\">\n"
                << "    <attributes><divisions>1</duration></divisions>"
                   "</attributes>\n"
                << "    <note><rest/><duration>1</duration></note>\n"
                << "  </measure>\n";
            continue;
        }
        */

        Measure measure;
        notesToMeasure(measure, notes);
        dumpMeasure(measure);

        bool writeClef = barIdx == 0;
        bool writeKey = barIdx == 0;
        bool writeTempo = barIdx == 0;
        bool writeTime = false;

        if (measure.beat != prevMeasure.beat
         || measure.beatType != prevMeasure.beatType)
        {
            prevMeasure = measure;
            writeTime = true;
        }

        xml << "  <measure number=\"" << (barIdx+1) << "\">\n";
        xml << "    <attributes>\n";

        xml << "      <divisions>" << measure.divisions << "</divisions>\n";

        if (writeKey)
        {
            xml
            << "      <key>\n"
            << "        <fifths>0</fifths>\n"
            << "        <mode>major</mode>\n"
            << "      </key>\n";

            if (curOctaveChange != 0)
            xml
            << "      <transpose>\n"
            << "        <chromatic>" << (12*curOctaveChange)
                        << "</chromatic>\n"
            << "        <octave-change>" << curOctaveChange
                        << "</octave-change>\n"
            << "      </transpose>\n";
        }

        if (writeTime)
        xml << "      <time>\n"
            << "        <beats>" << measure.beat << "</beats>\n"
            << "        <beat-type>" << measure.beatType << "</beat-type>\n"
            << "      </time>\n";

        if (writeClef)
        {
            xml
            << "      <staves>1</staves>\n"
            << "      <clef number=\"1\">\n"
            << "        <sign>G</sign>\n"
            << "        <line>2</line>\n";
            if (curOctaveChange != 0)
            xml
            << "        <clef-octave-change>" << curOctaveChange
                << "</clef-octave-change>\n";
            xml
            << "      </clef>\n";
        }

        xml << "    </attributes>\n";

        if (writeTempo)
        xml << "    <sound tempo=\"" << stream.beatsPerMinute(0) << "\"/>\n";

        for (const MeasureNote& note : measure.notes)
        {
            if (!note.isRest)
            {
                xml << "    <note><pitch><step>" << note.noteName << "</step>"
                        << "<octave>" << note.octave << "</octave>";
                if (note.alter != 0)
                    xml << "<alter>" << note.alter << "</alter>";
                xml << "</pitch>\n";
                xml << "      <duration>" << note.duration << "</duration>\n";
                xml << "      <type>" << noteTypeNames[note.noteType] << "</type>\n";
                xml << "    </note>\n";
            }
            else
            {
                xml << "    <note><rest/><duration>"
                        << note.duration << "</duration>";
                xml << "<type>" << noteTypeNames[note.noteType] << "</type>";
                xml << "</note>\n";
            }
        }

        xml << "  </measure>\n";
    }

    //xml << "  <note><pitch>"

    xml << "</part>\n";
}



#if 0
void ExportMusicXML::Private::exportChords(size_t row, size_t rowCount)
{
    xml << "<part id=\"P" << (row+1) << "\">\n";

    int prevBeats = -1, prevBeatType = -1;
    const NoteStream& stream = score.noteStream(0);
    for (size_t barIdx=0; barIdx<stream.numBars(); ++barIdx)
    {
        const Bar& bar = stream.bar(barIdx);

        if (barIdx > 0 && !bar.containsNotes())
        {
            xml << "  <measure number=\"" << (barIdx+1) << "\">\n"
                << "    <attributes><divisions>1</duration></divisions>\n"
                << "    <note><rest/><duration>1</duration></note>\n"
                << "  </measure>\n";
            continue;
        }

        QList<Notes> notes;
        for (size_t i=0; i<rowCount; ++i)
            notes << bar.notes(i + row);

        int numNotes = notes[0].length();
        int beats = numNotes % 3 == 0 ? 3 : 4;
        int beatType = 4;
        int divisions = beats * beatType;

        int noteType = 0, duration = 0;
        switch (notes.length())
        {
            case 1:  noteType = 0; duration = divisions*4; break;
            case 2:  noteType = 1; duration = divisions*2; break;
            case 4:  noteType = 2; duration = divisions;   break;
            case 8:  noteType = 3; duration = divisions/2; break;
            case 16: noteType = 4; duration = divisions/4; break;
            case 32: noteType = 5; duration = divisions/8; break;
            case 64: noteType = 6; duration = divisions/16; break;

            case 3:  noteType = 2; duration = divisions;   break;
            case 6:  noteType = 3; duration = divisions/2; break;
            case 9:  noteType = 4; duration = divisions/3; break;
            case 12: noteType = 4; duration = divisions/4; break;
            case 24: noteType = 5; duration = divisions/8; break;
            case 48: noteType = 6; duration = divisions/16; break;
        }
        duration = std::max(1, duration);

        bool writeClef = barIdx == 0;
        bool writeKey = barIdx == 0;
        bool writeTempo = barIdx == 0;
        bool writeTime = false;

        if (prevBeats < 0 || (beats != prevBeats
                              || beatType != prevBeatType))
        {
            prevBeats = beats;
            prevBeatType = beatType;
            writeTime = true;
        }

        xml << "  <measure number=\"" << (barIdx+1) << "\">\n";
        xml << "    <attributes>\n";

        xml << "      <divisions>" << divisions << "</divisions>\n";

        if (writeKey)
        xml << "      <key>\n"
            << "        <fifths>0</fifths>\n"
            << "        <mode>major</mode>\n"
            << "      </key>\n";

        if (writeTime)
        xml << "      <time>\n"
            << "        <beats>" << beats << "</beats>\n"
            << "        <beat-type>" << beatType << "</beat-type>\n"
            << "      </time>\n";

        if (writeClef)
        xml << "      <staves>1</staves>\n"
            << "      <clef number=\"1\">\n"
            << "        <sign>G</sign>\n"
            << "        <line>2</line>\n"
            << "      </clef>\n";

        xml << "    </attributes>\n";

        if (writeTempo)
        xml << "    <sound tempo=\"" << stream.beatsPerMinute(0) << "\"/>\n";

        for (int noteIdx = 0; noteIdx < numNotes; ++noteIdx)
        {
            bool wasNote = false;
            for (int rowIdx = 0; rowIdx < notes.size(); ++rowIdx)
            {
                const Note& note = notes[rowIdx][noteIdx];
                if (note.isNote())
                {
                    xml << "    <note>";
                    if (wasNote)
                        xml << "<chord/>";
                    xml << "<pitch><step>"
                            << note.noteName() << "</step>"
                            << "<octave>" << note.octave()
                            << "</octave></pitch>\n";
                    xml << "      <duration>" << duration << "</duration>\n";
                    xml << "      <type>" << noteNames[noteType] << "</type>\n";
                    xml << "    </note>\n";
                    wasNote = true;
                }
            }
        }

        xml << "  </measure>\n";
    }

    //xml << "  <note><pitch>"

    xml << "</part>\n";
}
#endif

} // namespace Sonot
