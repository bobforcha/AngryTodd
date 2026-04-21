// =============================================================================
// ToneStack.h
//
// Marshall JCM800-style TMB (Treble/Mid/Bass) tone stack with LIMIT and
// BOOSTER HIGH CUT controls.
//
// Core tone stack: R24 (56kΩ) slope, C20 (270pF) treble, C21 (100nF) bass,
// C22 (56nF) mid, P5 (200kΩ) treble, P6 (200kΩ) bass, P7 (25kΩ) mid.
//
// At the tone stack output node:
//   P8 (1MΩ LIMIT) — volume attenuator (pin1=input, pin3=GND, wiper=output)
//   C23 (1.5nF) → P9 (1MΩ BOOSTER HIGH CUT) — switchable treble shunt
//     BOOST OFF: P9 pin3 floating → no effect
//     BOOST ON:  P9 pin3 to GND → variable treble cut
//
// =============================================================================
#pragma once

#include <juce_dsp/juce_dsp.h>

class ToneStack
{
public:
    ToneStack();
    ~ToneStack();

    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (juce::dsp::AudioBlock<float>& block);
    void reset();

    void setTreble (float normalised);  // 0–1
    void setBass (float normalised);    // 0–1
    void setMid (float normalised);     // 0–1
    void setLimit (float normalised);   // 0 = muted, 1 = full
    void setBoosterHighCut (float normalised); // 0 = max cut, 1 = no cut
    void setBoostHighCutEnabled (bool enabled); // BOOST relay switches P9

private:
    // TMB tone stack components
    static constexpr double R1 = 56e3;      // 56kΩ slope
    static constexpr double C1 = 270e-12;   // 270pF treble
    static constexpr double C2 = 100e-9;    // 100nF bass
    static constexpr double C3 = 56e-9;     // 56nF mid
    static constexpr double RT = 200e3;     // 200kΩ treble pot
    static constexpr double RB = 200e3;     // 200kΩ bass pot
    static constexpr double RM = 25e3;      // 25kΩ mid pot

    // LIMIT + HIGH CUT components
    static constexpr double P8_total = 1e6;  // 1MΩ LIMIT pot
    static constexpr double C23 = 1.5e-9;    // 1.5nF high cut cap
    static constexpr double P9_total = 1e6;  // 1MΩ HIGH CUT pot

    double sampleRate = 44100.0;
    float treble = 0.5f;
    float bass = 0.5f;
    float mid = 0.5f;
    float limitAmount = 1.0f;       // 0 = muted, 1 = full
    float highCutAmount = 1.0f;     // 0 = max treble cut, 1 = no cut
    bool highCutEnabled = false;    // controlled by BOOST relay

    // Per-channel 3rd-order IIR filter states (TMB)
    struct ThirdOrderState
    {
        float x1 = 0, x2 = 0, x3 = 0;
        float y1 = 0, y2 = 0, y3 = 0;
    };
    std::vector<ThirdOrderState> filterStates;

    // TMB filter coefficients (3rd order)
    float b0 = 0, b1 = 0, b2 = 0, b3 = 0;
    float a1 = 0, a2 = 0, a3 = 0;

    // Per-channel high-cut lowpass filter (C23 + P9)
    std::vector<juce::dsp::IIR::Filter<float>> highCutFilters;

    void updateTmbCoefficients();
    void updateHighCutCoefficients();
};
