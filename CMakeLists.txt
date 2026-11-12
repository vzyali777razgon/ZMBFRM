cmake_minimum_required(VERSION 3.24)
project(ZMBFRM VERSION 2.0.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

FetchContent_Declare(signalsmith-stretch
    GIT_REPOSITORY https://github.com/Signalsmith-Audio/signalsmith-stretch.git
    GIT_TAG 88061fd4eda56a2353c5d65c74aa3f4ccf76f9da
)
FetchContent_MakeAvailable(signalsmith-stretch)

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
        signalsmith-stretch
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(ZMBFRM PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)