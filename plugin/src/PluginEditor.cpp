#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (1200, 500);

    // Input Gain knob
    inputGainSlider.setSliderStyle (juce::Slider::Rotary);
    inputGainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    inputGainSlider.setRange (0.0, 10.0, 0.01);
    inputGainSlider.setValue (1.0);
    inputGainSlider.addListener (this);
    addAndMakeVisible (inputGainSlider);

    inputGainLabel.setText ("Input Gain", juce::dontSendNotification);
    inputGainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (inputGainLabel);

    // Low Contour knob
    lowContourSlider.setSliderStyle (juce::Slider::Rotary);
    lowContourSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    lowContourSlider.setRange (0.0, 1.0, 0.01);
    lowContourSlider.setValue (0.5);
    lowContourSlider.addListener (this);
    addAndMakeVisible (lowContourSlider);

    lowContourLabel.setText ("Low Contour", juce::dontSendNotification);
    lowContourLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (lowContourLabel);

    // Booster Fat knob
    boosterFatSlider.setSliderStyle (juce::Slider::Rotary);
    boosterFatSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    boosterFatSlider.setRange (0.0, 1.0, 0.01);
    boosterFatSlider.setValue (0.5);
    boosterFatSlider.addListener (this);
    addAndMakeVisible (boosterFatSlider);

    boosterFatLabel.setText ("Booster Fat", juce::dontSendNotification);
    boosterFatLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (boosterFatLabel);

    // Boost switch
    boostSwitch.setButtonText ("Boost");
    boostSwitch.addListener (this);
    addAndMakeVisible (boostSwitch);

    // Level knob
    levelSlider.setSliderStyle (juce::Slider::Rotary);
    levelSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    levelSlider.setRange (0.0, 1.0, 0.01);
    levelSlider.setValue (0.3);
    levelSlider.addListener (this);
    addAndMakeVisible (levelSlider);

    levelLabel.setText ("Level", juce::dontSendNotification);
    levelLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (levelLabel);

    // High Contour knob
    highContourSlider.setSliderStyle (juce::Slider::Rotary);
    highContourSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    highContourSlider.setRange (0.0, 1.0, 0.01);
    highContourSlider.setValue (0.5);
    highContourSlider.addListener (this);
    addAndMakeVisible (highContourSlider);

    highContourLabel.setText ("High Contour", juce::dontSendNotification);
    highContourLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (highContourLabel);

    // Treble knob
    trebleSlider.setSliderStyle (juce::Slider::Rotary);
    trebleSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    trebleSlider.setRange (0.0, 1.0, 0.01);
    trebleSlider.setValue (0.5);
    trebleSlider.addListener (this);
    addAndMakeVisible (trebleSlider);

    trebleLabel.setText ("Treble", juce::dontSendNotification);
    trebleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (trebleLabel);

    // Bass knob
    bassSlider.setSliderStyle (juce::Slider::Rotary);
    bassSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    bassSlider.setRange (0.0, 1.0, 0.01);
    bassSlider.setValue (0.5);
    bassSlider.addListener (this);
    addAndMakeVisible (bassSlider);

    bassLabel.setText ("Bass", juce::dontSendNotification);
    bassLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (bassLabel);

    // Mid knob
    midSlider.setSliderStyle (juce::Slider::Rotary);
    midSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    midSlider.setRange (0.0, 1.0, 0.01);
    midSlider.setValue (0.5);
    midSlider.addListener (this);
    addAndMakeVisible (midSlider);

    midLabel.setText ("Mid", juce::dontSendNotification);
    midLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (midLabel);

    // Limit knob
    limitSlider.setSliderStyle (juce::Slider::Rotary);
    limitSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    limitSlider.setRange (0.0, 1.0, 0.01);
    limitSlider.setValue (1.0);
    limitSlider.addListener (this);
    addAndMakeVisible (limitSlider);

    limitLabel.setText ("Limit", juce::dontSendNotification);
    limitLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (limitLabel);

    // Booster High Cut knob
    highCutSlider.setSliderStyle (juce::Slider::Rotary);
    highCutSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    highCutSlider.setRange (0.0, 1.0, 0.01);
    highCutSlider.setValue (0.5);
    highCutSlider.addListener (this);
    addAndMakeVisible (highCutSlider);

    highCutLabel.setText ("High Cut", juce::dontSendNotification);
    highCutLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (highCutLabel);
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

    // Second row: Tone Stack
    int row2Y = startY + labelHeight + knobSize + 40;
    x = 100;

    // Treble
    trebleLabel.setBounds (x, row2Y, knobSize, labelHeight);
    trebleSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);

    // Bass
    x += knobSize + spacing;
    bassLabel.setBounds (x, row2Y, knobSize, labelHeight);
    bassSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);

    // Mid
    x += knobSize + spacing;
    midLabel.setBounds (x, row2Y, knobSize, labelHeight);
    midSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);

    // Limit
    x += knobSize + spacing;
    limitLabel.setBounds (x, row2Y, knobSize, labelHeight);
    limitSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);

    // High Cut
    x += knobSize + spacing;
    highCutLabel.setBounds (x, row2Y, knobSize, labelHeight);
    highCutSlider.setBounds (x, row2Y + labelHeight, knobSize, knobSize);
}

void AudioPluginAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &inputGainSlider)
        processorRef.inputGain = static_cast<float> (inputGainSlider.getValue());
    else if (slider == &lowContourSlider)
        processorRef.setLowContour (static_cast<float> (lowContourSlider.getValue()));
    else if (slider == &boosterFatSlider)
        processorRef.setBoosterFat (static_cast<float> (boosterFatSlider.getValue()));
    else if (slider == &levelSlider)
        processorRef.setLevel (static_cast<float> (levelSlider.getValue()));
    else if (slider == &highContourSlider)
        processorRef.setHighContour (static_cast<float> (highContourSlider.getValue()));
    else if (slider == &trebleSlider)
        processorRef.setTreble (static_cast<float> (trebleSlider.getValue()));
    else if (slider == &bassSlider)
        processorRef.setBass (static_cast<float> (bassSlider.getValue()));
    else if (slider == &midSlider)
        processorRef.setMid (static_cast<float> (midSlider.getValue()));
    else if (slider == &limitSlider)
        processorRef.setLimit (static_cast<float> (limitSlider.getValue()));
    else if (slider == &highCutSlider)
        processorRef.setBoosterHighCut (static_cast<float> (highCutSlider.getValue()));
}

void AudioPluginAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &boostSwitch)
        processorRef.setBoostSwitch (boostSwitch.getToggleState());
}
