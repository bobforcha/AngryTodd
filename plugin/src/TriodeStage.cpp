// =============================================================================
// TriodeStage.cpp
//
// Implementation of the ECC83 triode gain stage model.
//
// Gain analysis for a common-cathode stage with dual cathode bypass:
//
//   Cathode impedance:
//     Zk = Rk || (1/sCk) || (1/sCk2 + Rpot)
//
//   This gives a frequency-dependent gain:
//     Av(s) = mu·Ra · N(s) / D(s)
//
//   where N(s) and D(s) are second-order polynomials (see updateCoefficients).
//
//   When only a single bypass cap is present (Ck2 = 0), this reduces to
//   the standard first-order shelf:
//     Av(s) = Av_low · (1 + s/wz) / (1 + s/wp)
//
// =============================================================================

#include "TriodeStage.h"

TriodeStage::TriodeStage (const TriodeStageConfig& cfg)
    : config (cfg),
      currentPotR (cfg.RpotMax),
      hasDualBypass (cfg.Ck2 > 0.0 && cfg.RpotMax > 0.0)
{
}

TriodeStage::~TriodeStage() {}

void TriodeStage::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    gainFilters.clear();
    juce::dsp::ProcessSpec monoSpec { spec.sampleRate, spec.maximumBlockSize, 1 };

    for (juce::uint32 ch = 0; ch < spec.numChannels; ++ch)
    {
        gainFilters.emplace_back();
        gainFilters.back().prepare (monoSpec);
    }

    updateCoefficients();
}

void TriodeStage::process (juce::dsp::AudioBlock<float>& block)
{
    auto numChannels = block.getNumChannels();
    auto numSamples  = block.getNumSamples();

    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* data = block.getChannelPointer (ch);

        // --- 1. Gain filter (cathode bypass frequency shaping + gain) ---
        auto singleChannelBlock = juce::dsp::AudioBlock<float> (&data, 1, numSamples);
        auto context = juce::dsp::ProcessContextReplacing<float> (singleChannelBlock);
        gainFilters[ch].process (context);

        // --- 2. Waveshaper (12AX7 nonlinearity) ---
        for (size_t n = 0; n < numSamples; ++n)
            data[n] = waveshape (data[n]);
    }
}

void TriodeStage::reset()
{
    for (auto& f : gainFilters)
        f.reset();
}

void TriodeStage::setCathodePotResistance (float ohms)
{
    currentPotR = juce::jlimit (0.0, config.RpotMax, static_cast<double> (ohms));
    updateCoefficients();
}

void TriodeStage::updateCoefficients()
{
    double fs = sampleRate;
    double A  = config.Ra + config.rp;               // Ra + rp
    double mu = config.mu;
    double Rk = config.Rk;
    double Ck = config.Ck;

    if (hasDualBypass)
    {
        // =================================================================
        // Biquad gain filter for dual cathode bypass
        //
        // Cathode admittance (common denominator with 1 + s·Rpot·Ck2):
        //
        //   G_total = [n0 + n1·s + n2·s²] / (1 + s·Rpot·Ck2)
        //
        // Gain transfer function:
        //
        //   Av(s) = mu·Ra · (n0 + n1·s + n2·s²)
        //                  / (d0 + d1·s + d2·s²)
        //
        // where:
        //   n0 = 1/Rk
        //   n1 = Rpot·Ck2/Rk + Ck + Ck2
        //   n2 = Ck · Rpot · Ck2
        //
        //   d0 = A/Rk + (mu+1)
        //   d1 = A·n1 + (mu+1)·Rpot·Ck2
        //   d2 = A · n2
        // =================================================================

        double Ck2  = config.Ck2;
        double Rpot = currentPotR;

        double n0 = 1.0 / Rk;
        double n1 = Rpot * Ck2 / Rk + Ck + Ck2;
        double n2 = Ck * Rpot * Ck2;

        double d0 = A / Rk + (mu + 1.0);
        double d1 = A * n1 + (mu + 1.0) * Rpot * Ck2;
        double d2 = A * n2;

        double gain = mu * config.Ra;

        // Bilinear transform: s = c·(z-1)/(z+1), c = 2·fs
        double c  = 2.0 * fs;
        double c2 = c * c;

        // Numerator in z-domain (before normalization)
        double B0 = gain * (n2 * c2 + n1 * c + n0);
        double B1 = gain * (-2.0 * n2 * c2 + 2.0 * n0);
        double B2 = gain * (n2 * c2 - n1 * c + n0);

        // Denominator in z-domain
        double A0 = d2 * c2 + d1 * c + d0;
        double A1 = -2.0 * d2 * c2 + 2.0 * d0;
        double A2 = d2 * c2 - d1 * c + d0;

        // Normalize
        for (auto& f : gainFilters)
            *f.coefficients = juce::dsp::IIR::Coefficients<float> (
                static_cast<float> (B0 / A0),
                static_cast<float> (B1 / A0),
                static_cast<float> (B2 / A0),
                1.0f,
                static_cast<float> (A1 / A0),
                static_cast<float> (A2 / A0));
    }
    else if (Ck > 0.0)
    {
        // =================================================================
        // First-order shelf for single cathode bypass
        //
        //   Av(s) = Av_low · (1 + s/wz) / (1 + s/wp)
        //
        //   wz = 1 / (Rk · Ck)
        //   wp = (A + (mu+1)·Rk) / (A · Rk · Ck)
        // =================================================================

        double B  = (mu + 1.0) * Rk;
        double tau = Rk * Ck;
        double wz  = 1.0 / tau;
        double wp  = (A + B) / (A * tau);
        double K   = mu * config.Ra / (A + B);  // unbypassed (DC) gain

        double c = 2.0 * fs;

        double b0 = K * (c + wz) / (c + wp);
        double b1 = K * (wz - c) / (c + wp);
        double a1 = (wp - c) / (c + wp);

        for (auto& f : gainFilters)
            *f.coefficients = juce::dsp::IIR::Coefficients<float> (
                static_cast<float> (b0), static_cast<float> (b1),
                1.0f, static_cast<float> (a1));
    }
    else
    {
        // No cathode bypass — flat gain
        double gain = mu * config.Ra / (A + (mu + 1.0) * Rk);

        for (auto& f : gainFilters)
            *f.coefficients = juce::dsp::IIR::Coefficients<float> (
                static_cast<float> (gain), 0.0f,
                1.0f, 0.0f);
    }
}

float TriodeStage::waveshape (float x)
{
    // Asymmetric soft clipping modeled after ECC83/12AX7 characteristics.
    //
    // The tube inverts, so in terms of the plate voltage:
    //   Positive swing (toward B+) = tube approaching cutoff → harder clip
    //   Negative swing (toward ground) = plate saturation → softer compression
    //
    // Using tanh with different drive coefficients for each half.

    if (x >= 0.0f)
        return std::tanh (x);                           // cutoff: standard tanh
    else
        return std::tanh (x * 0.85f) / 0.85f;          // saturation: softer, rescaled
}
