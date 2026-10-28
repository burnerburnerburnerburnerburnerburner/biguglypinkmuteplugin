#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class GlassLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // 0 = muted look, 1 = on look; eased by the editor so state changes glide.
    float onAmount = 1.0f;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle, juce::Slider&) override;
};

class StatusPill : public juce::Component
{
public:
    StatusPill();

    std::function<void()> onClick;
    bool isOn = true;
    float onAmount = 1.0f;

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }
};

class GlassMuteEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit GlassMuteEditor (GlassMuteProcessor&);
    ~GlassMuteEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void renderBackground (float pixelScale);
    void updatePercentLabel();

    GlassMuteProcessor& audioProcessor;
    GlassLookAndFeel glassLookAndFeel;

    juce::Slider dial;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::Label percentLabel;
    StatusPill pill;

    juce::Image background;
    float backgroundScale = 0.0f;
    float onAmount = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlassMuteEditor)
};
