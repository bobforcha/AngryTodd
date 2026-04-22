#include "AngryToddLookAndFeel.h"

const juce::Colour AngryToddLookAndFeel::panelColour = juce::Colours::aquamarine;

AngryToddLookAndFeel::AngryToddLookAndFeel()
{
    const auto labelColour = juce::Colour (0xff6b4f3a); // warm coffee-beige, 6.1:1 on aquamarine

    setColour (juce::Slider::textBoxTextColourId,       labelColour);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId,  juce::Colours::black.withAlpha (0.2f));
    setColour (juce::Label::textColourId,               labelColour);
    setColour (juce::Label::textWhenEditingColourId,    labelColour);
}

void AngryToddLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float rotaryStartAngle,
                                             float rotaryEndAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<float> (static_cast<float> (x),
                                          static_cast<float> (y),
                                          static_cast<float> (width),
                                          static_cast<float> (height)).reduced (6.0f);

    auto centre = bounds.getCentre();
    auto diameter = std::min (bounds.getWidth(), bounds.getHeight());
    auto faceBounds = juce::Rectangle<float> (diameter, diameter).withCentre (centre);
    auto radius = diameter * 0.5f;

    // Soft shadow beneath the knob
    {
        juce::Path shadow;
        shadow.addEllipse (faceBounds.translated (0.0f, 2.5f));
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillPath (shadow);
    }

    // White face
    g.setColour (juce::Colours::white);
    g.fillEllipse (faceBounds);

    // Black border
    g.setColour (juce::Colours::black);
    g.drawEllipse (faceBounds, 2.5f);

    // Pointer — black line from centre toward the edge
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float pointerThickness = 3.0f;
    const float pointerLength = radius * 0.82f;
    const float pointerInset = radius * 0.12f;

    juce::Path pointer;
    pointer.addRoundedRectangle (-pointerThickness * 0.5f,
                                 -radius + pointerInset,
                                 pointerThickness,
                                 pointerLength,
                                 pointerThickness * 0.5f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle)
                                .translated (centre.x, centre.y));
    g.setColour (juce::Colours::black);
    g.fillPath (pointer);
}

void AngryToddLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
    const bool on = button.getToggleState();

    // LED above the button
    const float ledDiameter = 10.0f;
    auto ledBounds = juce::Rectangle<float> (ledDiameter, ledDiameter)
                        .withCentre ({ bounds.getCentreX(), bounds.getY() + ledDiameter * 0.5f + 2.0f });

    if (on)
    {
        g.setColour (juce::Colours::red.withAlpha (0.35f));
        g.fillEllipse (ledBounds.expanded (5.0f));
        g.setColour (juce::Colours::red.withAlpha (0.6f));
        g.fillEllipse (ledBounds.expanded (2.0f));
    }

    g.setColour (on ? juce::Colours::red : juce::Colour (0xff3a1010));
    g.fillEllipse (ledBounds);
    g.setColour (juce::Colours::black);
    g.drawEllipse (ledBounds, 1.0f);

    // Circular white button body with black border
    auto bodyArea = bounds.withTop (ledBounds.getBottom() + 6.0f);
    auto diameter = std::min (bodyArea.getWidth(), bodyArea.getHeight());
    auto circleBounds = juce::Rectangle<float> (diameter, diameter)
                           .withCentre ({ bodyArea.getCentreX(), bodyArea.getCentreY() });

    // Subtle shadow beneath
    {
        juce::Path shadow;
        shadow.addEllipse (circleBounds.translated (0.0f, 2.5f));
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillPath (shadow);
    }

    auto faceColour = juce::Colours::white;
    if (shouldDrawButtonAsDown)
        faceColour = faceColour.darker (0.15f);
    else if (shouldDrawButtonAsHighlighted)
        faceColour = faceColour.darker (0.05f);

    g.setColour (faceColour);
    g.fillEllipse (circleBounds);
    g.setColour (juce::Colours::black);
    g.drawEllipse (circleBounds, 2.5f);
}

void AngryToddLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.fillAll (label.findColour (juce::Label::backgroundColourId));

    if (! label.isBeingEdited())
    {
        const auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const auto font = getLabelFont (label);
        const auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());
        const auto justification = label.getJustificationType();
        const auto maxLines = juce::jmax (1, (int) (static_cast<float> (textArea.getHeight()) / font.getHeight()));

        g.setFont (font);

        // Slider value text boxes are Labels whose parent is a Slider — skip the
        // engrave effect on those so only the parameter name labels are shaded.
        const bool isSliderValueBox =
            dynamic_cast<juce::Slider*> (label.getParentComponent()) != nullptr;

        if (! isSliderValueBox)
        {
            // Cream highlight offset down-right for an engraved look
            g.setColour (juce::Colour (0xfff5e8d0).withAlpha (0.55f * alpha));
            g.drawFittedText (label.getText(),
                              textArea.translated (1, 1),
                              justification,
                              maxLines,
                              label.getMinimumHorizontalScale());
        }

        // Main text
        g.setColour (label.findColour (juce::Label::textColourId).withMultipliedAlpha (alpha));
        g.drawFittedText (label.getText(),
                          textArea,
                          justification,
                          maxLines,
                          label.getMinimumHorizontalScale());
    }
    else if (label.isEnabled())
    {
        g.setColour (label.findColour (juce::Label::outlineColourId));
    }

    g.setColour (label.findColour (juce::Label::outlineColourId));
    g.drawRect (label.getLocalBounds());
}

juce::Font AngryToddLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font (juce::FontOptions (13.0f).withStyle ("Bold"));
}
