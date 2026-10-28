#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class GlassMuteProcessor : public juce::AudioProcessor
{
public:
    static constexpr const char* onOffId = "onoff";

    GlassMuteProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Below 50% the signal is muted, from 50% upwards it passes through.
    bool isOn() const { return onOff->load() >= 0.5f; }

    juce::AudioProcessorValueTreeState apvts;
    juce::RangedAudioParameter* onOffParam = nullptr;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    std::atomic<float>* onOff = nullptr;

    // A few milliseconds of fade so switching never clicks.
    juce::SmoothedValue<float> gain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlassMuteProcessor)
};
