#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MultiSlewAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MultiSlewAudioProcessorEditor (MultiSlewAudioProcessor&);
    ~MultiSlewAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    MultiSlewAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::Slider inputSlider, depthSlider, mixSlider, outputSlider;
    juce::Label inputLabel, depthLabel, mixLabel, outputLabel;

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> depthAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;

    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiSlewAudioProcessorEditor)
};
