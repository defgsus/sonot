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

#ifndef SONOTSRC_SYNTHDEVICE_H
#define SONOTSRC_SYNTHDEVICE_H

#include <vector>

#include <QIODevice>

#include "Synth.h"
#include "core/Score.h"

namespace Sonot {

class SynthDevice : public QIODevice
{
    Q_OBJECT
public:
    SynthDevice(QObject* parent = nullptr);

    bool isSequential() const override { return true; }

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char*, qint64 ) override { return 0; }

    const Synth& synth() const { return p_synth; }

    size_t sampleRate() const { return p_synth.sampleRate(); }

    double currentSecond() const { return p_curSample * sampleRate(); }

public slots:

    void setScore(const Score& score);

    void setSynthProperties(const QProps::Properties& p)
        { p_synth.setProperties(p); }

protected:

    bool p_fillBuffer();

    std::vector<char> p_buffer;
    qint64 p_consumed;
    Synth p_synth;
    Score p_score;
    Score::Index p_index;
    uint64_t p_curSample;
    double p_curBarTime;
    std::vector<int> p_notesPlaying;
};

} // namespace Sonot

#endif // SONOTSRC_SYNTHDEVICE_H
