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

#ifndef SONOTSRC_SYNTH_H
#define SONOTSRC_SYNTH_H

#include <cstddef>

#include <QtCore>

#include "QProps/Properties.h"

#include "EnvelopeGenerator.h"
#include "NoteFreq.h"

namespace Sonot {

class Synth;

/** One synthesizer voice */
class SynthVoice
{
    friend class Synth;
public:

    SynthVoice(Synth *);
    ~SynthVoice();

    // ------------ getter -------------

    Synth * synth() const;
    /** Returns the index of this voice in Synth */
    size_t index() const;
    bool active() const;
    bool cued() const;
    size_t startSample() const;

    int note() const;
    double frequency() const;
    double phase() const;
    double velocity() const;
    double attack() const;
    double decay() const;
    double sustain() const;
    double release() const;

    const EnvelopeGenerator<double>& envelope() const;

    /** Returns the next unisono voice, or NULL */
    SynthVoice * nextUnisonVoice() const;

    /** Returns the data previously set with setUserData(),
        or the one that was passed to Synth::noteOn(). */
    void * userData() const;

    // ----------- setter --------------

    /** Freely chooseable user data */
    void setUserData(void * data);

private:

    class Private;
    Private * p_;
};



/** A stand-alone polyphonic synthesizer */
class Synth
{
    Q_DECLARE_TR_FUNCTIONS(Synth)
public:

    /** Voice reuse policy when reached max polyphony */
    enum VoicePolicy
    {
        /** Forget new voice */
        VP_FORGET,
        /** Stop lowest voice */
        VP_LOWEST,
        /** Stop highest voice */
        VP_HIGHEST,
        /** Stop oldest voice */
        VP_OLDEST,
        /** Stop newest voice */
        VP_NEWEST,
        /** Stop the quitest voice */
        VP_QUITEST,
        /** Stop the loudest voice */
        VP_LOUDEST
    };

    static QProps::Properties::NamedValues voicePolicyNamedValues();

    Synth();
    ~Synth();

    // ------------ getter ----------------

    const QProps::Properties& props() const;

    size_t sampleRate() const;

    size_t numberVoices() const { return props().get("number-voices").toUInt(); }
    VoicePolicy voicePolicy() const {
        return (VoicePolicy)props().get("voice-policy").toInt(); }

    double volume() const { return props().get("volume").toDouble(); }
    bool combinedUnison() const
        { return !props().get("real-unisono").toBool(); }
    size_t unisonVoices() const
        { return props().get("number-unisono-voices").toUInt(); }
    double unisonDetune() const
        { return props().get("unisono-detune").toDouble(); }
    int unisonNoteStep() const
        { return props().get("unisono-note-step").toInt(); }

    double attack() const { return props().get("attack").toDouble(); }
    double decay() const { return props().get("decay").toDouble(); }
    double sustain() const { return props().get("sustain").toDouble(); }
    double release() const { return props().get("release").toDouble(); }

    double baseFreq() const { return props().get("base-freq").toDouble(); }
    double notesPerOctave()
        { return props().get("notes-per-octave").toDouble(); }

    // ----------- setter -----------------

    /** Sets the sampling rate in Hertz. */
    void setSampleRate(size_t sr);

    void setProperties(QProps::Properties& p);

    // ---------- callbacks ---------------

    /** Supplies a function that should be called when a voice was started.
        The call is immediate and might come from the audio thread!
        Actually the only function that can cause this callback is process(). */
    void setVoiceStartedCallback(std::function<void(SynthVoice*)> func);

    /** Supplies a function that should be called when a voice has ended.
        The call is immediate and might come from the audio thread!
        Actually the only functions that can cause this callback are process(),
        setNumberVoices() and the destructor. */
    void setVoiceEndedCallback(std::function<void(SynthVoice*)> func);

    // ---------- audio -------------------

    /** Starts the next free voice.
        Returns the triggered voice, or NULL if no free voice was found and
        the VoicePolicy doesn't allow for stopping a voice.
        For true unisono voices
        (when unisonVoices() > 1 and combinedUnison() == true),
        the additional voices are in SynthVoice::nextUnisonVoice().
        If @p startSample > 0, the start of the voice will be delayed for the
        given number of samples. The returned voice will be valid
        but not active yet.
        Note that startSample must be smaller than the bufferLength parameter
        in process() to start the voice.
        @p userData is passed to the SynthVoice. */
    SynthVoice * noteOn(int note, double velocity,
                        size_t startSample = 0, void * userData = 0);

    /** Stops all active voices of the given note.
        Depending on their sustain level, they will immidiately stop
        (sustain==0) or enter into RELEASE evelope state. */
    void noteOff(int note);

    /** Turn all notes off imediately. */
    void panic();

    /** Generates @p bufferLength samples of synthesizer music.
        The output will be mono and @p output is expected to point
        at a buffer of size @p bufferLength. */
    void process(float * output, size_t bufferLength);

    /** Generates @p bufferLength samples of synthesizer music.
        Each voice's channel is placed into the buffer of @p output[x].
        @p output is expected to point at numberVoices() buffers
        of size @p bufferLength. */
    void process(float ** output, size_t bufferLength);

private:

    class Private;
    Private * p_;
};


} // namespace Sonot

#endif // SONOTSRC_SYNTH_H
