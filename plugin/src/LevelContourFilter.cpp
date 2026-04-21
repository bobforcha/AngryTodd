// =============================================================================
// LevelContourFilter.cpp
//
// Implementation of the LEVEL + HIGH CONTOUR tone network.
//
// Signal flow:
//   1. C13 highpass (input coupling from V2B plate)
//   2. Split into two parallel paths:
//      A: R15 lowpass (C14 treble shunt) → LEVEL attenuation
//      B: C15 highpass → HIGH CONTOUR attenuation
//   3. Sum both paths → V2A grid
//
// Path A filter analysis:
//   R15 (100kΩ) feeds node_Y, loaded by P3 (200kΩ) to ground and C14 (820pF).
//   The lowpass is formed by R15 and the parallel load C14 || P3:
//     fc = 1 / (2π · R15 · C14)  ≈ 1.94 kHz (approximate, ignoring P3 loading)
//   P3 voltage divider: output = V_nodeY · (1 - level_pos)
//
// Path B filter analysis:
//   C15 (10nF) in series with P4 (variable 0–1MΩ) to the output.
//   Acts as a highpass: fc = 1 / (2π · C15 · R_load)
//   P4 attenuates by varying the series resistance relative to the grid impedance.
//   Since grid impedance >> P4, P4 mainly affects the signal amplitude via
//   voltage divider with the downstream impedance. For DSP, we model this as
//   a highpass followed by variable attenuation.
//
// =============================================================================

#include "LevelContourFilter.h"

LevelContourFilter::LevelContourFilter() {}
LevelContourFilter::~LevelContourFilter() {}

void LevelContourFilter::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    inputHpFilters.clear();
    levelLpFilters.clear();
    contourHpFilters.clear();

    juce::dsp::ProcessSpec monoSpec { spec.sampleRate, spec.maximumBlockSize, 1 };

    for (juce::uint32 ch = 0; ch < spec.numChannels; ++ch)
    {
        inputHpFilters.emplace_back();
        inputHpFilters.back().prepare (monoSpec);

        levelLpFilters.emplace_back();
        levelLpFilters.back().prepare (monoSpec);

        contourHpFilters.emplace_back();
        contourHpFilters.back().prepare (monoSpec);
    }

    tempBuffer.setSize (static_cast<int> (spec.numChannels),
                        static_cast<int> (spec.maximumBlockSize));

    updateCoefficients();
}

void LevelContourFilter::process (juce::dsp::AudioBlock<float>& block)
{
    auto numChannels = block.getNumChannels();
    auto numSamples  = block.getNumSamples();

    // Ensure temp buffer is big enough
    tempBuffer.setSize (static_cast<int> (numChannels),
                        static_cast<int> (numSamples), false, false, true);

    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* data = block.getChannelPointer (ch);
        auto* temp = tempBuffer.getWritePointer (static_cast<int> (ch));

        // --- 1. Input coupling HPF (C13) ---
        auto chanBlock = juce::dsp::AudioBlock<float> (&data, 1, numSamples);
        auto ctx = juce::dsp::ProcessContextReplacing<float> (chanBlock);
        inputHpFilters[ch].process (ctx);

        // --- 2. Copy input to temp for path B before path A modifies it ---
        std::memcpy (temp, data, numSamples * sizeof (float));

        // --- 3. Path A: LEVEL (lowpass + attenuation) ---
        auto chanBlockA = juce::dsp::AudioBlock<float> (&data, 1, numSamples);
        auto ctxA = juce::dsp::ProcessContextReplacing<float> (chanBlockA);
        levelLpFilters[ch].process (ctxA);

        // --- 4. Path B: HIGH CONTOUR (highpass) ---
        auto tempBlock = juce::dsp::AudioBlock<float> (&temp, 1, numSamples);
        auto ctxB = juce::dsp::ProcessContextReplacing<float> (tempBlock);
        contourHpFilters[ch].process (ctxB);

        // --- 5. Mix both paths accounting for P3 R_lower loading ---
        //
        // In the real circuit, both paths meet at V2A grid. P3's lower
        // section (R_lower = (1-level) * 200kΩ) provides a path to ground
        // that loads the contour path. The contour signal is attenuated by:
        //
        //   R_lower / (P4_R + R_lower)
        //
        // where P4_R = (1-contour) * 1MΩ.
        //
        // When LEVEL is muted (R_lower = 0), the output node is shorted
        // to ground and both paths are killed.

        // levelAmount: 0 = muted, 1 = full volume (audio taper)
        float lvl = levelAmount * levelAmount;
        float R_lower = lvl * static_cast<float> (P3_total);
        float P4_R = (1.0f - highContourAmount) * static_cast<float> (P4_max);
        float contourGain = (P4_R + R_lower > 0.0f)
                          ? R_lower / (P4_R + R_lower)
                          : 0.0f;
        float levelGain = lvl;

        for (size_t n = 0; n < numSamples; ++n)
            data[n] = data[n] * levelGain + temp[n] * contourGain;
    }
}

