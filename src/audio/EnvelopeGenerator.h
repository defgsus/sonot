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

#ifndef SONOTSRC_ENVELOPEGENERATOR_H
#define SONOTSRC_ENVELOPEGENERATOR_H

namespace Sonot {

enum EnvelopeState
{
    ENV_ATTACK,
    ENV_DECAY,
    ENV_SUSTAIN,
    ENV_RELEASE
};

template <typename F>
class EnvelopeGenerator
{
public:
    EnvelopeGenerator();

    // ----------- getter ------------

    int sampleRate() const { return sr_; }

    /** Returns true when envelope was triggered and has not
        elapsed the release state. */
    bool active() const { return active_; }

    /** Returns the current state */
    EnvelopeState state() const { return state_; }

    /** Returns current envelope value */
    F value() const { return value_; }

    F attack() const { return attack_; }
    F decay() const { return decay_; }
    F sustain() const { return sustain_; }
    F release() const { return release_; }

    /** Returns true if this envelope is at a later state than @p other */
    bool laterThan(const EnvelopeGenerator& other) { return state_ > other.state_; }

    /** Returns true if this envelope is at an earlier state than @p other */
    bool earlierThan(const EnvelopeGenerator& other) { return state_ < other.state_; }

    // ----------- setter ------------

    void setSampleRate(int sr) { sr_ = sr; calcCoeffs_(); }

    void setAttack(F v) { attack_ = v; attack_c_ = F(8) / std::max(F(8), attack_ * sr_); }
    void setDecay(F v) { decay_ = v; decay_c_ = F(8) / std::max(F(8), decay_ * sr_); }
    void setSustain(F v) { sustain_ = v; }
    void setRelease(F v) { release_ = v; release_c_ = F(8) / std::max(F(8), release_ * sr_); }

    /** Sets the envelope generator to a particular state. */
    void setState(EnvelopeState s) { state_ = s; }

    // -------- processing -----------

    /** Starts the envelope generator */
    void trigger(F init_value = F(0)) { active_ = true; state_ = ENV_ATTACK; value_ = init_value; }

    /** Stops the envelope generator */
    void stop() { active_ = false; value_ = F(0); }

    /** Forwards the envelop by one sample and returns the value */
    F next();

    /** Generates @p blockSize samples of the envelop in @p output.
        @p stride is the number of samples to forward in @p output for each sample. */
    void process(F * output, int blockSize, int stride = 1);

    // __________ PRIVATE ____________

private:

    void calcCoeffs_();

    EnvelopeState state_;

    bool active_;

    int sr_;

    F value_,
      attack_,
      decay_,
      sustain_,
      release_,
      attack_c_,
      decay_c_,
      release_c_;

};


// ______________________ IMPL ______________________

template <typename F>
EnvelopeGenerator<F>::EnvelopeGenerator()
    : state_    (ENV_ATTACK),
      active_   (false),
      sr_       (44100),
      value_    (0.0),
      attack_   (0.05),
      decay_    (1.0),
      sustain_  (0.0),
      release_  (1.0)
{
    calcCoeffs_();
}

template <typename F>
void EnvelopeGenerator<F>::calcCoeffs_()
{
    attack_c_ =  F(8) / std::max(F(8), attack_ * sr_);
    decay_c_ =   F(8) / std::max(F(8), decay_ * sr_);
    release_c_ = F(8) / std::max(F(8), release_ * sr_);
}


template <typename F>
F EnvelopeGenerator<F>::next()
{
    if (!active_)
        return F(0);

    switch (state_)
    {
        case ENV_ATTACK:
            value_ += attack_c_ * (F(1) - value_);

            if (value_ >= F(0.999))
                state_ = ENV_DECAY;
        break;

        case ENV_DECAY:
            value_ += decay_c_ * (sustain_ - value_);

            if (std::abs(value_ - sustain_) < F(0.001))
            {
                if (sustain_ > 0)
                    state_ = ENV_SUSTAIN;
                else
                    stop();
            }

        break;

        case ENV_RELEASE:
            value_ -= release_c_ * value_;

            if (value_ <= 0.0001)
                stop();

        break;

        case ENV_SUSTAIN:
        break;
    }

    return value_;
}

template <typename F>
void EnvelopeGenerator<F>::process(F * output, int count, int stride)
{
    for (int i=0; i<count; ++i, output += stride)
        *output = next();
}



} // namespace Sonot

#endif // SONOTSRC_ENVELOPEGENERATOR_H

