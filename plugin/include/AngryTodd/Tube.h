// C++ DSP Object to model tube distortion effect
// Author: Bob Forcha
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

struct TubeParameters
{
    float drive = 1.0f;      // Amount of distortion
    float mix = 0.5f;        // Dry/Wet mix
};

class Tube
{
public:
    // ==========================================================
    // Constructor / Destructor
    Tube();
    ~Tube();

    // Main methods to be called in the prepare and process blocks
    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::dsp::AudioBlock<float>& block, const TubeParameters& params);
    void reset();
private:
    // Internal state
    juce::dsp::ProcessSpec processSpec;
    juce::AudioBuffer<float> tempBuffer;
};
