#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ZMBFRMLookAndFeel final : public juce::LookAndFeel_V4
{
public:
 void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
 void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
 void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
 void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
};

class ZMBFRMAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit ZMBFRMAudioProcessorEditor(ZMBFRMAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
private:
    void timerCallback() override;
    void selectTab(int index);
    void applyFactoryPreset(int index);
    void saveUserPreset();
    void loadUserPreset();
    void updateVisibility();
    ZMBFRMAudioProcessor& processor;
    ZMBFRMLookAndFeel customLookAndFeel;
    juce::Label title, subtitle, liveNote, inputMeter, outputMeter;
    juce::ComboBox presetBox;
    juce::TextButton savePresetButton, loadPresetButton;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::TextButton tabs[9];
    juce::Slider sliders[45];
    juce::Label labels[45];
    juce::ToggleButton toggles[8];
    juce::ComboBox keyBox, scaleBox;
    juce::Slider speed, humanize, detune;
    juce::Label tuneLabels[3];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments[45], tuneAttachments[3];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachments[8];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachments[2];
    int selectedTab = 1;
    int selectedEQNode = -1;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZMBFRMAudioProcessorEditor)
};
