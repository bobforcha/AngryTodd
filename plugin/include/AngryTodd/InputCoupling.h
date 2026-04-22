// =============================================================================
// InputCoupling.h
//
// Models the input coupling network of the Angry Todd amplifier.
//
// Circuit: J1 (tip) ── C1 (56nF) ──┬── R2 (62kΩ) ── V1B grid
//                                    │
//                                   R1 (1MΩ)
//                                    │
//                                   GND
//
// R2 is a series grid stopper (not a shunt), so C1 sees R1 (1MΩ) as its
// discharge path to ground. The highpass corner is:
//   fc = 1 / (2π * C1 * R1) ≈ 2.84 Hz
//
// R2 (62kΩ) in series with the grid prevents parasitic HF oscillation
// but does not significantly affect the audio-band frequency response.
// It does form a lowpass with the grid's input capacitance (~1.6pF for
// a 12AX7 Cgk + Miller-multiplied Cga), but at fc > 1 MHz — well above
// audio and handled implicitly by the sample rate.
//
// =============================================================================
#pragma once

#include <juce_dsp/juce_dsp.h>

class InputCoupling
{
public:
    InputCoupling();
    ~InputCoupling();

    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (juce::dsp::AudioBlock<float>& block);
    void reset();

private:
    static constexpr double C1 = 56.0e-9;       // 56nF coupling cap
    static constexpr double R1 = 1.0e6;          // 1MΩ grid leak to ground

    juce::dsp::IIR::Filter<float> hpFilter;
    double sampleRate = 44100.0;

    void updateCoefficients();
};
