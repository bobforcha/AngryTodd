// =============================================================================
// MasterSection.h
//
// Models everything from LIMIT output through V4A to V4B grid:
//
// 1. LIMIT→V4A coupling: R24(240k)||C24(270pF) → C25(100nF) → R25(1MΩ) grid leak
//    This is a second-order highpass with treble boost (C24 bypasses R24 at HF).
//
// 2. V4A cathode follower: unity gain buffer (no DSP processing).
//
// 3. Master volume: C26(100nF) coupling → P10 or P11 (switched by BOOST relay)
//    BOOST OFF: P11 (Master, 100kΩ) active
//    BOOST ON:  P10 (Boost Master, 100kΩ) active
//
// 4. C28 (100nF) coupling to V4B grid.
//
// =============================================================================
#pragma once

#include <juce_dsp/juce_dsp.h>

class MasterSection
{
public:
    MasterSection();
    ~MasterSection();

    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (juce::dsp::AudioBlock<float>& block);
    void reset();

    void setMaster (float normalised);       // P11, 0 = muted, 1 = full
    void setBoostMaster (float normalised);  // P10, 0 = muted, 1 = full
    void setBoostEngaged (bool engaged);     // REL2 selects P10 vs P11

private:
    // Coupling filter components (LIMIT → V4A)
    static constexpr double R24 = 240e3;
    static constexpr double C24 = 270e-12;
    static constexpr double C25 = 100e-9;
    static constexpr double R25 = 1e6;

    // Master volume components
    static constexpr double C26 = 100e-9;
    static constexpr double P10_total = 100e3;
    static constexpr double P11_total = 100e3;
    static constexpr double C28 = 100e-9;

    double sampleRate = 44100.0;
    float masterAmount = 0.5f;
    float boostMasterAmount = 0.5f;
    bool boostEngaged = false;

    // Per-channel coupling biquad (R24||C24 + C25 + R25)
    std::vector<juce::dsp::IIR::Filter<float>> couplingFilters;

    // Per-channel master coupling HPF (C26 into pot, then C28)
    std::vector<juce::dsp::IIR::Filter<float>> masterHpFilters;

    void updateCouplingCoefficients();
    void updateMasterHpCoefficients();
};
