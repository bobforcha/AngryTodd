#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&lookAndFeel);
    setSize (1000, 600);

    auto setupKnob = [this] (juce::Slider& s, juce::Label& label, const juce::String& text,
                             const juce::String& paramID,
                             std::unique_ptr<SliderAttachment>& attachment)
    {
        addAndMakeVisible (s);
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);

        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.attachToComponent (&s, false);
        addAndMakeVisible (label);

        attachment = std::make_unique<SliderAttachment> (processorRef.apvts, paramID, s);
    };

    // Top row
    setupKnob (inputGainSlider,   inputGainLabel,   "INPUT GAIN",  "inputGain",   inputGainAttachment);
    setupKnob (lowContourSlider,  lowContourLabel,  "LOW CONT",    "lowContour",  lowContourAttachment);
    setupKnob (boosterFatSlider,  boosterFatLabel,  "BOOSTER FAT", "boosterFat",  boosterFatAttachment);
    setupKnob (levelSlider,       levelLabel,       "LEVEL",       "level",       levelAttachment);
    setupKnob (highContourSlider, highContourLabel, "HIGH CONT",   "highContour", highContourAttachment);
    setupKnob (masterSlider,      masterLabel,      "MASTER",      "master",      masterAttachment);

    // Bottom row
    setupKnob (trebleSlider,      trebleLabel,      "TREBLE",       "treble",      trebleAttachment);
    setupKnob (bassSlider,        bassLabel,        "BASS",         "bass",        bassAttachment);
    setupKnob (midSlider,         midLabel,         "MID",          "mid",         midAttachment);
    setupKnob (limitSlider,       limitLabel,       "LIMIT",        "limit",       limitAttachment);
    setupKnob (highCutSlider,     highCutLabel,     "BOOST HIGH CUT", "highCut",   highCutAttachment);
    setupKnob (boostMasterSlider, boostMasterLabel, "BOOST MASTER", "boostMaster", boostMasterAttachment);

    // Boost button (with LED drawn by LookAndFeel) + label below
    boostSwitch.setButtonText ({});
    addAndMakeVisible (boostSwitch);
    boostAttachment = std::make_unique<ButtonAttachment> (processorRef.apvts, "boost", boostSwitch);

    boostLabel.setText ("BOOST", juce::dontSendNotification);
    boostLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (boostLabel);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (AngryToddLookAndFeel::panelColour);

    auto bounds = getLocalBounds().toFloat();

    // "Angry Todd" signature-style logo, right-aligned at the bottom (echoing
    // the Langner signature position on the real DCP-1 panel).
    auto logoArea = juce::Rectangle<float> (bounds.getRight() - 360.0f,
                                            bounds.getBottom() - 135.0f,
                                            340.0f,
                                            90.0f);

    juce::Font logoFont (juce::FontOptions (64.0f).withStyle ("Bold Italic"));
    g.setFont (logoFont);

    // Drop shadow
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawText ("Angry Todd",
                logoArea.translated (3.0f, 3.0f).toNearestInt(),
                juce::Justification::centredRight);

    // Logo itself
    g.setColour (juce::Colours::white);
    g.drawText ("Angry Todd", logoArea.toNearestInt(), juce::Justification::centredRight);

    // Trademark, plain black, right-aligned under the logo
    auto tmArea = logoArea.withY (logoArea.getBottom() + 4.0f).withHeight (20.0f);
    g.setColour (juce::Colours::black);
    g.setFont (juce::Font (juce::FontOptions (14.0f)));
    g.drawText (juce::String::fromUTF8 ("Bob's Plugin Bargain Bin\xe2\x84\xa2"),
                tmArea.toNearestInt(),
                juce::Justification::centredRight);

    // DCP-1 model tag on the lower-left, same italic style as the logo
    juce::Rectangle<int> modelArea (20, getHeight() - 70, 150, 50);
    juce::Font modelFont (juce::FontOptions (32.0f).withStyle ("Bold Italic"));
    g.setFont (modelFont);

    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawText ("DCP-1", modelArea.translated (2, 2), juce::Justification::centredLeft);

    g.setColour (juce::Colours::white);
    g.drawText ("DCP-1", modelArea, juce::Justification::centredLeft);
}

void AudioPluginAudioProcessorEditor::resized()
{
    constexpr int knobSize   = 110;
    constexpr int labelHeight = 18;
    constexpr int textBoxHeight = 18;
    constexpr int topMargin  = 50;
    constexpr int rowGap     = 40;

    auto bounds = getLocalBounds();
    const int columnCount = 6;
    const int rowWidth = knobSize * columnCount;
    const int spacing  = (bounds.getWidth() - rowWidth - 60) / (columnCount - 1);
    const int startX   = 30;

    auto placeKnob = [&] (juce::Slider& s, int col, int yTop)
    {
        int x = startX + col * (knobSize + spacing);
        // attachToComponent places the label above automatically; leave room for it.
        s.setBounds (x, yTop + labelHeight, knobSize, knobSize);
    };

    const int row1Y = topMargin;
    const int row2Y = row1Y + labelHeight + knobSize + textBoxHeight + rowGap;

    placeKnob (inputGainSlider,   0, row1Y);
    placeKnob (lowContourSlider,  1, row1Y);
    placeKnob (boosterFatSlider,  2, row1Y);
    placeKnob (levelSlider,       3, row1Y);
    placeKnob (highContourSlider, 4, row1Y);
    placeKnob (masterSlider,      5, row1Y);

    placeKnob (trebleSlider,      0, row2Y);
    placeKnob (bassSlider,        1, row2Y);
    placeKnob (midSlider,         2, row2Y);
    placeKnob (limitSlider,       3, row2Y);
    placeKnob (highCutSlider,     4, row2Y);
    placeKnob (boostMasterSlider, 5, row2Y);

    // Boost button on its own row beneath the knobs, centered horizontally
    const int boostW = 80;
    const int boostH = 90;
    const int knobsBottom = row2Y + labelHeight + knobSize + textBoxHeight;
    const int boostX = (bounds.getWidth() - boostW) / 2;
    const int boostY = knobsBottom + 20;
    boostSwitch.setBounds (boostX, boostY, boostW, boostH);
    boostLabel.setBounds (boostX - 20, boostY + boostH + 2, boostW + 40, labelHeight);
}
