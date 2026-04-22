// =============================================================================
// TriodeStage.h
//
// Models a single ECC83 (12AX7) common-cathode triode gain stage.
//
// DSP signal flow:
//   1. Gain filter — models the cathode bypass cap(s) effect on
//      frequency-dependent gain. Implemented as a biquad when a second
//      bypass path with potentiometer is present (e.g. V1B LOW CONTOUR),
//      or a first-order shelf for single-bypass stages.
//   2. Waveshaper — asymmetric soft clipping modeled after 12AX7 transfer
//      characteristics (softer saturation, harder cutoff).
//
// Cathode circuit for V1B:
//
//   V1B cathode ──┬── R3 (2.7kΩ) ── GND      [bias]
//                  ├── C3 (470nF) ── GND      [primary bypass]
//                  └── C5b (33µF) ── P1 wiper
//                                     │
//                                  (0-50kΩ)
//                                     │
//                                    GND       [variable bass bypass]
//
// Interstage coupling is modeled separately by InterstageFilter.
//
// =============================================================================
#pragma once

#include <juce_dsp/juce_dsp.h>

struct TriodeStageConfig
{
    // ECC83/12AX7 tube parameters
    double mu = 100.0;          // amplification factor
    double rp = 62500.0;        // plate resistance (Ω)

    // Circuit components
    double Ra;                   // plate load resistance (Ω)
    double Rk;                   // cathode resistance (Ω)
    double Ck;                   // primary cathode bypass capacitance (F), 0 = no bypass

    // Optional second cathode bypass path (cap in series with pot to ground)
    double Ck2 = 0.0;           // second bypass capacitance (F), 0 = none
    double RpotMax = 0.0;       // pot max resistance (Ω), 0 = none
};

// Pre-defined configurations for each stage in the Angry Todd circuit
namespace AngryToddStages
{
    // V1B: Input gain stage
    // R4=100kΩ plate, R3=2.7kΩ cathode
    // C3=470nF primary bypass, C5b=33µF secondary bypass, P1=50kΩ LOW CONTOUR
    inline TriodeStageConfig V1B()
    {
        return { 100.0, 62500.0, 100e3, 2.7e3, 470e-9, 33e-6, 50e3 };
    }

    // V1A: Second gain stage
    // R9=100kΩ plate, R8=2.7kΩ cathode, C7=470nF bypass
    inline TriodeStageConfig V1A()
    {
        return { 100.0, 62500.0, 100e3, 2.7e3, 470e-9 };
    }

    // V2B: Third gain stage
    // R14=100kΩ plate, R13=2.7kΩ cathode, C11=470nF bypass
    // C12=33µF secondary bypass, P2=50kΩ BOOSTER FAT (with BOOST relay)
    inline TriodeStageConfig V2B()
    {
        return { 100.0, 62500.0, 100e3, 2.7e3, 470e-9, 33e-6, 50e3 };
    }

    // V2A: Fourth gain stage
    // R17=100kΩ plate, R16=2.7kΩ cathode, C16=470nF bypass
    inline TriodeStageConfig V2A()
    {
        return { 100.0, 62500.0, 100e3, 2.7e3, 470e-9 };
    }

    // V3A: Fifth gain stage
    // R22=100kΩ plate, R21=2.7kΩ cathode, C19=470nF bypass
    inline TriodeStageConfig V3A()
    {
        return { 100.0, 62500.0, 100e3, 2.7e3, 470e-9 };
    }
}

class TriodeStage
{
public:
    explicit TriodeStage (const TriodeStageConfig& config);
    ~TriodeStage();

    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (juce::dsp::AudioBlock<float>& block);
    void reset();

    // Set the cathode pot resistance in ohms (0 to RpotMax).
    // Call this from the audio thread to update the LOW CONTOUR control.
    void setCathodePotResistance (float ohms);

private:
    TriodeStageConfig config;
    double sampleRate = 44100.0;
    double currentPotR = 0.0;
    bool hasDualBypass = false;

    // Per-channel gain filters (first or second order depending on config)
    std::vector<juce::dsp::IIR::Filter<float>> gainFilters;

    void updateCoefficients();

    // 12AX7-style asymmetric soft clipping
    static float waveshape (float x);
};
