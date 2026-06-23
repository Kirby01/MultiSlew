#include "PluginEditor.h"

MultiSlewAudioProcessorEditor::MultiSlewAudioProcessorEditor (MultiSlewAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (520, 250);

    titleLabel.setText ("MultiSlew", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::Font (28.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    setupSlider (inputSlider,  inputLabel,  "Input");
    setupSlider (depthSlider,  depthLabel,  "Depth");
    setupSlider (mixSlider,    mixLabel,    "Mix");
    setupSlider (outputSlider, outputLabel, "Output");

    depthSlider.setNumDecimalPlacesToDisplay (0);
    mixSlider.setNumDecimalPlacesToDisplay (2);

    inputAttachment  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "INPUT",  inputSlider);
    depthAttachment  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "DEPTH",  depthSlider);
    mixAttachment    = std::make_unique<SliderAttachment> (audioProcessor.apvts, "MIX",    mixSlider);
    outputAttachment = std::make_unique<SliderAttachment> (audioProcessor.apvts, "OUTPUT", outputSlider);
}

void MultiSlewAudioProcessorEditor::setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void MultiSlewAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff20242a));
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (14.0f));
    g.drawFittedText ("Harmonious Records", getLocalBounds().removeFromBottom (28), juce::Justification::centred, 1);
}

void MultiSlewAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    titleLabel.setBounds (area.removeFromTop (45));

    area.removeFromTop (10);
    auto knobArea = area.removeFromTop (140);
    const int w = knobArea.getWidth() / 4;

    auto place = [] (juce::Slider& s, juce::Label& l, juce::Rectangle<int> r)
    {
        l.setBounds (r.removeFromTop (24));
        s.setBounds (r);
    };

    place (inputSlider,  inputLabel,  knobArea.removeFromLeft (w).reduced (8));
    place (depthSlider,  depthLabel,  knobArea.removeFromLeft (w).reduced (8));
    place (mixSlider,    mixLabel,    knobArea.removeFromLeft (w).reduced (8));
    place (outputSlider, outputLabel, knobArea.removeFromLeft (w).reduced (8));
}
