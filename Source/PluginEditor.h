#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class ZMBFRMAudioProcessorEditor final:public juce::AudioProcessorEditor{
public: explicit ZMBFRMAudioProcessorEditor(ZMBFRMAudioProcessor&);
 void paint(juce::Graphics&) override; void resized() override;
private:
 ZMBFRMAudioProcessor& processor; juce::Label title,subtitle;
 juce::Slider sliders[11]; juce::Label labels[11]; juce::ToggleButton toggles[7];
 juce::ComboBox keyBox,scaleBox; juce::Slider speed,humanize;
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments[11],tuneAttachments[2];
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachments[7];
 std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachments[2];
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZMBFRMAudioProcessorEditor)
};
