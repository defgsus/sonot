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

#include "SynthDevice.h"
#include "core/Bar.h"
#include "core/NoteStream.h"

namespace Sonot {


SynthDevice::SynthDevice(QObject* parent)
    : QIODevice (parent)
    , p_buffer  (1024*4)
    , p_consumed(0)
    , p_synth   ()
    , p_index   (p_score.index(0,0,0,0))
    , p_curSample(0)
    , p_curBarTime(0.)
{

}


qint64 SynthDevice::readData(char *data, qint64 maxlen)
{
    qint64 written = 0;

    while (written < maxlen)
    {
        if (p_consumed >= (qint64)p_buffer.size())
        {
            if (!p_fillBuffer())
                return written;
            p_consumed = 0;
        }

        while (written < maxlen && p_buffer.size() - p_consumed > 0)
        {
            *data = p_buffer[p_consumed];
            ++data;
            ++p_consumed;
            ++written;
        }
    }

    return written;
}

bool SynthDevice::p_fillBuffer()
{
    if (!p_index.isValid())
        return false;

    //double curTime = currentSecond();
    // time per bar
    double barTime = 1.;
    // length of dsp buffer in samples
    double bufferLength = (double)p_buffer.size() / sizeof(float)
                            / sampleRate();
    double procTime = 0.;

    while (procTime < bufferLength)
    {
        // get next bar
        if (p_curBarTime >= barTime)
        {
            // start again
            if (!p_index.nextBar())
            {
                p_index = p_score.index(0,0,0,0);
            }
            if (p_index.isValid())
                p_notesPlaying.resize(p_index.getNoteStream().numRows());

            p_curBarTime = 0.;
        }

        Score::Index cursor = p_index.topLeft();
        if (!cursor.isValid())
            break;

        // send all notes in bar window to synth
        for (size_t r=0; r<cursor.getNoteStream().numRows(); ++r)
        {
            const Bar& bar = cursor.getBar(r);
            for (size_t c=0; c<bar.length(); ++c)
            {
                Note n = bar.note(c);
                if (!n.isValid())
                    continue;

                // window-local time
                double ltime = barTime * bar.columnTime(c) - p_curBarTime;
                if (ltime >= 0 && ltime < bufferLength)
                {
                    // stop prev note
                    if (n.value() != Note::Space)
                    {
                        if (p_notesPlaying[r] >= 0)
                            p_synth.noteOff(p_notesPlaying[r],
                                            ltime * sampleRate());
                        p_notesPlaying[r] = Note::Invalid;
                    }
                    if (n.isNote())
                    {
                        p_synth.noteOn(n.value(), 0.1, ltime * sampleRate());
                        p_notesPlaying[r] = n.value();
                    }
                }
            }
        }

        double timeInc = std::min(barTime - p_curBarTime, bufferLength);
        p_curBarTime += timeInc;
        procTime += timeInc;
    }


    // execute synth block
    p_synth.process(reinterpret_cast<float*>(p_buffer.data()),
                    p_buffer.size() / sizeof(float));
    p_curSample += p_buffer.size() / sizeof(float);

    return true;
}

void SynthDevice::setScore(const Score &score)
{
    p_score = score;
    p_index = p_score.index(0,0,0,0);
    if (p_index.isValid())
        p_notesPlaying.resize(p_index.getNoteStream().numRows());
    p_curSample = 0;
    p_curBarTime = 0;
}

} // namespace Sonot
