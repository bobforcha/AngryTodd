// =============================================================================
// InterstageFilter.cpp
//
// Implementation of the interstage coupling/tone-shaping network.
//
// The network is analyzed as a two-node circuit (node_A after coupling cap,
// node_B at the output/grid). KCL at both nodes yields a second-order
// transfer function implemented as a digital biquad via bilinear transform.
//
// =============================================================================

#include "InterstageFilter.h"

InterstageFilter::InterstageFilter (const InterstageConfig& cfg)
    : config (cfg)
{
}

InterstageFilter::~InterstageFilter() {}

void InterstageFilter::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    filters.clear();
    juce::dsp::ProcessSpec monoSpec { spec.sampleRate, spec.maximumBlockSize, 1 };

    for (juce::uint32 ch = 0; ch < spec.numChannels; ++ch)
    {
        filters.emplace_back();
        filters.back().prepare (monoSpec);
    }

    updateCoefficients();
}

void InterstageFilter::process (juce::dsp::AudioBlock<float>& block)
{
    auto numChannels = block.getNumChannels();
    auto numSamples  = block.getNumSamples();

    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* data = block.getChannelPointer (ch);
        auto singleChannelBlock = juce::dsp::AudioBlock<float> (&data, 1, numSamples);
        auto context = juce::dsp::ProcessContextReplacing<float> (singleChannelBlock);
        filters[ch].process (context);
    }
}

void InterstageFilter::reset()
{
    for (auto& f : filters)
        f.reset();
}

void InterstageFilter::updateCoefficients()
{
    double fs = sampleRate;

    double C_in    = config.C_in;
    double C_sh    = config.C_shunt;
    double R_s1    = config.R_s1;
    double R_sh    = config.R_shunt;
    double C_s2    = config.C_s2;
    double R_s2    = config.R_s2;

    bool hasDualPath = (C_s2 > 0.0 && R_s2 > 0.0);

    if (hasDualPath && C_sh > 0.0)
    {
        // =================================================================
        // Full two-path network with shunt cap
        //
        // H(s) = s·C_in·R_sh·(1 + s·C_s2·S)
        //       / (1 + s·(Ct·P + C_s2·S) + s²·C_s2·Q·Ct)
        //
        // Ct = C_in + C_sh
        // P  = R_s1 + R_sh
        // Q  = R_s1·R_sh + R_s1·R_s2 + R_sh·R_s2
        // S  = R_s1 + R_s2
        // =================================================================

        double Ct = C_in + C_sh;
        double P  = R_s1 + R_sh;
        double Q  = R_s1 * R_sh + R_s1 * R_s2 + R_sh * R_s2;
        double S  = R_s1 + R_s2;

        // Analog transfer function coefficients: H(s) = (a2·s² + a1·s) / (b2·s² + b1·s + b0)
        double a2 = C_in * R_sh * C_s2 * S;
        double a1 = C_in * R_sh;
        // a0 = 0 (DC-blocking)

        double b2 = C_s2 * Q * Ct;
        double b1 = Ct * P + C_s2 * S;
        double b0 = 1.0;

        // Bilinear transform: s = c·(z-1)/(z+1), c = 2·fs
        double c  = 2.0 * fs;
        double c2 = c * c;

        // z-domain numerator
        double B0 = a2 * c2 + a1 * c;
        double B1 = -2.0 * a2 * c2;
        double B2 = a2 * c2 - a1 * c;

        // z-domain denominator
        double A0 = b2 * c2 + b1 * c + b0;
        double A1 = -2.0 * b2 * c2 + 2.0 * b0;
        double A2 = b2 * c2 - b1 * c + b0;

        for (auto& f : filters)
            *f.coefficients = juce::dsp::IIR::Coefficients<float> (
                static_cast<float> (B0 / A0),
                static_cast<float> (B1 / A0),
                static_cast<float> (B2 / A0),
                1.0f,
                static_cast<float> (A1 / A0),
                static_cast<float> (A2 / A0));
    }
    else
    {
        // =================================================================
        // Single-path coupling cap + voltage divider, with optional shunt cap.
        //
        // Setting C_s2 = 0 in the full network transfer function yields:
        //
        //   H(s) = s·C_in·R_sh / (1 + s·Ct·(R_s1 + R_sh))
        //
        // where Ct = C_in + C_shunt (or just C_in if no shunt).
        //
        // The shunt cap slightly reduces HF gain by the factor
        // C_in / (C_in + C_shunt) and lowers the cutoff frequency.
        // =================================================================

        double Ct = C_in + C_sh;   // C_sh is 0 if no shunt cap
        double R_total = R_s1 + R_sh;
        double tau = Ct * R_total;

        // HF gain: C_in·R_sh / (Ct · R_total)
        double gain = C_in * R_sh / (Ct * R_total);

        // H(s) = gain · s·tau / (1 + s·tau)
        // Bilinear: s = c(z-1)/(z+1)

        double c = 2.0 * fs;
        double ct_val = c * tau;

        double b0 = gain * ct_val / (1.0 + ct_val);
        double b1 = -b0;
        double a1 = (1.0 - ct_val) / (1.0 + ct_val);

        for (auto& f : filters)
            *f.coefficients = juce::dsp::IIR::Coefficients<float> (
                static_cast<float> (b0), static_cast<float> (b1),
                1.0f, static_cast<float> (a1));
    }
}
