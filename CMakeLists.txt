cmake_minimum_required(VERSION 3.22)
project(ZMBFRM VERSION 2.0.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(ZMBFRM
    COMPANY_NAME "ZMB"
    PRODUCT_NAME "ZMBFRM"
    PLUGIN_MANUFACTURER_CODE ZmBf
    PLUGIN_CODE Zm15
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Fx" "Dynamics" "EQ"
)

juce_generate_juce_header(ZMBFRM)

target_sources(ZMBFRM PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.h
)

target_compile_features(ZMBFRM PRIVATE cxx_std_17)

target_link_libraries(ZMBFRM
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(ZMBFRM PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)