// =============================================================================
// LevelContourFilter.h
//
// Models the LEVEL + HIGH CONTOUR network between V2B and V2A.
//
// Two parallel paths from node_X (after C13 coupling) to V2A grid:
//
//   Path A (LEVEL): R15 (100kΩ) → node_Y → P3 (200kΩ pot) → V2A grid
//                                  │
//                                  C14 (820pF) → GND  [treble shunt]
//
//   Path B (HIGH CONTOUR): C15 (10nF) → P4 (0–1MΩ var. R) → V2A grid
//
// LEVEL controls overall volume (treble-rolled-off by C14).
// HIGH CONTOUR adds variable treble content via C15.
//
// =============================================================================
#pragma once

#include <juce_dsp/juce_dsp.h>

class LevelContourFilter
{
public:
    LevelContourFilter();
    ~LevelContourFilter();

    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (juce::dsp::AudioBlock<float>& block);
    void reset();

    // LEVEL: 0.0 = full volume, 1.0 = muted
    void setLevel (float normalised);

    // HIGH CONTOUR: 0.0 = no treble added, 1.0 = full treble
    void setHighContour (float normalised);

private:
    static constexpr double C13 = 22e-9;     // 22nF input coupling
    static constexpr double R15 = 100e3;      // 100kΩ series to LEVEL path
    static constexpr double C14 = 820e-12;    // 820pF treble shunt at node_Y
    static constexpr double P3_total = 200e3; // 200kΩ LEVEL pot
    static constexpr double C15 = 10e-9;      // 10nF HIGH CONTOUR coupling
    static constexpr double P4_max = 1e6;     // 1MΩ HIGH CONTOUR pot

    double sampleRate = 44100.0;
    float levelAmount = 0.5f;       // 0 = full, 1 = muted
    float highContourAmount = 0.5f; // 0 = none, 1 = full treble

    // Per-channel filters
    // Input coupling HPF (C13)
    std::vector<juce::dsp::IIR::Filter<float>> inputHpFilters;
    // LEVEL path lowpass (R15 + C14)
    std::vector<juce::dsp::IIR::Filter<float>> levelLpFilters;
    // HIGH CONTOUR path highpass (C15)
    std::vector<juce::dsp::IIR::Filter<float>> contourHpFilters;

    // Temp buffer for the parallel path mixing
    juce::AudioBuffer<float> tempBuffer;

    void updateCoefficients();
};
