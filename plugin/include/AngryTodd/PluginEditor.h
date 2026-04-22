#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;
    using ButtonAttachment = APVTS::ButtonAttachment;

    juce::Slider inputGainSlider;
    juce::Label inputGainLabel;
    std::unique_ptr<SliderAttachment> inputGainAttachment;

    juce::Slider lowContourSlider;
    juce::Label lowContourLabel;
    std::unique_ptr<SliderAttachment> lowContourAttachment;

    juce::Slider boosterFatSlider;
    juce::Label boosterFatLabel;
    std::unique_ptr<SliderAttachment> boosterFatAttachment;

    juce::ToggleButton boostSwitch;
    std::unique_ptr<ButtonAttachment> boostAttachment;

    juce::Slider levelSlider;
    juce::Label levelLabel;
    std::unique_ptr<SliderAttachment> levelAttachment;

    juce::Slider highContourSlider;
    juce::Label highContourLabel;
    std::unique_ptr<SliderAttachment> highContourAttachment;

    juce::Slider trebleSlider;
    juce::Label trebleLabel;
    std::unique_ptr<SliderAttachment> trebleAttachment;

    juce::Slider bassSlider;
    juce::Label bassLabel;
    std::unique_ptr<SliderAttachment> bassAttachment;

    juce::Slider midSlider;
    juce::Label midLabel;
    std::unique_ptr<SliderAttachment> midAttachment;

    juce::Slider limitSlider;
    juce::Label limitLabel;
    std::unique_ptr<SliderAttachment> limitAttachment;

    juce::Slider highCutSlider;
    juce::Label highCutLabel;
    std::unique_ptr<SliderAttachment> highCutAttachment;

    juce::Slider masterSlider;
    juce::Label masterLabel;
    std::unique_ptr<SliderAttachment> masterAttachment;

    juce::Slider boostMasterSlider;
    juce::Label boostMasterLabel;
    std::unique_ptr<SliderAttachment> boostMasterAttachment;

    AudioPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
