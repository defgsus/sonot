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

#include <limits>

#include <QDebug>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QBuffer>
#include <QMap>

#include "SamplePlayer.h"

struct SamplePlayer::Private
{
    Private(SamplePlayer* p)
        : p         (p)
    { }

    struct Sample
    {
        Sample(size_t size)
            : data      (size, Qt::Uninitialized)
            , stream    (nullptr, nullptr)
            , audio     (nullptr)
        { }

        ~Sample()
        {
            qDebug() << "destroy " << (void*)this;
        }

        QByteArray data;
        QBuffer stream;
        QAudioFormat format;
        QAudioOutput* audio;
    };

    void remove(Sample* s)
    {
        if (s->audio)
        {
            s->audio->stop();
            s->audio->deleteLater();
        }
        sampleMap.remove(s->audio);
        delete s;
    }

    SamplePlayer* p;
    QMap<QAudioOutput*, Sample*> sampleMap;
};


SamplePlayer::SamplePlayer(QObject *parent)
    : QObject   (parent)
    , p_        (new Private(this))
{

}

SamplePlayer::~SamplePlayer()
{
    delete p_;
}


void SamplePlayer::play(const float* samplesFloat, size_t numSamples,
                        size_t sampleRate, size_t numChannels)
{
    auto sample = new Private::Sample(numSamples * numChannels * 2);

    sample->format.setSampleRate(sampleRate);
    sample->format.setChannelCount(numChannels);
    sample->format.setSampleSize(32);
    sample->format.setCodec("audio/pcm");
    sample->format.setSampleType(QAudioFormat::Float);
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    sample->format.setByteOrder(QAudioFormat::LittleEndian);
#else
    sample->format.setByteOrder(QAudioFormat::BigEndian);
#endif

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(sample->format))
    {
        qWarning() << "Raw audio format not supported by backend, "
                      "cannot play audio.";
        return;
    }

    // create int16 version
#if 0
    {
        const float* src = samplesFloat;
        int16_t* dst = reinterpret_cast<int16_t*>(sample->data.data());
        for (size_t i=0; i<numSamples * numChannels; ++i)
            *dst++ = *src++ * std::numeric_limits<int16_t>::max();
    }
#else
    sample->data = QByteArray(reinterpret_cast<const char*>(samplesFloat),
                              numSamples * numChannels * sizeof(float));
#endif
    sample->stream.setData(sample->data);
    sample->stream.open(QIODevice::ReadOnly);

    sample->audio = new QAudioOutput(sample->format, this);
    connect(sample->audio, &QAudioOutput::stateChanged,
    [=](QAudio::State state)
    {
        switch (state)
        {
            case QAudio::ActiveState: qDebug() << "active"; break;
            case QAudio::SuspendedState: qDebug() << "suspended"; break;
            case QAudio::IdleState:
                qDebug() << "idle";
                p_->remove(sample);
            break;
            case QAudio::StoppedState:
                qDebug() << "stopped";
            break;
        }
    });

    p_->sampleMap.insert(sample->audio, sample);

    sample->audio->start(&sample->stream);
}
