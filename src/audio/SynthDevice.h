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
    ~SynthDevice();

    bool isSequential() const override { return true; }

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char*, qint64 ) override { return 0; }


    const Synth& synth() const;
    const Score* score() const;

    size_t sampleRate() const;
    size_t bufferSize() const;

    double currentSecond() const;

public slots:

    void setScore(const Score* score);
    void setIndex(const Score::Index& index);

    void setPlaying(bool e);

    void setSynthProperties(const QProps::Properties& p);
    void setSynthModProperties(size_t idx, const QProps::Properties& p);

    /** Play a single note as soon as possible */
    void playNote(int8_t note, double duration = 1.);

signals:

    void indexChanged(const Score::Index&);

protected:
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_SYNTHDEVICE_H
