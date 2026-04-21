// =============================================================================
// InputCoupling.cpp
//
// Implementation of the input coupling highpass filter.
// =============================================================================

#include "InputCoupling.h"

InputCoupling::InputCoupling() {}
InputCoupling::~InputCoupling() {}

void InputCoupling::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    hpFilter.prepare (spec);
    updateCoefficients();
}

void InputCoupling::process (juce::dsp::AudioBlock<float>& block)
{
    auto context = juce::dsp::ProcessContextReplacing<float> (block);
    hpFilter.process (context);
}

void InputCoupling::reset()
{
    hpFilter.reset();
}

void InputCoupling::updateCoefficients()
{
    // First-order highpass: fc = 1 / (2π * C1 * R1)
    // R1 (1MΩ) is the only discharge path to ground; R2 (62kΩ) is in series
    // with the grid (not shunting to ground) so it doesn't affect the HPF.
    double fc = 1.0 / (2.0 * juce::MathConstants<double>::pi * C1 * R1);
    hpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (sampleRate, static_cast<float> (fc));
}
