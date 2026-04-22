#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "InputCoupling.h"
#include "TriodeStage.h"
#include "InterstageFilter.h"
#include "LevelContourFilter.h"
#include "ToneStack.h"
#include "MasterSection.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor,
                                        private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void syncFromParameters();
    void updateV2bCathodePot();

    float inputGain = 1.0f;
    bool boostEngaged = false;
    float boosterFatNormalised = 0.5f;

    // Preamp signal chain: V1B → V1A → V2B → Level/Contour → V2A → V3A → ToneStack → Master
    InputCoupling inputCoupling;
    TriodeStage v1bStage { AngryToddStages::V1B() };
    InterstageFilter v1bInterstage { AngryToddInterstages::V1B_to_V1A() };
    TriodeStage v1aStage { AngryToddStages::V1A() };
    InterstageFilter v1aInterstage { AngryToddInterstages::V1A_to_V2B() };
    TriodeStage v2bStage { AngryToddStages::V2B() };
    LevelContourFilter levelContour;
    TriodeStage v2aStage { AngryToddStages::V2A() };
    InterstageFilter v2aInterstage { AngryToddInterstages::V2A_to_V3A() };
    TriodeStage v3aStage { AngryToddStages::V3A() };
    // V3B cathode follower: unity gain buffer
    ToneStack toneStack;
    MasterSection masterSection;
    // V4A + V4B cathode followers: unity gain buffers

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
