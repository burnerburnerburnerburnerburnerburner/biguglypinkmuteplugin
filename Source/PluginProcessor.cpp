#include "PluginProcessor.h"
#include "PluginEditor.h"

GlassMuteProcessor::GlassMuteProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "GlassMute", createLayout())
{
    onOff = apvts.getRawParameterValue (onOffId);
    onOffParam = apvts.getParameter (onOffId);
}

juce::AudioProcessorValueTreeState::ParameterLayout GlassMuteProcessor::createLayout()
{
    auto attributes = juce::AudioParameterFloatAttributes()
                          .withStringFromValueFunction ([] (float v, int) -> juce::String
                                                        { return v >= 0.5f ? "On" : "Muted"; })
                          .withValueFromStringFunction ([] (const juce::String& text)
                                                        {
                                                            auto t = text.trim().toLowerCase();
                                                            if (t == "on" || t == "unmuted") return 1.0f;
                                                            if (t == "off" || t == "muted" || t == "mute") return 0.0f;
                                                            return juce::jlimit (0.0f, 1.0f, t.getFloatValue() / 100.0f);
                                                        });

    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { onOffId, 1 },
                                                             "On/Off",
                                                             juce::NormalisableRange<float> (0.0f, 1.0f),
                                                             1.0f, // opens unmuted
                                                             attributes));
    return layout;
}

void GlassMuteProcessor::prepareToPlay (double sampleRate, int)
{
    gain.reset (sampleRate, 0.004);
    gain.setCurrentAndTargetValue (isOn() ? 1.0f : 0.0f);
}

bool GlassMuteProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

void GlassMuteProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    gain.setTargetValue (isOn() ? 1.0f : 0.0f);
    gain.applyGain (buffer, buffer.getNumSamples());
}

juce::AudioProcessorEditor* GlassMuteProcessor::createEditor()
{
    return new GlassMuteEditor (*this);
}

void GlassMuteProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void GlassMuteProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GlassMuteProcessor();
}
