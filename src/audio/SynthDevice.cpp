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

namespace Sonot {


SynthDevice::SynthDevice(QObject* parent)
    : QIODevice(parent)
    , p_buffer  (1024*32)
    , p_consumed(0)
    , p_synth   ()
{

}


qint64 SynthDevice::readData(char *data, qint64 maxlen)
{
    qint64 written = 0;

    while (written < maxlen)
    {
        if (p_consumed >= (qint64)p_buffer.size())
        {
            p_fillBuffer();
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

void SynthDevice::p_fillBuffer()
{
    p_synth.noteOn(60 + rand()%19, .1);

    p_synth.process(reinterpret_cast<float*>(p_buffer.data()),
                    p_buffer.size() / sizeof(float));

}

} // namespace Sonot
