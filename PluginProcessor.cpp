#include "PluginProcessor.h"
#include "PluginEditor.h"

MultiSlewAudioProcessor::MultiSlewAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
        #if ! JucePlugin_IsMidiEffect
         #if ! JucePlugin_IsSynth
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
         #endif
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
        #endif
      ),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
}

juce::AudioProcessorValueTreeState::ParameterLayout MultiSlewAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "INPUT", "Input", juce::NormalisableRange<float> (0.0f, 2.0f, 0.01f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        "DEPTH", "Depth", 1, 12, 10));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "MIX", "Mix", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "OUTPUT", "Output", juce::NormalisableRange<float> (0.0f, 2.0f, 0.01f), 1.0f));

    return { params.begin(), params.end() };
}

void MultiSlewAudioProcessor::prepareToPlay (double sampleRate, int)
{
    slewL = slewR = 0.0f;
    dcInL = dcInR = 0.0f;
    dcOutL = dcOutR = 0.0f;

    constexpr float dcFreq = 10.0f;
    dcA = std::exp (-2.0f * juce::MathConstants<float>::pi * dcFreq / (float) sampleRate);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiSlewAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
}
#endif

void MultiSlewAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto* inParam    = apvts.getRawParameterValue ("INPUT");
    auto* depthParam = apvts.getRawParameterValue ("DEPTH");
    auto* mixParam   = apvts.getRawParameterValue ("MIX");
    auto* outParam   = apvts.getRawParameterValue ("OUTPUT");

    const float inGain  = inParam->load();
    const int maxOps    = juce::jlimit (1, 12, (int) std::round (depthParam->load()));
    const float mix     = mixParam->load();
    const float outGain = outParam->load();

    constexpr float smallSlew  = 0.0001f;
    constexpr float errorToOps = 80.0f;
    constexpr float curve      = 1.0f;

    auto* left  = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    for (int n = 0; n < buffer.getNumSamples(); ++n)
    {
        const float dryL = left[n] * inGain;
        const float dryR = right != nullptr ? right[n] * inGain : dryL;

        float err = std::max (std::abs (dryL - slewL), std::abs (dryR - slewR));
        float control = juce::jlimit (0.0f, 1.0f, err * errorToOps);
        control = std::pow (control, curve);

        const int ops = 1 + (int) std::floor (control * (float) (maxOps - 1));

        for (int i = 0; i < ops; ++i)
        {
            float dL = juce::jlimit (-smallSlew, smallSlew, dryL - slewL);
            slewL += dL;

            float dR = juce::jlimit (-smallSlew, smallSlew, dryR - slewR);
            slewR += dR;
        }

        float wetL = slewL;
        float wetR = slewR;

        dcOutL = wetL - dcInL + dcA * dcOutL;
        dcInL = wetL;
        wetL = dcOutL;

        dcOutR = wetR - dcInR + dcA * dcOutR;
        dcInR = wetR;
        wetR = dcOutR;

        left[n] = (dryL * (1.0f - mix) + wetL * mix) * outGain;
        if (right != nullptr)
            right[n] = (dryR * (1.0f - mix) + wetR * mix) * outGain;
    }
}

juce::AudioProcessorEditor* MultiSlewAudioProcessor::createEditor() { return new MultiSlewAudioProcessorEditor (*this); }

void MultiSlewAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MultiSlewAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiSlewAudioProcessor();
}
