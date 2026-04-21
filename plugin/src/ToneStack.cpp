// =============================================================================
// ToneStack.cpp
//
// Marshall JCM800-style TMB tone stack with LIMIT and BOOSTER HIGH CUT.
// =============================================================================

#include "ToneStack.h"

ToneStack::ToneStack() {}
ToneStack::~ToneStack() {}

void ToneStack::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    filterStates.clear();
    filterStates.resize (spec.numChannels);

    highCutFilters.clear();
    juce::dsp::ProcessSpec monoSpec { spec.sampleRate, spec.maximumBlockSize, 1 };
    for (juce::uint32 ch = 0; ch < spec.numChannels; ++ch)
    {
        highCutFilters.emplace_back();
        highCutFilters.back().prepare (monoSpec);
    }

    updateTmbCoefficients();
    updateHighCutCoefficients();
}

void ToneStack::process (juce::dsp::AudioBlock<float>& block)
{
    auto numChannels = block.getNumChannels();
    auto numSamples  = block.getNumSamples();

    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* data = block.getChannelPointer (ch);
        auto& s = filterStates[ch];

        for (size_t n = 0; n < numSamples; ++n)
        {
            // --- TMB tone stack (3rd-order IIR) ---
            float x = data[n];
            float y = b0 * x + b1 * s.x1 + b2 * s.x2 + b3 * s.x3
                             - a1 * s.y1 - a2 * s.y2 - a3 * s.y3;

            s.x3 = s.x2; s.x2 = s.x1; s.x1 = x;
            s.y3 = s.y2; s.y2 = s.y1; s.y1 = y;

            // --- LIMIT attenuator (P8) ---
            data[n] = y * limitAmount;
        }

        // --- BOOSTER HIGH CUT (C23 + P9, only when BOOST is on) ---
        if (highCutEnabled)
        {
            auto chanBlock = juce::dsp::AudioBlock<float> (&data, 1, numSamples);
            auto ctx = juce::dsp::ProcessContextReplacing<float> (chanBlock);
            highCutFilters[ch].process (ctx);
        }
    }
}

void ToneStack::reset()
{
    for (auto& s : filterStates)
        s = {};
    for (auto& f : highCutFilters)
        f.reset();
}

void ToneStack::setTreble (float normalised)
{
    treble = juce::jlimit (0.01f, 1.0f, normalised);
    updateTmbCoefficients();
}

void ToneStack::setBass (float normalised)
{
    bass = juce::jlimit (0.01f, 1.0f, normalised);
    updateTmbCoefficients();
}

void ToneStack::setMid (float normalised)
{
    mid = juce::jlimit (0.01f, 1.0f, normalised);
    updateTmbCoefficients();
}

void ToneStack::setLimit (float normalised)
{
    limitAmount = juce::jlimit (0.0f, 1.0f, normalised);
}

void ToneStack::setBoosterHighCut (float normalised)
{
    highCutAmount = juce::jlimit (0.01f, 1.0f, normalised);
    updateHighCutCoefficients();
}

void ToneStack::setBoostHighCutEnabled (bool enabled)
{
    highCutEnabled = enabled;
    if (! enabled)
    {
        // Reset filter state when disabling to avoid transients
        for (auto& f : highCutFilters)
            f.reset();
    }
}

