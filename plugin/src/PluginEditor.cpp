#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (1400, 500);

    auto setupKnob = [this] (juce::Slider& s, juce::Label& label, const juce::String& text,
                             const juce::String& paramID,
                             std::unique_ptr<SliderAttachment>& attachment)
    {
        s.setSliderStyle (juce::Slider::Rotary);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
        addAndMakeVisible (s);

        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);

        attachment = std::make_unique<SliderAttachment> (processorRef.apvts, paramID, s);
    };

    setupKnob (inputGainSlider,   inputGainLabel,   "Input Gain",   "inputGain",   inputGainAttachment);
    setupKnob (lowContourSlider,  lowContourLabel,  "Low Contour",  "lowContour",  lowContourAttachment);
    setupKnob (boosterFatSlider,  boosterFatLabel,  "Booster Fat",  "boosterFat",  boosterFatAttachment);

    boostSwitch.setButtonText ("Boost");
    addAndMakeVisible (boostSwitch);
    boostAttachment = std::make_unique<ButtonAttachment> (processorRef.apvts, "boost", boostSwitch);

    setupKnob (levelSlider,       levelLabel,       "Level",        "level",       levelAttachment);
    setupKnob (highContourSlider, highContourLabel, "High Contour", "highContour", highContourAttachment);
    setupKnob (trebleSlider,      trebleLabel,      "Treble",       "treble",      trebleAttachment);
    setupKnob (bassSlider,        bassLabel,        "Bass",         "bass",        bassAttachment);
    setupKnob (midSlider,         midLabel,         "Mid",          "mid",         midAttachment);
    setupKnob (limitSlider,       limitLabel,       "Limit",        "limit",       limitAttachment);
    setupKnob (highCutSlider,     highCutLabel,     "High Cut",     "highCut",     highCutAttachment);
    setupKnob (masterSlider,      masterLabel,      "Master",       "master",      masterAttachment);
    setupKnob (boostMasterSlider, boostMasterLabel, "Boost Master", "boostMaster", boostMasterAttachment);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::aquamarine);

    g.setColour (juce::Colours::beige);
    g.setFont (45.0f);
    g.drawFittedText ("Angry Todd", getLocalBounds(), juce::Justification::topLeft, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto knobSize = 120;
    auto labelHeight = 20;
    auto startY = 80;
    auto spacing = 15;

    int x = 30;

    // Input Gain
    inputGainLabel.setBounds (x, startY, knobSize, labelHeight);
    inputGainSlider.setBounds (x, startY + labelHeight, knobSize, knobSize);

    // Low Contour
    x += knobSize + spacing;
    lowContourLabel.setBounds (x, startY, knobSize, labelHeight);
    lowContourSlider.setBounds (x, startY + labelHeight, knobSize, knobSize);

    // Booster Fat
    x += knobSize + spacing;
    boosterFatLabel.setBounds (x, startY, knobSize, labelHeight);
    boosterFatSlider.setBounds (x, startY + labelHeight, knobSize, knobSize);

    // Boost switch (below Booster Fat knob)
    boostSwitch.setBounds (x + 15, startY + labelHeight + knobSize + 5, 90, 25);

    // Level
    x += knobSize + spacing;
    levelLabel.setBounds (x, startY, knobSize, labelHeight);
    levelSlider.setBounds (x, startY + labelHeight, knobSize, knobSize);

    // High Contour
    x += knobSize + spacing;
    highContourLabel.setBounds (x, startY, knobSize, labelHeight);
    highContourSlider.setBounds (x, startY + labelHeight, knobSize, knobSize);

    // Second row: Tone Stack + Master
    int row2Y = startY + labelHeight + knobSize + 40;
    x = 100;

    trebleLabel.setBounds (x, row2Y, knobSize, labelHeight);
    trebleSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);

    x += knobSize + spacing;
    bassLabel.setBounds (x, row2Y, knobSize, labelHeight);
    bassSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);

    x += knobSize + spacing;
    midLabel.setBounds (x, row2Y, knobSize, labelHeight);
    midSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);

    x += knobSize + spacing;
    limitLabel.setBounds (x, row2Y, knobSize, labelHeight);
    limitSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);

    x += knobSize + spacing;
    highCutLabel.setBounds (x, row2Y, knobSize, labelHeight);
    highCutSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);

    x += knobSize + spacing;
    masterLabel.setBounds (x, row2Y, knobSize, labelHeight);
    masterSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);

    x += knobSize + spacing;
    boostMasterLabel.setBounds (x, row2Y, knobSize, labelHeight);
    boostMasterSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);
}
