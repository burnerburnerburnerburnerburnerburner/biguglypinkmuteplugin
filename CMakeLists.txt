cmake_minimum_required(VERSION 3.22)

project(GlassMute VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(JUCE_DIR "$ENV{LOCALAPPDATA}/Temp/gm/JUCE" CACHE PATH "Path to a JUCE 8 checkout")

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(GlassMute
    COMPANY_NAME "Glass Audio"
    BUNDLE_ID com.glassaudio.bigulypinkmute
    PRODUCT_NAME "Big Ugly Pink Mute Plugin"
    PLUGIN_MANUFACTURER_CODE Glsa
    PLUGIN_CODE Gmut
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Tools)

target_sources(GlassMute PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(GlassMute PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(GlassMute
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
