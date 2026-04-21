//============================================================================= 
// DSPChain.h
//
// Created by Bob Forcha on 6 JAN 2025.
//
// DSP Processing chain for the Angry Todd amp simulator
//
// =============================================================================
//
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class DSPChain
{
public:
    DSPChain();
    ~DSPChain();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processAudioBlock(juce::AudioBuffer<float>& buffer);
    void reset();

private:
    juce::dsp::ProcessorChain<
        juce::dsp::Gain<float>,          // Input Gain
        juce::dsp::IIR::Filter<float>,   // Pre-EQ
        juce::dsp::WaveShaper<float>,    // Distortion
        juce::dsp::IIR::Filter<float>,   // Post-EQ
        juce::dsp::Gain<float>           // Output Gain
    > dspChain;

    void setupFilters(double sampleRate);
};
