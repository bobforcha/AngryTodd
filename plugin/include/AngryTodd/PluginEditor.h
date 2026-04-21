#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              public juce::Slider::Listener,
                                              public juce::Button::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged (juce::Slider* slider) override;
    void buttonClicked (juce::Button* button) override;

private:
    juce::Slider inputGainSlider;
    juce::Label inputGainLabel;

    juce::Slider lowContourSlider;
    juce::Label lowContourLabel;

    juce::Slider boosterFatSlider;
    juce::Label boosterFatLabel;

    juce::ToggleButton boostSwitch;

    juce::Slider levelSlider;
    juce::Label levelLabel;

    juce::Slider highContourSlider;
    juce::Label highContourLabel;

    juce::Slider trebleSlider;
    juce::Label trebleLabel;

    juce::Slider bassSlider;
    juce::Label bassLabel;

    juce::Slider midSlider;
    juce::Label midLabel;

    juce::Slider limitSlider;
    juce::Label limitLabel;

    juce::Slider highCutSlider;
    juce::Label highCutLabel;

    juce::Slider masterSlider;
    juce::Label masterLabel;

    juce::Slider boostMasterSlider;
    juce::Label boostMasterLabel;

    AudioPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