void ToneStack::updateTmbCoefficients()
{
    double t = static_cast<double> (treble);
    double b = static_cast<double> (bass);
    double m = static_cast<double> (mid);

    // =================================================================
    // Marshall-type TMB tone stack transfer function coefficients.
    //
    // H(s) = (b1*s + b2*s² + b3*s³) / (a0 + a1*s + a2*s² + a3*s³)
    // =================================================================

    double sb1 = t * C1 * R1
               + m * C3 * RM
               + b * (C1 * R1 + C2 * R1)
               + b * m * C3 * RM;

    double sb2 = t * (C1 * C2 * R1 * R1 + C1 * C2 * R1 * RT)
               + m * b * (C1 * C3 * R1 * RM + C2 * C3 * RM * R1)
               + m * (C2 * C3 * RM * R1);

    double sb3 = m * b * t * C1 * C2 * C3 * R1 * RM * RT
               + m * t * C1 * C2 * C3 * R1 * R1 * RM;

    double sa0 = 1.0;

    double sa1 = (C1 * R1 + C1 * RT + C2 * R1 + C3 * RM)
               + m * C3 * RM
               + b * (C2 * R1 + C2 * RB);

    double sa2 = (C1 * C2 * R1 * RT + C1 * C2 * R1 * R1 + C2 * C3 * RM * R1
                  + C1 * C3 * R1 * RM + C1 * C3 * RT * RM)
               + m * (C2 * C3 * RM * R1 + C1 * C3 * RM * R1)
               + b * (C1 * C2 * R1 * RB + C2 * C3 * RB * RM + C1 * C2 * RB * RT);

    double sa3 = (C1 * C2 * C3 * R1 * R1 * RM + C1 * C2 * C3 * R1 * RT * RM)
               + m * C1 * C2 * C3 * R1 * RM * R1
               + b * (C1 * C2 * C3 * R1 * RB * RM + C1 * C2 * C3 * RB * RT * RM);

    // Bilinear transform: s = c*(z-1)/(z+1), c = 2*fs
    double c = 2.0 * sampleRate;
    double c2 = c * c;
    double c3 = c2 * c;

    // Expand: multiply N(s) and D(s) by (z+1)³ after substituting s
    //
    // (z+1)³           → z³ + 3z² + 3z + 1        [s⁰ coefficient]
    // c(z-1)(z+1)²     → c(z³ + z² - z - 1)       [s¹ coefficient]
    // c²(z-1)²(z+1)    → c²(z³ - z² - z + 1)      [s² coefficient]
    // c³(z-1)³         → c³(z³ - 3z² + 3z - 1)     [s³ coefficient]

    double nb0 = sb1 * c + sb2 * c2 + sb3 * c3;
    double nb1 = sb1 * c - sb2 * c2 - 3.0 * sb3 * c3;
    double nb2 = -sb1 * c - sb2 * c2 + 3.0 * sb3 * c3;
    double nb3 = -sb1 * c + sb2 * c2 - sb3 * c3;

    double na0 = sa0 + sa1 * c + sa2 * c2 + sa3 * c3;
    double na1 = 3.0 * sa0 + sa1 * c - sa2 * c2 - 3.0 * sa3 * c3;
    double na2 = 3.0 * sa0 - sa1 * c - sa2 * c2 + 3.0 * sa3 * c3;
    double na3 = sa0 - sa1 * c + sa2 * c2 - sa3 * c3;

    this->b0 = static_cast<float> (nb0 / na0);
    this->b1 = static_cast<float> (nb1 / na0);
    this->b2 = static_cast<float> (nb2 / na0);
    this->b3 = static_cast<float> (nb3 / na0);
    this->a1 = static_cast<float> (na1 / na0);
    this->a2 = static_cast<float> (na2 / na0);
    this->a3 = static_cast<float> (na3 / na0);
}

void ToneStack::updateHighCutCoefficients()
{
    // C23 (1.5nF) in series with P9 wiper (variable 0–1MΩ) to ground.
    // At the tone stack output, this creates a lowpass shunt.
    //
    // The cut frequency depends on P9's wiper resistance and the source
    // impedance at the node. With P8 (1MΩ) as the load:
    //   fc ≈ 1 / (2π * C23 * R_eff)
    //
    // P9 wiper resistance: (1 - highCutAmount) * P9_total
    //   highCut = 0 → max resistance (1MΩ) → lowest fc → most cut
    //   Wait, that's backwards. Let me think...
    //   highCut = 0 → max cut → lowest fc → P9 resistance should be LOW
    //   highCut = 1 → no cut → highest fc → P9 resistance should be HIGH
    //
    // P9 pin 1 floating, wiper from C23, pin 3 to ground.
    // When wiper is near pin 3 (ground): low resistance → more treble shunted
    // When wiper is near pin 1 (floating): high resistance → less treble shunted
    //
    // R_p9 = highCutAmount * P9_total (0 = near ground = max cut, 1 = far = no cut)

    // Audio taper approximation: x² curve gives a log-like response
    double hc = static_cast<double> (highCutAmount);
    double R_p9 = hc * hc * P9_total;

    // Source impedance at the node is complex (tone stack output impedance
    // in parallel with P8). Approximate as P8_total for simplicity.
    // The lowpass: fc = 1 / (2π * C23 * (R_p9 + R_source_eff))
    // But since C23 is in series with P9, and the shunt is C23+P9 to ground,
    // the lowpass is formed by the source impedance and C23+P9:
    //   fc = 1 / (2π * C23 * R_p9)  when R_p9 dominates
    //
    // For P9 near 0: fc → very high (C23 shorts to ground, but barely any
    //   impedance so cut is aggressive)
    // For P9 near 1MΩ: fc = 1/(2π * 1.5nF * 1MΩ) ≈ 106 Hz (strong cut)
    //
    // Actually, with P9 as series R to ground and C23 in series:
    // Shunt impedance = 1/(sC23) + R_p9
    // Lowpass formed with source impedance R_source:
    //   H(s) ≈ Zshunt / (R_source + Zshunt)
    // This rolls off when |Zshunt| < R_source, i.e., when sC23 is large enough.
    //
    // Simpler model: just a first-order lowpass at fc based on C23 and the
    // parallel combination of R_p9 and the load impedance.

    // Use a simple lowpass with variable cutoff
    double fc = 1.0 / (2.0 * juce::MathConstants<double>::pi * C23 * (R_p9 + 1.0));

    // Clamp fc to reasonable range
    fc = juce::jlimit (100.0, 20000.0, fc);

    for (auto& f : highCutFilters)
        f.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass (
            sampleRate, static_cast<float> (fc));
}
