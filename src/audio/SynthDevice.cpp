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

#if (0)
#   include <QDebug>
#   define SONOT_DEBUG_SYNTH(arg__) qDebug() << arg__
#else
#   define SONOT_DEBUG_SYNTH(unused__)
#endif

namespace Sonot {


struct SynthDevice::Private
{
    Private(SynthDevice* p)
        : p             (p)
        , buffer        (1024)
        , consumed      (0)
        , synth         ()
        , playing       (false)
        , score         (nullptr)
        , index         ()
        , curSample     (0)
        , curBarTime    (0.)
    {

    }

    bool fillBuffer();

    struct PlayNote
    {
        int8_t note;
        int64_t idx;
        double duration, started;
    };

    SynthDevice* p;

    std::vector<char> buffer;
    qint64 consumed;
    Synth synth;
    bool playing;
    const Score* score;
    Score::Index index;
    uint64_t curSample;
    double curBarTime;
    std::list<PlayNote> playNotes;
};


SynthDevice::SynthDevice(QObject* parent)
    : QIODevice (parent)
    , p_        (new Private(this))
{

}

SynthDevice::~SynthDevice()
{
    delete p_;
}


qint64 SynthDevice::readData(char *data, qint64 maxlen)
{
    qint64 written = 0;

    while (written < maxlen)
    {
        if (p_->consumed >= (qint64)p_->buffer.size())
        {
            auto oldIdx = p_->index;
            bool ret = p_->fillBuffer();
            if (p_->index != oldIdx)
                emit indexChanged(p_->index);
            if (!ret)
                return written;

            p_->consumed = 0;
        }

        while (written < maxlen && p_->buffer.size() - p_->consumed > 0)
        {
            *data = p_->buffer[p_->consumed];
            ++data;
            ++p_->consumed;
            ++written;
        }
    }

    return written;
}

const Synth& SynthDevice::synth() const { return p_->synth; }
const Score* SynthDevice::score() const { return p_->score; }

size_t SynthDevice::sampleRate() const { return p_->synth.sampleRate(); }
size_t SynthDevice::bufferSize() const
    { return p_->buffer.size() / sizeof(float); }
double SynthDevice::currentSecond() const
    { return double(p_->curSample) / std::max(size_t(1), sampleRate()); }


void SynthDevice::setScore(const Score* score)
{
    p_->score = score;
    p_->index = p_->score->index(p_->index.stream(), p_->index.bar(),
                             p_->index.row(), p_->index.column());
    p_->synth.notesOff();
    p_->curSample = 0;
    p_->curBarTime = 0;
}

void SynthDevice::setIndex(const Score::Index& idx)
{
    p_->index = idx;
    p_->curBarTime = 0;
}

void SynthDevice::setPlaying(bool e)
{
    p_->playing = e;
    p_->synth.notesOff();
}

void SynthDevice::setSynthProperties(const QProps::Properties& p)
{
    p_->synth.setProperties(p);
}


void SynthDevice::playNote(int8_t note, double duration)
{
    Private::PlayNote n;
    n.note = note;
    n.started = -1.;
    n.duration = duration;
    p_->playNotes.push_back( n );
}

bool SynthDevice::Private::fillBuffer()
{
    // length of dsp buffer in samples
    double bufferLength = (double)p->bufferSize() / p->sampleRate();

    if (playing && score && index.isValid() && index.score() == score)
    {
        // time per bar
        double barLength = 1.15;
        // time in dsp block
        double procTime = 0.;

        SONOT_DEBUG_SYNTH("--- dsp-block --- " << bufferSize());

        while (procTime < bufferLength)
        {
            SONOT_DEBUG_SYNTH("while(procTime < bufferLength) "
                              << "procTime" << procTime
                              << ", curBarTime" << curBarTime
                              << ", bufferLength" << bufferLength
                              << ", barLength" << barLength);

            // get next bar
            if (curBarTime >= barLength)
            {
                SONOT_DEBUG_SYNTH("next bar");
                bool doStopNotes = false;
                size_t curStream = index.stream(),
                       numRows = index.numRows();
                if (!index.nextBar())
                {
                    // start again
                    index = score->index(0,0,0,0);
                    doStopNotes = true;
                }
                if (index.stream() != curStream)
                    doStopNotes = true;
                // note-off at stream end
                /// @todo this is not timed within the dsp-block!
                if (doStopNotes)
                    for (size_t r=0; r<numRows; ++r)
                        synth.noteOffByIndex(r, 0);

                curBarTime = 0.;
            }

            Score::Index cursor = index.topLeft();
            if (!cursor.isValid())
                break;

            double windowLength = std::min(bufferLength - procTime,
                                           barLength - curBarTime);

            SONOT_DEBUG_SYNTH("processing bar window "
                              << curBarTime << " to "
                              << (curBarTime + windowLength)
                              << " , len=" << windowLength);

            // send all notes in bar window to synth
            for (size_t r=0; r<cursor.getStream().numRows(); ++r)
            {
                const Bar& bar = cursor.getBar(r);
                for (size_t c=0; c<bar.length(); ++c)
                {
                    Note n = bar.note(c);
                    if (!n.isValid())
                        continue;

                    // window-local time
                    double ltime = barLength * bar.columnTime(c) - curBarTime;
                    if (ltime >= 0 && ltime < windowLength)
                    {
                        size_t samplePos = (procTime + ltime) * p->sampleRate();

                        // stop prev note
                        if (n.value() != Note::Space)
                        {
                            synth.noteOffByIndex(r, samplePos);
                        }
                        if (n.isNote())
                        {
                            synth.noteOn(n.value(), 0.1, samplePos, r);
                        }
                    }
                }
            }

            SONOT_DEBUG_SYNTH("fed " << windowLength);

            curBarTime += windowLength;
            procTime += windowLength;
        }
    }


    // also add all PlayNote requests
    std::list<PlayNote> notes;
    for (PlayNote& n : playNotes)
    {
        if (n.started < 0.)
        {
            n.idx = 10000 + playNotes.size();
            n.started = p->currentSecond();
            synth.noteOn(n.note, 0.1, 0, n.idx);
        }

        // stop when duration is within this window
        double d = p->currentSecond() - n.started
                 - n.duration + bufferLength;
        if (d > 0. && d < bufferLength)
        {
            synth.noteOffByIndex(n.idx, std::min(
                                     p->bufferSize()-1,
                                     size_t(d * p->sampleRate())));
            continue;
        }

        notes.push_back(n);
    }
    playNotes.swap(notes);

    // execute synth block
    synth.process(reinterpret_cast<float*>(buffer.data()),
                    p->bufferSize());
    curSample += p->bufferSize();

    return true;
}


} // namespace Sonot
