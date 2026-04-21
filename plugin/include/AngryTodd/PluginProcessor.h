#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "InputCoupling.h"
#include "TriodeStage.h"
#include "InterstageFilter.h"
#include "LevelContourFilter.h"
#include "ToneStack.h"
#include "MasterSection.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
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

    // Parameters (directly accessible for now; will migrate to AudioProcessorValueTreeState)
    float inputGain = 1.0f;

    // Set LOW CONTOUR pot (0.0 = full bass boost, 1.0 = no extra bypass)
    void setLowContour (float normalised);

    // Set BOOSTER FAT pot (0.0 = full bass boost, 1.0 = no extra bypass)
    void setBoosterFat (float normalised);

    // Set BOOST switch (true = P2 controls bass, false = C12 shorted to ground = full boost)
    void setBoostSwitch (bool engaged);

    // Set LEVEL (0.0 = full volume, 1.0 = muted)
    void setLevel (float normalised);

    // Set HIGH CONTOUR (0.0 = no treble, 1.0 = full treble)
    void setHighContour (float normalised);

    // Tone stack controls (0–1)
    void setTreble (float normalised);
    void setBass (float normalised);
    void setMid (float normalised);
    void setLimit (float normalised);
    void setBoosterHighCut (float normalised);

    // Master volume controls (0–1)
    void setMaster (float normalised);
    void setBoostMaster (float normalised);

private:
    // BOOST switch and pot state
    bool boostEngaged = false;
    float boosterFatNormalised = 0.5f;
    void updateV2bCathodePot();

    // Preamp signal chain: V1B → V1A → V2B → Level/Contour → V2A
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
