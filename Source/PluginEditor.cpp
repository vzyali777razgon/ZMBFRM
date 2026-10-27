
#include "PluginEditor.h"

ZMBFRMAudioProcessorEditor::ZMBFRMAudioProcessorEditor(
    ZMBFRMAudioProcessor& p)
    : juce::AudioProcessorEditor(&p),
      processor(p)
{
    setSize(820, 590);

    const auto textColour = juce::Colour(0xffdfe7f1);
    const auto mutedColour = juce::Colour(0xffaab5c6);
    const auto accentColour = juce::Colour(0xffa9d8ff);

    title.setText("ZMBFRM", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(30.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId,
                    juce::Colour(0xfff2f4f8));
    addAndMakeVisible(title);

    subtitle.setText("VOCAL PROCESSOR / ZMB AUDIO",
                     juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(11.0f)));
    subtitle.setColour(juce::Label::textColourId,
                       juce::Colour(0xff8b96a8));
    addAndMakeVisible(subtitle);

    const char* parameterIDs[11] =
    {
        "low", "mid", "high",
        "threshold", "ratio",
        "sat", "chorus", "reverb", "delay",
        "mix", "out"
    };

    const char* sliderNames[11] =
    {
        "LOW", "MID", "HIGH",
        "THRESH", "RATIO",
        "SAT", "CHORUS", "REVERB", "DELAY",
        "WET", "OUTPUT"
    };

    for (int i = 0; i < 11; ++i)
    {
        sliders[i].setSliderStyle(
            juce::Slider::RotaryHorizontalVerticalDrag);

        sliders[i].setTextBoxStyle(
            juce::Slider::TextBoxBelow, false, 62, 18);

        sliders[i].setColour(
            juce::Slider::rotarySliderFillColourId, accentColour);

        sliders[i].setColour(
            juce::Slider::thumbColourId,
            juce::Colour(0xffe8f2ff));

        sliders[i].setColour(
            juce::Slider::textBoxTextColourId, textColour);

        sliders[i].setColour(
            juce::Slider::textBoxBackgroundColourId,
            juce::Colour(0xff202630));

        addAndMakeVisible(sliders[i]);

        labels[i].setText(
            sliderNames[i], juce::dontSendNotification);

        labels[i].setJustificationType(
            juce::Justification::centred);

        labels[i].setColour(
            juce::Label::textColourId, mutedColour);

        addAndMakeVisible(labels[i]);

        sliderAttachments[i] =
            std::make_unique<
                juce::AudioProcessorValueTreeState::SliderAttachment
            >(
                processor.state,
                parameterIDs[i],
                sliders[i]);
    }

    const char* buttonIDs[7] =
    {
        "eqOn", "compOn", "satOn", "chorusOn",
        "reverbOn", "delayOn", "tuneOn"
    };

    const char* buttonNames[7] =
    {
        "ZMBEQ", "ZMBComp", "ZMBSaturation",
        "ZMBChorus", "ZMBReverb", "ZMBDelay",
        "ZMBTune"
    };

    for (int i = 0; i < 7; ++i)
    {
        toggles[i].setButtonText(buttonNames[i]);

        toggles[i].setColour(
            juce::ToggleButton::textColourId, textColour);

        addAndMakeVisible(toggles[i]);

        buttonAttachments[i] =
            std::make_unique<
                juce::AudioProcessorValueTreeState::ButtonAttachment
            >(
                processor.state,
                buttonIDs[i],
                toggles[i]);
    }

    keyBox.addItemList(
        juce::StringArray
        {
            "C", "C#", "D", "D#", "E", "F",
            "F#", "G", "G#", "A", "A#", "B"
        },
        1);

    scaleBox.addItemList(
        juce::StringArray
        {
            "Chromatic", "Major", "Minor"
        },
        1);

    addAndMakeVisible(keyBox);
    addAndMakeVisible(scaleBox);

    comboAttachments[0] =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment
        >(
            processor.state, "key", keyBox);

    comboAttachments[1] =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment
        >(
            processor.state, "scale", scaleBox);

    speed.setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);

    speed.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 55, 18);

    speed.setColour(
        juce::Slider::rotarySliderFillColourId, accentColour);

    speed.setColour(
        juce::Slider::textBoxTextColourId, textColour);

    speed.setColour(
        juce::Slider::textBoxBackgroundColourId,
        juce::Colour(0xff202630));

    humanize.setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);

    humanize.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 55, 18);

    humanize.setColour(
        juce::Slider::rotarySliderFillColourId, accentColour);

    humanize.setColour(
        juce::Slider::textBoxTextColourId, textColour);

    humanize.setColour(
        juce::Slider::textBoxBackgroundColourId,
        juce::Colour(0xff202630));

    addAndMakeVisible(speed);
    addAndMakeVisible(humanize);

    tuneAttachments[0] =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment
        >(
            processor.state, "speed", speed);

    tuneAttachments[1] =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment
        >(
            processor.state, "humanize", humanize);

    auto setupSmallLabel =
        [this, mutedColour](juce::Label& label,
                            const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(
            juce::Label::textColourId, mutedColour);
        addAndMakeVisible(label);
    };

    setupSmallLabel(speedLabel, "SPEED");
    setupSmallLabel(humanizeLabel, "HUMANIZE");
    setupSmallLabel(keyLabel, "KEY");
    setupSmallLabel(scaleLabel, "SCALE");
}

void ZMBFRMAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff11141a));

    g.setColour(juce::Colour(0xff202630));
    g.fillRoundedRectangle(16.0f, 98.0f, 788.0f, 150.0f, 12.0f);
    g.fillRoundedRectangle(16.0f, 260.0f, 788.0f, 204.0f, 12.0f);
    g.fillRoundedRectangle(16.0f, 470.0f, 788.0f, 108.0f, 12.0f);

    g.setColour(juce::Colour(0xffa9d8ff));
    g.fillRoundedRectangle(16.0f, 98.0f, 4.0f, 150.0f, 2.0f);
    g.fillRoundedRectangle(16.0f, 260.0f, 4.0f, 204.0f, 2.0f);
    g.fillRoundedRectangle(16.0f, 470.0f, 4.0f, 108.0f, 2.0f);

    g.setColour(juce::Colour(0xff758194));
    g.setFont(12.0f);

    g.drawText(
        "TONE / DYNAMICS / CHARACTER",
        30, 104, 700, 18,
        juce::Justification::left);

    g.drawText(
        "SPACE / OUTPUT",
        30, 266, 700, 18,
        juce::Justification::left);

    g.drawText(
        "ZMBTUNE / PITCH SETTINGS",
        30, 476, 500, 18,
        juce::Justification::left);

    g.setColour(juce::Colour(0xff697588));
    g.setFont(10.0f);

    g.drawText(
        "DEVELOPMENT BUILD",
        630, 30, 160, 18,
        juce::Justification::right);
}

void ZMBFRMAudioProcessorEditor::resized()
{
    title.setBounds(24, 8, 300, 36);
    subtitle.setBounds(26, 43, 400, 17);

    for (int i = 0; i < 7; ++i)
        toggles[i].setBounds(24 + i * 112, 68, 108, 22);

    for (int i = 0; i < 5; ++i)
    {
        const int x = 26 + i * 150;

        sliders[i].setBounds(x, 130, 116, 92);
        labels[i].setBounds(x, 110, 116, 18);
    }

    for (int i = 5; i < 11; ++i)
    {
        const int x = 24 + (i - 5) * 128;

        sliders[i].setBounds(x, 307, 110, 105);
        labels[i].setBounds(x, 286, 110, 18);
    }

    keyLabel.setBounds(30, 495, 80, 16);
    keyBox.setBounds(30, 512, 90, 24);

    scaleLabel.setBounds(130, 495, 120, 16);
    scaleBox.setBounds(130, 512, 120, 24);

    speedLabel.setBounds(275, 488, 100, 16);
    speed.setBounds(280, 501, 92, 70);

    humanizeLabel.setBounds(390, 488, 110, 16);
    humanize.setBounds(400, 501, 92, 70);
}
