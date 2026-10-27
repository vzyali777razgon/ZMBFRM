
#include "PluginProcessor.h"
#include "PluginEditor.h"

ZMBFRMAudioProcessor::ZMBFRMAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "ZMBFRM_STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
ZMBFRMAudioProcessor::createParameterLayout()
{
    using F = juce::AudioParameterFloat;
    using B = juce::AudioParameterBool;
    using C = juce::AudioParameterChoice;

    juce::AudioProcessorValueTreeState::ParameterLayout p;

    p.add(std::make_unique<B>("eqOn", "ZMBEQ", true));
    p.add(std::make_unique<F>(
        "low", "Low",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    p.add(std::make_unique<F>(
        "mid", "Mid",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    p.add(std::make_unique<F>(
        "high", "High",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

    p.add(std::make_unique<B>("compOn", "ZMBComp", true));
    p.add(std::make_unique<F>(
        "threshold", "Threshold",
        juce::NormalisableRange<float>(-48.0f, 0.0f, 0.1f), -18.0f));
    p.add(std::make_unique<F>(
        "ratio", "Ratio",
        juce::NormalisableRange<float>(1.0f, 12.0f, 0.1f), 3.0f));

    p.add(std::make_unique<B>("satOn", "ZMBSaturation", true));
    p.add(std::make_unique<F>(
        "sat", "Saturation",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.15f));

    p.add(std::make_unique<B>("chorusOn", "ZMBChorus", false));
    p.add(std::make_unique<F>(
        "chorus", "Chorus",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.2f));

    p.add(std::make_unique<B>("reverbOn", "ZMBReverb", true));
    p.add(std::make_unique<F>(
        "reverb", "Reverb",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.15f));

    p.add(std::make_unique<B>("delayOn", "ZMBDelay", false));
    p.add(std::make_unique<F>(
        "delay", "Delay",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.2f));

    p.add(std::make_unique<F>(
        "mix", "Wet",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));
    p.add(std::make_unique<F>(
        "out", "Output",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f));

    // ZMBTune parameters are exposed for UI and automation.
    // Pitch correction DSP is not implemented in this build.
    p.add(std::make_unique<B>("tuneOn", "ZMBTune", false));
    p.add(std::make_unique<C>(
        "key", "Key",
        juce::StringArray{
            "C", "C#", "D", "D#", "E", "F",
            "F#", "G", "G#", "A", "A#", "B"
        },
        0));
    p.add(std::make_unique<C>(
        "scale", "Scale",
        juce::StringArray{"Chromatic", "Major", "Minor"}, 2));
    p.add(std::make_unique<F>(
        "speed", "Speed",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 50.0f));
    p.add(std::make_unique<F>(
        "humanize", "Humanize",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 20.0f));

    return p;
}

void ZMBFRMAudioProcessor::prepareToPlay(double sr, int blockSize)
{
    sampleRateHz = sr;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = static_cast<juce::uint32>(blockSize);
    spec.numChannels =
        static_cast<juce::uint32>(getTotalNumOutputChannels());

    for (int ch = 0; ch < 2; ++ch)
    {
        lowFilter[ch].prepare(spec);
        midFilter[ch].prepare(spec);
        highFilter[ch].prepare(spec);

        lowFilter[ch].reset();
        midFilter[ch].reset();
        highFilter[ch].reset();
    }

    compressor.prepare(spec);
    compressor.setAttack(8.0f);
    compressor.setRelease(100.0f);

    chorus.prepare(spec);
    chorus.setRate(0.25f);
    chorus.setDepth(0.2f);

    // JUCE 8: prepare the DSP reverb using ProcessSpec.
    reverb.prepare(spec);
    reverb.reset();

    delayBuffer.setSize(
        2, static_cast<int>(sr * 2.0) + blockSize + 1);
    delayBuffer.clear();
    delayWritePosition = 0;
}

void ZMBFRMAudioProcessor::processBlock(
    juce::AudioBuffer<float>& b,
    juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int n = b.getNumSamples();
    const int chans = juce::jmin(2, b.getNumChannels());

    // Preserve the original signal for the final wet/dry mix.
    juce::AudioBuffer<float> dry(b);

    auto v = [this](const char* id)
    {
        return state.getRawParameterValue(id)->load();
    };

    // ZMBTune settings are saved, but pitch correction is not implemented.
    if (v("eqOn") > 0.5f)
    {
        auto lc =
            juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                sampleRateHz, 160.0, 0.707,
                juce::Decibels::decibelsToGain(v("low")));

        auto mc =
            juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                sampleRateHz, 1000.0, 0.8,
                juce::Decibels::decibelsToGain(v("mid")));

        auto hc =
            juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sampleRateHz, 6500.0, 0.707,
                juce::Decibels::decibelsToGain(v("high")));

        for (int ch = 0; ch < chans; ++ch)
        {
            lowFilter[ch].coefficients = lc;
            midFilter[ch].coefficients = mc;
            highFilter[ch].coefficients = hc;

            auto block =
                juce::dsp::AudioBlock<float>(b)
                    .getSingleChannelBlock(static_cast<size_t>(ch));

            juce::dsp::ProcessContextReplacing<float> ctx(block);

            lowFilter[ch].process(ctx);
            midFilter[ch].process(ctx);
            highFilter[ch].process(ctx);
        }
    }

    if (v("compOn") > 0.5f)
    {
        compressor.setThreshold(v("threshold"));
        compressor.setRatio(v("ratio"));

        juce::dsp::AudioBlock<float> block(b);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        compressor.process(ctx);
    }

    if (v("satOn") > 0.5f)
    {
        const float drive = 1.0f + v("sat") * 9.0f;
        const float norm = std::tanh(drive);

        for (int ch = 0; ch < chans; ++ch)
        {
            auto* x = b.getWritePointer(ch);

            for (int i = 0; i < n; ++i)
                x[i] = std::tanh(x[i] * drive) / norm;
        }
    }

    if (v("chorusOn") > 0.5f)
    {
        chorus.setDepth(v("chorus"));

        juce::dsp::AudioBlock<float> block(b);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        chorus.process(ctx);
    }

    // Stereo reverb using the JUCE DSP processing interface.
    if (v("reverbOn") > 0.5f && chans == 2)
    {
        juce::dsp::Reverb::Parameters rp;

        rp.roomSize = 0.2f + v("reverb") * 0.7f;
        rp.damping = 0.45f;
        rp.wetLevel = v("reverb") * 0.45f;
        rp.dryLevel = 0.0f;
        rp.width = 0.9f;

        reverb.setParameters(rp);

        juce::dsp::AudioBlock<float> block(b);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        reverb.process(ctx);
    }

    if (v("delayOn") > 0.5f)
    {
        const int cap = delayBuffer.getNumSamples();

        const int dly = juce::jlimit(
            1, cap - 1,
            static_cast<int>(
                sampleRateHz * (0.08 + v("delay") * 0.55)));

        for (int i = 0; i < n; ++i)
        {
            const int readPosition =
                (delayWritePosition - dly + cap) % cap;

            for (int ch = 0; ch < chans; ++ch)
            {
                auto* x = b.getWritePointer(ch);
                auto* d = delayBuffer.getWritePointer(ch);

                const float z = d[readPosition];

                d[delayWritePosition] = x[i] + z * 0.32f;
                x[i] += z * 0.35f;
            }

            delayWritePosition =
                (delayWritePosition + 1) % cap;
        }
    }

    const float mix = v("mix");
    const float gain =
        juce::Decibels::decibelsToGain(v("out"));

    for (int ch = 0; ch < b.getNumChannels(); ++ch)
    {
        auto* x = b.getWritePointer(ch);
        const auto* d = dry.getReadPointer(ch);

        for (int i = 0; i < n; ++i)
        {
            x[i] = (d[i] * (1.0f - mix) + x[i] * mix) * gain;
        }
    }
}

juce::AudioProcessorEditor*
ZMBFRMAudioProcessor::createEditor()
{
    return new ZMBFRMAudioProcessorEditor(*this);
}

void ZMBFRMAudioProcessor::getStateInformation(
    juce::MemoryBlock& d)
{
    auto copy = state.copyState();
    std::unique_ptr<juce::XmlElement> x(copy.createXml());
    copyXmlToBinary(*x, d);
}

void ZMBFRMAudioProcessor::setStateInformation(
    const void* d, int n)
{
    std::unique_ptr<juce::XmlElement> x(getXmlFromBinary(d, n));

    if (x && x->hasTagName(state.state.getType()))
        state.replaceState(juce::ValueTree::fromXml(*x));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZMBFRMAudioProcessor();
}
