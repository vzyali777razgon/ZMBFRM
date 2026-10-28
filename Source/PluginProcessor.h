#pragma once
#include <JuceHeader.h>
#include <array>

class ZMBFRMAudioProcessor final : public juce::AudioProcessor
{
public:
    ZMBFRMAudioProcessor();
    ~ZMBFRMAudioProcessor() override = default;
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "ZMBFRM"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState state;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float getDetectedMidiNote() const noexcept { return detectedMidiNote.load(); }
    float getCorrectionCents() const noexcept { return correctionCents.load(); }
private:
    using Filter = juce::dsp::IIR::Filter<float>;
    Filter lowFilter[2], midFilter[2], highFilter[2];
    juce::dsp::Compressor<float> compressor;
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;
    juce::AudioBuffer<float> delayBuffer;
    juce::AudioBuffer<float> pitchShiftBuffer;
    int delayWritePosition = 0;
    int pitchWritePosition = 0;
    double sampleRateHz = 44100.0;

    static constexpr int pitchWindowSize = 2048;
    std::array<float, pitchWindowSize> pitchHistory{};
    int pitchHistoryWrite = 0;
    int pitchSamplesSinceAnalysis = 0;
    float smoothedPitchRatio = 1.0f;
    float pitchPhase = 0.0f;
    std::atomic<float> detectedMidiNote { -1.0f };
    std::atomic<float> correctionCents { 0.0f };

    float detectPitchMidi() const noexcept;
    int getScaleNote(int midiNote, int key, int scale) const noexcept;
    float pitchShiftSample(int channel, float input, float ratio) noexcept;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZMBFRMAudioProcessor)
};
