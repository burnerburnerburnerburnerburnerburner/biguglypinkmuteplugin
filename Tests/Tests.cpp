#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"

#include <juce_audio_utils/juce_audio_utils.h>

namespace
{
    int failures = 0;

    void check (bool ok, const juce::String& what)
    {
        std::cout << (ok ? "PASS  " : "FAIL  ") << what << std::endl;
        if (! ok)
            ++failures;
    }

    // Feeds a few blocks of constant 0.5 through the plugin and returns the last block's peak.
    float runAt (GlassMuteProcessor& p, float value)
    {
        p.onOffParam->setValueNotifyingHost (value);

        juce::AudioBuffer<float> buffer (2, 512);
        juce::MidiBuffer midi;

        for (int block = 0; block < 4; ++block)
        {
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                juce::FloatVectorOperations::fill (buffer.getWritePointer (ch), 0.5f, buffer.getNumSamples());

            p.processBlock (buffer, midi);
        }

        return buffer.getMagnitude (0, buffer.getNumSamples());
    }

    void snapshot (GlassMuteProcessor& p, float value, const juce::File& file)
    {
        p.onOffParam->setValueNotifyingHost (value);
        std::unique_ptr<juce::AudioProcessorEditor> editor (p.createEditor());
        auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true, 2.0f);

        file.deleteFile();
        juce::FileOutputStream out (file);
        juce::PNGImageFormat().writeImageToStream (image, out);
        std::cout << "Wrote " << file.getFullPathName() << std::endl;
    }
}

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI gui;

    auto outDir = argc > 1 ? juce::File (juce::String (argv[1])) : juce::File::getCurrentWorkingDirectory();

    GlassMuteProcessor p;
    p.setPlayConfigDetails (2, 2, 48000.0, 512);
    p.prepareToPlay (48000.0, 512);

    check (juce::approximatelyEqual (p.onOffParam->getValue(), 1.0f), "opens at 100% (unmuted)");
    check (p.onOffParam->getText (1.0f, 32) == "On" && p.onOffParam->getText (0.3f, 32) == "Muted", "host shows On / Muted");

    check (juce::approximatelyEqual (runAt (p, 1.0f), 0.5f), "100% passes audio");
    check (juce::approximatelyEqual (runAt (p, 0.75f), 0.5f), "75% passes audio");
    check (runAt (p, 0.49f) == 0.0f, "49% is silent");
    check (juce::approximatelyEqual (runAt (p, 0.5f), 0.5f), "50% passes audio");
    check (runAt (p, 0.0f) == 0.0f, "0% is silent");

    {
        p.onOffParam->setValueNotifyingHost (0.2f);
        juce::MemoryBlock state;
        p.getStateInformation (state);

        GlassMuteProcessor restored;
        restored.setStateInformation (state.getData(), (int) state.getSize());
        check (juce::approximatelyEqual (restored.onOffParam->getValue(), 0.2f), "state saves and restores");
    }

    snapshot (p, 1.0f, outDir.getChildFile ("ui_on.png"));
    snapshot (p, 0.3f, outDir.getChildFile ("ui_muted.png"));

    std::cout << (failures == 0 ? "All checks passed" : "Some checks FAILED") << std::endl;
    return failures == 0 ? 0 : 1;
}
