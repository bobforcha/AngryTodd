// =============================================================================
// MasterSection.cpp
//
// LIMIT→V4A coupling, cathode follower buffer, and master volume section.
//
// Coupling filter analysis:
//   Z_series = R24 || (1/sC24) = R24 / (1 + s·R24·C24)
//   Then C25 in series, R25 to ground.
//
//   H(s) = (s·τ2 + s²·τ1·τ2) / (1 + s·(τ1+τ2+τ3) + s²·τ1·τ2)
//
//   where τ1 = R24·C24, τ2 = C25·R25, τ3 = C25·R24
//
//   DC: blocked (C25). HF: unity (C24 shorts R24).
//   Treble boost starts at f = 1/(2π·R24·C24) ≈ 2.46 kHz
//
// =============================================================================

#include "MasterSection.h"

MasterSection::MasterSection() {}
MasterSection::~MasterSection() {}

void MasterSection::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    couplingFilters.clear();
    masterHpFilters.clear();

    juce::dsp::ProcessSpec monoSpec { spec.sampleRate, spec.maximumBlockSize, 1 };

    for (juce::uint32 ch = 0; ch < spec.numChannels; ++ch)
    {
        couplingFilters.emplace_back();
        couplingFilters.back().prepare (monoSpec);

        masterHpFilters.emplace_back();
        masterHpFilters.back().prepare (monoSpec);
    }

    updateCouplingCoefficients();
    updateMasterHpCoefficients();
}

void MasterSection::process (juce::dsp::AudioBlock<float>& block)
{
    auto numChannels = block.getNumChannels();
    auto numSamples  = block.getNumSamples();

    float masterGain = boostEngaged ? boostMasterAmount : masterAmount;

    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* data = block.getChannelPointer (ch);

        // --- 1. Coupling filter (R24||C24 + C25 + R25) ---
        auto chanBlock = juce::dsp::AudioBlock<float> (&data, 1, numSamples);
        auto ctx = juce::dsp::ProcessContextReplacing<float> (chanBlock);
        couplingFilters[ch].process (ctx);

        // --- 2. V4A cathode follower: unity gain, no processing ---

        // --- 3. Master volume HPF (C26 + C28 coupling) ---
        auto ctx2 = juce::dsp::ProcessContextReplacing<float> (chanBlock);
        masterHpFilters[ch].process (ctx2);

        // --- 4. Master volume attenuation (audio taper) ---
        float gain = masterGain * masterGain;  // audio taper
        for (size_t n = 0; n < numSamples; ++n)
            data[n] *= gain;
    }
}

void MasterSection::reset()
{
    for (auto& f : couplingFilters)   f.reset();
    for (auto& f : masterHpFilters)   f.reset();
}

void MasterSection::setMaster (float normalised)
{
    masterAmount = juce::jlimit (0.0f, 1.0f, normalised);
}

void MasterSection::setBoostMaster (float normalised)
{
    boostMasterAmount = juce::jlimit (0.0f, 1.0f, normalised);
}

void MasterSection::setBoostEngaged (bool engaged)
{
    boostEngaged = engaged;
}

void MasterSection::updateCouplingCoefficients()
{
    // H(s) = (s·τ2 + s²·τ1·τ2) / (1 + s·(τ1+τ2+τ3) + s²·τ1·τ2)
    double t1 = R24 * C24;          // 64.8µs
    double t2 = C25 * R25;          // 0.1s
    double t3 = C25 * R24;          // 24ms

    double a1s = t2;
    double a2s = t1 * t2;
    double b0s = 1.0;
    double b1s = t1 + t2 + t3;
    double b2s = t1 * t2;

    // Bilinear transform
    double c  = 2.0 * sampleRate;
    double c2 = c * c;

    // Numerator: a1s*s + a2s*s²  (no s⁰ term)
    // (z+1)² coefficient for s⁰: not present
    // c(z-1)(z+1) for s¹: c(z² - 1)
    // c²(z-1)² for s²: c²(z² - 2z + 1)
    double nb0 = a1s * c + a2s * c2;
    double nb1 = -a1s * c * 0.0 + a2s * c2 * (-2.0);
    // Let me use the proper expansion for 2nd order:
    //
    // s⁰: (z+1)² = z² + 2z + 1
    // s¹: c(z-1)(z+1) = c(z² - 1)
    // s²: c²(z-1)² = c²(z² - 2z + 1)

    double Nb0 = a1s * c + a2s * c2;
    double Nb1 = -2.0 * a2s * c2;
    double Nb2 = -a1s * c + a2s * c2;

    double Db0 = b0s + b1s * c + b2s * c2;
    double Db1 = 2.0 * b0s - 2.0 * b2s * c2;
    double Db2 = b0s - b1s * c + b2s * c2;

    for (auto& f : couplingFilters)
        *f.coefficients = juce::dsp::IIR::Coefficients<float> (
            static_cast<float> (Nb0 / Db0),
            static_cast<float> (Nb1 / Db0),
            static_cast<float> (Nb2 / Db0),
            1.0f,
            static_cast<float> (Db1 / Db0),
            static_cast<float> (Db2 / Db0));
}

void MasterSection::updateMasterHpCoefficients()
{
    // C26 (100nF) and C28 (100nF) in series with the pot.
    // Combined series capacitance: C_total = C26 * C28 / (C26 + C28) = 50nF
    // Load: selected pot (100kΩ)
    // fc = 1/(2π · C_total · R_pot) ≈ 31.8 Hz
    double C_total = (C26 * C28) / (C26 + C28);
    double R_pot = boostEngaged ? P10_total : P11_total;
    double fc = 1.0 / (2.0 * juce::MathConstants<double>::pi * C_total * R_pot);

    for (auto& f : masterHpFilters)
        f.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (
            sampleRate, static_cast<float> (fc));
}
