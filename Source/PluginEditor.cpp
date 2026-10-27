#include "PluginEditor.h"
ZMBFRMAudioProcessorEditor::ZMBFRMAudioProcessorEditor(ZMBFRMAudioProcessor& p):AudioProcessorEditor(&p),processor(p){
 setSize(820,570);
 title.setText("ZMBFRM",juce::dontSendNotification);title.setFont(juce::Font(juce::FontOptions(30.f,juce::Font::bold)));title.setColour(juce::Label::textColourId,juce::Colour(0xfff2f4f8));addAndMakeVisible(title);
 subtitle.setText("VOCAL PROCESSOR  /  ZMB AUDIO",juce::dontSendNotification);subtitle.setFont(juce::Font(juce::FontOptions(11.f)));subtitle.setColour(juce::Label::textColourId,juce::Colour(0xff8b96a8));addAndMakeVisible(subtitle);
 const char* ids[]={"low","mid","high","threshold","ratio","sat","chorus","reverb","delay","mix","out"};
 const char* names[]={"LOW","MID","HIGH","THRESH","RATIO","SAT","CHORUS","REVERB","DELAY","WET","OUTPUT"};
 for(int i=0;i<11;++i){sliders[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);sliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow,false,62,18);sliders[i].setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffa9d8ff));sliders[i].setColour(juce::Slider::thumbColourId,juce::Colour(0xffe8f2ff));sliders[i].setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffdfe7f1));addAndMakeVisible(sliders[i]);labels[i].setText(names[i],juce::dontSendNotification);labels[i].setJustificationType(juce::Justification::centred);labels[i].setColour(juce::Label::textColourId,juce::Colour(0xffaab5c6));addAndMakeVisible(labels[i]);sliderAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,ids[i],sliders[i]);}
 const char* bids[]={"eqOn","compOn","satOn","chorusOn","reverbOn","delayOn","tuneOn"};
 const char* bnames[]={"ZMBEQ","ZMBComp","ZMBSaturation","ZMBChorus","ZMBReverb","ZMBDelay","ZMBTune"};
 for(int i=0;i<7;++i){
  toggles[i].setButtonText(bnames[i]);
  toggles[i].setColour(juce::ToggleButton::textColourId,juce::Colour(0xffdfe7f1));
  addAndMakeVisible(toggles[i]);
  buttonAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,bids[i],toggles[i]);
 }
 keyBox.addItemList(juce::StringArray{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},1);scaleBox.addItemList(juce::StringArray{"Chromatic","Major","Minor"},1);addAndMakeVisible(keyBox);addAndMakeVisible(scaleBox);
 comboAttachments[0]=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"key",keyBox);
 comboAttachments[1]=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"scale",scaleBox);
 speed.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);speed.setTextBoxStyle(juce::Slider::TextBoxBelow,false,55,18);humanize.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);humanize.setTextBoxStyle(juce::Slider::TextBoxBelow,false,55,18);addAndMakeVisible(speed);addAndMakeVisible(humanize);
 tuneAttachments[0]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"speed",speed);tuneAttachments[1]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"humanize",humanize);
}
void ZMBFRMAudioProcessorEditor::paint(juce::Graphics&g){g.fillAll(juce::Colour(0xff11141a));g.setColour(juce::Colour(0xff202630));g.fillRoundedRectangle(16,92,788,150,12);g.fillRoundedRectangle(16,254,788,204,12);g.fillRoundedRectangle(16,470,788,86,12);g.setColour(juce::Colour(0xffa9d8ff));g.fillRoundedRectangle(16,92,4,150,2);g.fillRoundedRectangle(16,254,4,204,2);g.fillRoundedRectangle(16,470,4,86,2);g.setColour(juce::Colour(0xff758194));g.setFont(12);g.drawText("TONE  /  DYNAMICS  /  CHARACTER",30,99,700,18,juce::Justification::left);g.drawText("SPACE  /  OUTPUT",30,261,700,18,juce::Justification::left);g.drawText("ZMBTUNE  /  PITCH SETTINGS — CORRECTION DSP NOT IMPLEMENTED YET",30,477,750,18,juce::Justification::left);g.setColour(juce::Colour(0xff697588));g.setFont(10);g.drawText("ZMBFRM  •  DEVELOPMENT BUILD",630,30,160,18,juce::Justification::right);}
void ZMBFRMAudioProcessorEditor::resized(){title.setBounds(24,12,300,44);subtitle.setBounds(26,54,400,20);for(int i=0;i<7;++i)toggles[i].setBounds(24+i*112,69,108,20);
 for(int i=0;i<5;++i){int x=26+i*150;sliders[i].setBounds(x,122,116,92);labels[i].setBounds(x,103,116,18);}
 for(int i=5;i<11;++i){int x=24+(i-5)*128;sliders[i].setBounds(x,302,110,105);labels[i].setBounds(x,281,110,18);}
 keyBox.setBounds(32,510,90,24);scaleBox.setBounds(130,510,120,24);speed.setBounds(280,492,92,58);humanize.setBounds(400,492,92,58);
}
