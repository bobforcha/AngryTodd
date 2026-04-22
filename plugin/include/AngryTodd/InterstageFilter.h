// =============================================================================
// InterstageFilter.h
//
// Models the coupling and tone-shaping network between two triode stages.
//
// V1B → V1A interstage topology:
//
//   V1B plate ── C_in (22nF) ── node_A ──┬── R_s1 (470kΩ) ──────────────┬── node_B
//                                         ├── C_s2 (22nF) ── R_s2 (560kΩ)┘   (V1A grid)
//                                         │                                │
//                                         C_shunt (1.5nF)               R_shunt (62kΩ)
//                                         │                                │
//                                        GND                             GND
//
// Transfer function (second-order bandpass):
//
//   H(s) = s·C_in·R_shunt·(1 + s·C_s2·S)
//        / (1 + s·(Ct·P + C_s2·S) + s²·C_s2·Q·Ct)
//
//   where:
//     Ct = C_in + C_shunt
//     P  = R_s1 + R_shunt
//     Q  = R_s1·R_shunt + R_s1·R_s2 + R_shunt·R_s2
//     S  = R_s1 + R_s2
//
// =============================================================================
#pragma once

#include <juce_dsp/juce_dsp.h>

struct InterstageConfig
{
    double C_in;        // input coupling capacitor (F)
    double C_shunt;     // HF shunt cap at node_A to ground (F), 0 = none
    double R_s1;        // primary series resistor to output node (Ω)
    double R_shunt;     // shunt resistor at output node to ground (Ω)
    double C_s2;        // second path: coupling cap (F), 0 = no second path
    double R_s2;        // second path: series resistor (Ω), 0 = no second path
};

namespace AngryToddInterstages
{
    // V1B → V1A coupling network
    // C4=22nF, C5a=1.5nF, R5=470kΩ, R6=62kΩ, C6=22nF, R7=560kΩ
    inline InterstageConfig V1B_to_V1A()
    {
        return { 22e-9, 1.5e-9, 470e3, 62e3, 22e-9, 560e3 };
    }

    // V1A → V2B coupling network
    // C8=22nF, C9=1.5nF shunt, R11=470kΩ, R12=62kΩ, no second path
    inline InterstageConfig V1A_to_V2B()
    {
        return { 22e-9, 1.5e-9, 470e3, 62e3, 0.0, 0.0 };
    }

    // V2A → V3A coupling network
    // C17=22nF, C18=680pF shunt, R19=470kΩ, R20=62kΩ, no second path
    inline InterstageConfig V2A_to_V3A()
    {
        return { 22e-9, 680e-12, 470e3, 62e3, 0.0, 0.0 };
    }
}

class InterstageFilter
{
public:
    explicit InterstageFilter (const InterstageConfig& config);
    ~InterstageFilter();

    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (juce::dsp::AudioBlock<float>& block);
    void reset();

private:
    InterstageConfig config;
    double sampleRate = 44100.0;

    // Per-channel biquad filters
    std::vector<juce::dsp::IIR::Filter<float>> filters;

    void updateCoefficients();
};
