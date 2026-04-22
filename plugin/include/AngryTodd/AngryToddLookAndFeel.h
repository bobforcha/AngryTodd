#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AngryToddLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AngryToddLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

    void drawLabel (juce::Graphics& g, juce::Label& label) override;

    juce::Font getLabelFont (juce::Label& label) override;

    static const juce::Colour panelColour;
};
