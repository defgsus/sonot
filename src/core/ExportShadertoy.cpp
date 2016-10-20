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

#include <QList>

#include "ExportShadertoy.h"
#include "Score.h"
#include "NoteStream.h"
#include "NoteFreq.h"

namespace Sonot {

struct ExportShadertoy::Private
{
    Private(ExportShadertoy* p, const Score& s)
        : p         (p)
        , score     (s)
    { }

    struct ExportNote
    {
        Note n;
        double
            start,
            len;
    };

    void transformStream(const NoteStream& s, double start);

    ExportShadertoy* p;
    Score score;

    // exported notes per stream
    QList<QList<ExportNote>> exportNotes;
};

ExportShadertoy::ExportShadertoy(const Score& s)
    : p_        (new Private(this, s))
{

}

ExportShadertoy::~ExportShadertoy()
{
    delete p_;
}

QString ExportShadertoy::toString()
{
    QString code1 =
    "/* created with https://github.com/defgsus/sonot */\n"
    "\n"
    "//%defines%\n"
    "\n"
    "vec2 voice(in float t, float amp)\n"
    "{\n"
    "    t *= 6.283185307;\n"
    "    return vec2(amp*sin(t+amp*sin(t*2.+sin(t*3.))));\n"
    "}\n"
    "\n"
    "float envelope(in float beat, in float len, in float relLen)\n"
    "{\n"
    "    float amp;\n"
    "    if (beat < len)\n"
    "        amp = smoothstep(0., 0.01, beat);\n"
    "    else\n"
    "        amp = smoothstep(relLen, 0., beat-len);\n"
    "    return amp;\n"
    "}\n"
    "\n"
    "float n2f(in float note) { return 440. * pow(pow(2.,1./12.), note-57.); }\n"
    "\n"
    "vec2 music(in float time, in float beat)\n"
    "{\n"
    "    #define N(note, onBeat, susLen) if (beat >= float(onBeat) && beat <= float(onBeat)+float(susLen)+release) s += voice(time * n2f(float(note)), envelope(float(beat) - float(onBeat), float(susLen), release));\n"
    "\n"
    "    float release = .15; // fraction of bar-length\n"
    "    vec2 s = vec2(0.);\n\n",
    code2 =
    "\n"
    "    return s;\n"
    "}\n"
    "\n"
    "// http://www.musicdsp.org/showone.php?id=238\n"
    "float Tanh(in float x) { return clamp(x*(27.+x*x) / (27.+9.*x*x),-1.,1.); }\n"
    "vec2 Tanh(in vec2 x) { return vec2(Tanh(x.x), Tanh(x.y)); }\n"
    "\n"
    "vec2 mainSound(in float time)\n"
    "{\n"
    "    float beat = time / 2.; // 120bpm\n"
    "    return Tanh(.2*music(time, beat));\n"
    "}\n";

    p_->exportNotes.clear();

    int start = 0;
    for (const NoteStream& s : p_->score.noteStreams())
    {
        // get all notes from this stream
        p_->transformStream(s, start);

        int len = s.numBars();
        if (s.isPauseOnEnd())
            ++len;
        start += len;
    }

    // sort exported notes
    for (QList<Private::ExportNote>& notes : p_->exportNotes)
        std::sort(notes.begin(), notes.end(),
                  [](const Private::ExportNote& l, const Private::ExportNote& r)
    {
        return l.start == r.start
                    ? l.len < r.len
                    : l.start < r.start;
    });

    // generate code
    QString code;
    int part = 1;
    start = 0;
    for (const QList<Private::ExportNote>& notes : p_->exportNotes)
    {
        if (part > 1)
            code += QString("#if INCLUDE_PART >= %1\n").arg(part);

        code += QString("    // part %1\n").arg(part);

        int end = start + p_->score.noteStream(part-1).numBars();
        if (p_->score.noteStream(part-1).isPauseOnEnd())
            ++end;
        code += QString("    if (beat >= %1. && beat <= %2.)\n    {\n")
                .arg(start).arg(end);
        start = end;

        for (const Private::ExportNote& n : notes)
            code += QString("        N(%1, %2, %3);\n")
                    .arg(n.n.value())
                    .arg(n.start)
                    .arg(n.len);
        code += "    }\n";
        if (part > 1)
            code += QString("#endif\n");
        ++part;
    }

    // generate defines
    QString defines = "";
    if (p_->score.numNoteStreams() > 1)
    {
        defines = QString("#define INCLUDE_PART %1\n")
                .arg(p_->score.numNoteStreams());
    }

    return code1.replace("//%defines%", defines)
           + code
           + code2;
}

void ExportShadertoy::Private::transformStream(
        const NoteStream& s, double streamStart)
{
    auto keysig = s.keySignature();

    QList<ExportNote> enotes;

    for (size_t rowIdx=0; rowIdx < s.numRows(); ++rowIdx)
    {
        double start = streamStart;

        ExportNote en;

        for (size_t barIdx=0; barIdx < s.numBars(); ++barIdx)
        {
            const Bar& bar = s.bar(barIdx);
            const size_t len = bar[rowIdx].length();
            if (len == 0)
                continue;
            const double unit = 1. / double(len);

            for (size_t column=0; column < len; ++column)
            {
                const Note& note = bar[rowIdx][column];
                if (note.isSpace())
                {
                    en.len += unit;
                }
                else if (note.isRest() || note.isNote())
                {
                    // write note, now we know it's length
                    if (en.n.isNote())
                    {
                        enotes << en;
                        en.n = Note();
                    }

                    // start a new note
                    if (note.isNote())
                    {
                        en.n = keysig.transform(note);
                        en.start = start;
                        en.len = unit;
                    }
                }

                start += unit;
            }
        }

        // write pending note
        if (en.n.isNote())
            enotes << en;
    }

    exportNotes << enotes;
}


} // namespace Sonot
