#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ZMBFRMAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit ZMBFRMAudioProcessorEditor(ZMBFRMAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
private:
    void timerCallback() override;
    void selectTab(int index);
    void updateVisibility();
    ZMBFRMAudioProcessor& processor;
    juce::Label title, subtitle, liveNote;
    juce::TextButton tabs[9];
    juce::Slider sliders[11];
    juce::Label labels[11];
    juce::ToggleButton toggles[7];
    juce::ComboBox keyBox, scaleBox;
    juce::Slider speed, humanize, detune;
    juce::Label tuneLabels[3];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments[11], tuneAttachments[3];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachments[7];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachments[2];
    int selectedTab = 1;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZMBFRMAudioProcessorEditor)
};