void LevelContourFilter::reset()
{
    for (auto& f : inputHpFilters)   f.reset();
    for (auto& f : levelLpFilters)   f.reset();
    for (auto& f : contourHpFilters) f.reset();
}

void LevelContourFilter::setLevel (float normalised)
{
    levelAmount = juce::jlimit (0.0f, 1.0f, normalised);
}

void LevelContourFilter::setHighContour (float normalised)
{
    highContourAmount = juce::jlimit (0.0f, 1.0f, normalised);
}

void LevelContourFilter::updateCoefficients()
{
    double fs = sampleRate;

    // --- Input coupling HPF: C13 into R15 || P3_total load ---
    // Load on C13: R15 in series with (P3 || C14), approximately R15 + P3 ≈ 300kΩ
    // For HPF corner: fc = 1 / (2π · C13 · R_load)
    double R_load_c13 = R15 + P3_total;
    double fc_input = 1.0 / (2.0 * juce::MathConstants<double>::pi * C13 * R_load_c13);

    for (auto& f : inputHpFilters)
        f.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (
            fs, static_cast<float> (fc_input));

    // --- Path A lowpass: R15 + C14 shunt ---
    // C14 (820pF) shunts node_Y to ground. With R15 (100kΩ) as source impedance
    // and P3 (200kΩ) as parallel load:
    //   R_eff = R15 || P3_total ≈ 66.7kΩ (but R15 is series, P3 is shunt...)
    //
    // More accurately: R15 feeds into node_Y loaded by C14 || P3.
    // The lowpass is: fc = 1 / (2π · R15 · C14) ≈ 1.94 kHz
    // (P3 provides a DC path but doesn't significantly affect the LP frequency)
    double fc_lp = 1.0 / (2.0 * juce::MathConstants<double>::pi * R15 * C14);

    for (auto& f : levelLpFilters)
        f.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass (
            fs, static_cast<float> (fc_lp));

    // --- Path B highpass: C15 ---
    // C15 (10nF) in series. The load is the grid impedance (very high),
    // so the HPF is set by C15 and the effective source/load impedance.
    // From node_X, source impedance ≈ 0 (driven by C13 coupling).
    // Load = P4 (up to 1MΩ) + grid impedance.
    // Use P4_max as load for the HPF corner: fc = 1 / (2π · C15 · P4_max) ≈ 15.9 Hz
    // In practice, the HPF corner is very low — C15 mainly blocks DC and passes audio.
    // The attenuation is handled by the highContourAmount parameter.
    double fc_hp = 1.0 / (2.0 * juce::MathConstants<double>::pi * C15 * P4_max);

    for (auto& f : contourHpFilters)
        f.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (
            fs, static_cast<float> (fc_hp));
}
