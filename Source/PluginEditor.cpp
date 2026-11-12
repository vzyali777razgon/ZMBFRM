#include "PluginEditor.h"
#include <utility>

namespace {
const juce::Colour bg(0xff08090d), panel(0xff11131a), panel2(0xff191c25), line(0xff303541), text(0xfff4f4f6), muted(0xff9297a3), accent(0xffff3048);
const juce::StringArray tabNames { "ZMBEQ", "ZMBTUNE", "ZMBCOMP", "ZMBSATURATION", "ZMBDEESSER", "ZMBREVERB", "ZMBDELAY", "ZMBCHORUS", "ZMBMASTER" };
}

ZMBFRMAudioProcessorEditor::ZMBFRMAudioProcessorEditor(ZMBFRMAudioProcessor& p) : AudioProcessorEditor(&p), processor(p)
{
 setSize(1280, 820); setResizable(true, true); setResizeLimits(1050, 720, 1680, 1120);
 title.setText("ZMBFRM", juce::dontSendNotification); title.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold))); title.setColour(juce::Label::textColourId, text); addAndMakeVisible(title);
 subtitle.setText("VERSION 2.0   /   VOCAL PRODUCTION SYSTEM   /   PRECISION PITCH ENGINE", juce::dontSendNotification); subtitle.setFont(juce::Font(juce::FontOptions(10.0f))); subtitle.setColour(juce::Label::textColourId, muted); addAndMakeVisible(subtitle);
 liveNote.setText("WAITING FOR VOICE", juce::dontSendNotification); liveNote.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold))); liveNote.setColour(juce::Label::textColourId, accent); liveNote.setJustificationType(juce::Justification::centredRight); addAndMakeVisible(liveNote);
 presetBox.addItemList(juce::StringArray{"Init / Neutral","Natural Vocal","Modern Pop","Rap Vocal","Hard Tune","Wide Vocal"},1);
 presetBox.addItem("Custom / Loaded",7);
 presetBox.setSelectedId(1,juce::dontSendNotification);
 presetBox.setTooltip("Load a factory starting point. Every control remains editable.");
 presetBox.setColour(juce::ComboBox::backgroundColourId,panel2);
 presetBox.setColour(juce::ComboBox::textColourId,text);
 presetBox.setColour(juce::ComboBox::outlineColourId,line);
 presetBox.setColour(juce::ComboBox::arrowColourId,accent);
 presetBox.onChange=[this]{const int id=presetBox.getSelectedId();if(id>=1&&id<=6)applyFactoryPreset(id-1);};
 addAndMakeVisible(presetBox);
 savePresetButton.setButtonText("SAVE");loadPresetButton.setButtonText("LOAD");
 for(auto* button:{&savePresetButton,&loadPresetButton}){
  button->setColour(juce::TextButton::buttonColourId,panel2);
  button->setColour(juce::TextButton::textColourOffId,accent);
  button->setColour(juce::TextButton::textColourOnId,text);
  addAndMakeVisible(*button);
 }
 savePresetButton.setTooltip("Save the current complete parameter state as a ZMBFRM preset.");
 loadPresetButton.setTooltip("Load a previously saved ZMBFRM preset.");
 savePresetButton.onClick=[this]{saveUserPreset();};
 loadPresetButton.onClick=[this]{loadUserPreset();};
 inputMeter.setText("IN  -inf dB", juce::dontSendNotification);
 outputMeter.setText("OUT  -inf dB", juce::dontSendNotification);
 for (auto* meter : { &inputMeter, &outputMeter }) {
  meter->setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
  meter->setColour(juce::Label::textColourId, accent);
  meter->setJustificationType(juce::Justification::centredRight);
  addAndMakeVisible(*meter);
 }
 for (int i=0;i<9;++i) { tabs[i].setButtonText(tabNames[i]); tabs[i].setClickingTogglesState(false); tabs[i].setColour(juce::TextButton::buttonColourId, panel2); tabs[i].setColour(juce::TextButton::textColourOffId, muted); tabs[i].setColour(juce::TextButton::textColourOnId, accent); tabs[i].onClick=[this,i]{selectTab(i);}; addAndMakeVisible(tabs[i]); }
 const char* ids[]={"low","mid","high","threshold","ratio","sat","deessAmount","deessFreq","reverb","delay","chorus","mix","out","lowFreq","midFreq","midQ","highFreq","compAttack","compRelease","satMix","reverbDamping","delayFeedback","delayMix","chorusRate","deessThreshold","band4Gain","band4Freq","band4Q","band5Gain","band5Freq","band5Q","band6Gain","band6Freq","band6Q","band7Gain","band7Freq","band7Q","band8Gain","band8Freq","band8Q","chorusMix","chorusFeedback","chorusDelay","reverbMix","reverbWidth"};
 const char* names[]={"LOW GAIN","MID GAIN","HIGH GAIN","THRESHOLD","RATIO","DRIVE","DE-ESS AMOUNT","DE-ESS FREQ","ROOM","DELAY TIME","CHORUS DEPTH","WET / DRY","OUTPUT","LOW FREQ","MID FREQ","MID Q","HIGH FREQ","ATTACK","RELEASE","SAT MIX","REVERB DAMPING","DELAY FEEDBACK","DELAY MIX","CHORUS RATE","DE-ESS THRESHOLD","BAND 4 GAIN","BAND 4 FREQ","BAND 4 Q","BAND 5 GAIN","BAND 5 FREQ","BAND 5 Q","BAND 6 GAIN","BAND 6 FREQ","BAND 6 Q","BAND 7 GAIN","BAND 7 FREQ","BAND 7 Q","BAND 8 GAIN","BAND 8 FREQ","BAND 8 Q","CHORUS MIX","CHORUS FEEDBACK","CHORUS DELAY","REVERB MIX","REVERB WIDTH"};
 for(int i=0;i<45;++i){ sliders[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); sliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow,false,68,20); sliders[i].setColour(juce::Slider::rotarySliderFillColourId,accent); sliders[i].setColour(juce::Slider::rotarySliderOutlineColourId,line); sliders[i].setColour(juce::Slider::thumbColourId,text); sliders[i].setColour(juce::Slider::textBoxTextColourId,text); sliders[i].setColour(juce::Slider::textBoxBackgroundColourId,panel2); addAndMakeVisible(sliders[i]); sliders[i].setTooltip(juce::String(names[i]) + " — double-click to reset"); labels[i].setText(names[i],juce::dontSendNotification); labels[i].setJustificationType(juce::Justification::centred); labels[i].setColour(juce::Label::textColourId,muted); addAndMakeVisible(labels[i]); sliderAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,ids[i],sliders[i]); }
 const char* bids[]={"eqOn","compOn","satOn","deesserOn","reverbOn","delayOn","chorusOn","tuneOn"};
 const char* bnames[]={"EQ ACTIVE","COMP ACTIVE","SATURATION","DE-ESSER ACTIVE","REVERB ACTIVE","DELAY ACTIVE","CHORUS ACTIVE","ZMBTUNE ACTIVE"};
 for(int i=0;i<8;++i){toggles[i].setButtonText(bnames[i]);toggles[i].setTooltip("Enable or bypass this processing module");toggles[i].setColour(juce::ToggleButton::textColourId,text);toggles[i].setColour(juce::ToggleButton::tickColourId,accent);addAndMakeVisible(toggles[i]);buttonAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,bids[i],toggles[i]);}
 keyBox.addItemList(juce::StringArray{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},1);
 scaleBox.addItemList(juce::StringArray{"Chromatic","Major","Natural Minor","Harmonic Minor","Melodic Minor","Dorian","Phrygian","Lydian","Mixolydian","Locrian","Major Pentatonic","Minor Pentatonic","Blues"},1);
 for(auto* box:{&keyBox,&scaleBox}) { box->setTooltip("Select the musical key or scale used by pitch correction"); box->setColour(juce::ComboBox::backgroundColourId,panel2);box->setColour(juce::ComboBox::textColourId,text);box->setColour(juce::ComboBox::outlineColourId,line);box->setColour(juce::ComboBox::arrowColourId,accent);addAndMakeVisible(*box); }
 comboAttachments[0]=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"key",keyBox);comboAttachments[1]=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"scale",scaleBox);
 juce::Slider* tuneSliders[]={&speed,&humanize,&detune};const char* tuneIds[]={"speed","humanize","detune"};const char* tuneNames[]={"RETUNE SPEED","HUMANIZE","DETUNE / CENTS"};
 for(int i=0;i<3;++i){tuneSliders[i]->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);tuneSliders[i]->setTextBoxStyle(juce::Slider::TextBoxBelow,false,74,20);tuneSliders[i]->setColour(juce::Slider::rotarySliderFillColourId,accent);tuneSliders[i]->setColour(juce::Slider::rotarySliderOutlineColourId,line);tuneSliders[i]->setColour(juce::Slider::thumbColourId,text);tuneSliders[i]->setColour(juce::Slider::textBoxTextColourId,text);tuneSliders[i]->setColour(juce::Slider::textBoxBackgroundColourId,panel2);addAndMakeVisible(*tuneSliders[i]);tuneLabels[i].setText(tuneNames[i],juce::dontSendNotification);tuneLabels[i].setJustificationType(juce::Justification::centred);tuneLabels[i].setColour(juce::Label::textColourId,muted);addAndMakeVisible(tuneLabels[i]);tuneAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,tuneIds[i],*tuneSliders[i]);}
 selectTab(1); resized(); startTimerHz(15);
}


void ZMBFRMAudioProcessorEditor::applyFactoryPreset(int index)
{
 auto set=[this](const char* id,float value){
  if(auto* parameter=processor.state.getParameter(id)){
   parameter->beginChangeGesture();
   parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
   parameter->endChangeGesture();
  }
 };
 // Start each preset from a known baseline so no hidden parameter is
 // accidentally inherited from the previously selected sound.
 const std::pair<const char*,float> defaults[]={
  {"eqOn",1.0f},{"low",0.0f},{"mid",0.0f},{"high",0.0f},{"lowFreq",160.0f},{"midFreq",1000.0f},{"midQ",0.8f},{"highFreq",6500.0f},
  {"band4Gain",0.0f},{"band4Freq",250.0f},{"band4Q",1.0f},{"band5Gain",0.0f},{"band5Freq",500.0f},{"band5Q",1.0f},
  {"band6Gain",0.0f},{"band6Freq",1000.0f},{"band6Q",1.0f},{"band7Gain",0.0f},{"band7Freq",2000.0f},{"band7Q",1.0f},
  {"band8Gain",0.0f},{"band8Freq",4000.0f},{"band8Q",1.0f},
  {"compOn",1.0f},{"threshold",-18.0f},{"ratio",3.0f},{"compAttack",8.0f},{"compRelease",100.0f},
  {"satOn",1.0f},{"sat",0.15f},{"satMix",1.0f},{"deesserOn",1.0f},{"deessAmount",0.35f},{"deessFreq",6500.0f},{"deessThreshold",0.035f},
  {"reverbOn",1.0f},{"reverb",0.15f},{"reverbDamping",0.45f},{"reverbMix",0.15f},{"reverbWidth",0.9f},{"delayOn",0.0f},{"delay",0.2f},{"delayFeedback",0.32f},{"delayMix",0.35f},
  {"chorusOn",0.0f},{"chorus",0.2f},{"chorusRate",0.25f},{"chorusMix",0.5f},{"chorusFeedback",0.0f},{"chorusDelay",7.0f},{"mix",1.0f},{"out",0.0f},
  {"tuneOn",1.0f},{"key",0.0f},{"scale",2.0f},{"speed",50.0f},{"humanize",20.0f},{"detune",0.0f}
 };
 for(const auto& item:defaults) set(item.first,item.second);

 switch(index){
  case 1: // Natural Vocal
   set("low",-2);set("mid",0.5f);set("high",1.5f);set("lowFreq",120);set("midFreq",2800);set("midQ",0.9f);set("highFreq",9000);
   set("threshold",-24);set("ratio",2.5f);set("compAttack",18);set("compRelease",140);
   set("sat",0.08f);set("satMix",0.25f);set("deessAmount",0.3f);set("reverb",0.08f);set("reverbMix",0.08f);
   set("scale",1);set("speed",30);set("humanize",75);
   break;
  case 2: // Modern Pop
   set("low",-1.5f);set("mid",-1);set("high",2.5f);set("midFreq",3200);set("midQ",1.1f);set("highFreq",8500);
   set("band6Gain",1);set("band6Freq",2800);set("band7Gain",1);set("band7Freq",5200);set("band8Gain",1.5f);set("band8Freq",11000);
   set("threshold",-22);set("ratio",3.5f);set("compAttack",8);set("compRelease",100);
   set("sat",0.18f);set("satMix",0.3f);set("deessAmount",0.45f);set("reverb",0.18f);set("reverbMix",0.18f);
   set("delayOn",1);set("delay",0.22f);set("delayMix",0.14f);set("delayFeedback",0.2f);
   set("scale",1);set("speed",65);set("humanize",35);
   break;
  case 3: // Rap Vocal
   set("low",-3);set("mid",-2);set("high",2);set("lowFreq",140);set("midFreq",450);set("midQ",1.2f);set("highFreq",7500);
   set("threshold",-26);set("ratio",4.5f);set("compAttack",4);set("compRelease",85);
   set("sat",0.28f);set("satMix",0.4f);set("deessAmount",0.32f);set("reverb",0.05f);set("reverbMix",0.05f);
   set("delayOn",1);set("delay",0.16f);set("delayMix",0.2f);set("delayFeedback",0.25f);
   set("scale",2);set("speed",72);set("humanize",22);
   break;
  case 4: // Hard Tune
   set("tuneOn",1);set("scale",0);set("speed",100);set("humanize",0);set("detune",0);
   set("reverb",0.08f);set("reverbMix",0.08f);set("delayOn",0);set("chorusOn",0);
   break;
  case 5: // Wide Vocal
   set("low",-1);set("high",1.5f);set("threshold",-20);set("ratio",2.8f);
   set("sat",0.12f);set("satMix",0.2f);set("deessAmount",0.4f);
   set("reverb",0.24f);set("reverbMix",0.24f);set("reverbDamping",0.55f);
   set("delayOn",1);set("delay",0.28f);set("delayFeedback",0.28f);set("delayMix",0.24f);
   set("chorusOn",1);set("chorus",0.32f);set("chorusRate",0.22f);set("chorusMix",0.35f);set("chorusFeedback",0.1f);set("chorusDelay",9.0f);
   set("speed",35);set("humanize",65);
   break;
  default: break;
 }
}


void ZMBFRMAudioProcessorEditor::saveUserPreset()
{
 if(fileChooser)return;
 auto folder=juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("ZMBFRM");
 folder.createDirectory();
 fileChooser=std::make_unique<juce::FileChooser>("Save ZMBFRM preset",folder,"*.zmbpreset");
 juce::Component::SafePointer<ZMBFRMAudioProcessorEditor> safeThis(this);
 fileChooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles,
  [safeThis](const juce::FileChooser& chooser){
   if(safeThis==nullptr)return;
   auto file=chooser.getResult();
   if(file!=juce::File{}){
    if(!file.hasFileExtension("zmbpreset"))file=file.withFileExtension(".zmbpreset");
    if(auto xml=safeThis->processor.state.copyState().createXml()){
     if(!xml->writeTo(file))
      juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"ZMBFRM","Could not write the preset file.");
     else safeThis->presetBox.setSelectedId(7,juce::dontSendNotification);
    }
   }
   safeThis->fileChooser.reset();
  });
}

void ZMBFRMAudioProcessorEditor::loadUserPreset()
{
 if(fileChooser)return;
 auto folder=juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("ZMBFRM");
 folder.createDirectory();
 fileChooser=std::make_unique<juce::FileChooser>("Load ZMBFRM preset",folder,"*.zmbpreset");
 juce::Component::SafePointer<ZMBFRMAudioProcessorEditor> safeThis(this);
 fileChooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
  [safeThis](const juce::FileChooser& chooser){
   if(safeThis==nullptr)return;
   const auto file=chooser.getResult();
   if(file!=juce::File{}){
    if(auto xml=juce::XmlDocument::parse(file)){
     if(xml->hasTagName(safeThis->processor.state.state.getType())){
      const auto restored=juce::ValueTree::fromXml(*xml);
      if(restored.isValid()){
       safeThis->processor.state.replaceState(restored);
       safeThis->presetBox.setSelectedId(7,juce::dontSendNotification);
      }
     }else juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"ZMBFRM","This file is not a valid ZMBFRM preset.");
    }else juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"ZMBFRM","Could not read the preset file.");
   }
   safeThis->fileChooser.reset();
  });
}

void ZMBFRMAudioProcessorEditor::selectTab(int index){selectedEQNode=-1;selectedTab=juce::jlimit(0,8,index);for(int i=0;i<9;++i){tabs[i].setToggleState(i==selectedTab,juce::dontSendNotification);tabs[i].setColour(juce::TextButton::buttonColourId,i==selectedTab?juce::Colour(0xff421923):panel2);tabs[i].setColour(juce::TextButton::textColourOffId,i==selectedTab?accent:muted);}updateVisibility();resized();repaint();}
void ZMBFRMAudioProcessorEditor::updateVisibility(){
 const bool tune=selectedTab==1;
 for(auto& s:sliders)s.setVisible(false); for(auto& l:labels)l.setVisible(false); for(auto& t:toggles)t.setVisible(false);
 keyBox.setVisible(tune); scaleBox.setVisible(tune); speed.setVisible(tune); humanize.setVisible(tune); detune.setVisible(tune); for(auto& l:tuneLabels)l.setVisible(tune); liveNote.setVisible(true);
 auto ss=[this](int i){sliders[i].setVisible(true);labels[i].setVisible(true);};
 auto tt=[this](int i){toggles[i].setVisible(true);};
 switch(selectedTab){
 case 0:
   for(int i : {0,1,2,13,14,15,16}) ss(i);
   for(int i=25;i<40;++i) ss(i);
   tt(0);break;
 case 1: tt(7); break;
 case 2: ss(3);ss(4);ss(17);ss(18);tt(1);break;
 case 3: ss(5);ss(19);tt(2);break;
 case 4: ss(6);ss(7);ss(24);tt(3);break;
 case 5: ss(8);ss(20);ss(43);ss(44);tt(4);break;
 case 6: ss(9);ss(21);ss(22);tt(5);break;
 case 7: ss(10);ss(23);ss(40);ss(41);ss(42);tt(6);break;
 case 8: ss(11);ss(12);break;
 default: break;
 }
}

void ZMBFRMAudioProcessorEditor::timerCallback()
{
 const float note=processor.getDetectedMidiNote();
 if(selectedTab==1){
  if(note<0.0f) liveNote.setText("WAITING FOR VOICE",juce::dontSendNotification);
  else {
   const juce::StringArray notes{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
   const int midi=(int)std::lround(note);
   liveNote.setText(notes[midi%12]+juce::String(midi/12-1)+"   "+juce::String(processor.getCorrectionCents(),1)+" cents",juce::dontSendNotification);
  }
 } else liveNote.setText("REAL-TIME AUDIO",juce::dontSendNotification);
 const auto dbText=[](float peak){ return peak<0.000001f ? juce::String("-inf") : juce::String(juce::Decibels::gainToDecibels(peak),1); };
 inputMeter.setText("IN  "+dbText(processor.getInputPeak())+" dB",juce::dontSendNotification);
 outputMeter.setText("OUT  "+dbText(processor.getOutputPeak())+" dB",juce::dontSendNotification);
 inputMeter.setColour(juce::Label::textColourId,processor.getInputPeak()>=1.0f?juce::Colours::red:accent);
 outputMeter.setColour(juce::Label::textColourId,processor.getOutputPeak()>=1.0f?juce::Colours::red:accent);
 const char* moduleNames[]={"EQ","COMPRESSOR","SATURATION","DE-ESSER","REVERB","DELAY","CHORUS","AUTO-TUNE"};
 for(int i=0;i<8;++i){
  const bool enabled=toggles[i].getToggleState();
  const juce::String status=juce::String(moduleNames[i])+(enabled?"  /  ON":"  /  BYPASSED");
  if(toggles[i].getButtonText()!=status)toggles[i].setButtonText(status);
  toggles[i].setColour(juce::ToggleButton::textColourId,enabled?accent:text);
  toggles[i].setColour(juce::ToggleButton::tickColourId,enabled?accent:muted);
 }
 repaint();
}

void ZMBFRMAudioProcessorEditor::paint(juce::Graphics& g)
{
 g.fillAll(bg);g.setColour(panel);g.fillRoundedRectangle(18,84,(float)getWidth()-36,44,10);g.setColour(panel);g.fillRoundedRectangle(18,142,(float)getWidth()-36,(float)getHeight()-160,14);
 g.setColour(line);g.drawRoundedRectangle(18,84,(float)getWidth()-36,44,10,1);g.drawRoundedRectangle(18,142,(float)getWidth()-36,(float)getHeight()-160,14,1);
 g.setColour(muted);g.setFont(9.0f);g.drawText("ZMB AUDIO  /  VST3",getWidth()-180,69,150,12,juce::Justification::centredRight);
 // Compact input/output level bars share the same peak telemetry as the labels.
 const float meterValues[]={processor.getInputPeak(),processor.getOutputPeak()};
 const int meterX[]={getWidth()-300,getWidth()-170};
 for(int i=0;i<2;++i){
  const float db=meterValues[i]>0.000001f?juce::Decibels::gainToDecibels(meterValues[i]):-60.0f;
  const float level=juce::jlimit(0.0f,1.0f,(db+60.0f)/60.0f);
  g.setColour(panel2);g.fillRoundedRectangle((float)meterX[i],61.0f,112.0f,5.0f,2.5f);
  g.setColour(meterValues[i]>=1.0f?juce::Colours::red:accent);
  g.fillRoundedRectangle((float)meterX[i],61.0f,112.0f*level,5.0f,2.5f);
 }
 if(selectedTab==1){
  auto area=juce::Rectangle<float>(36,160,(float)getWidth()-72,270);g.setColour(panel2);g.fillRoundedRectangle(area,10);g.setColour(line);g.drawRoundedRectangle(area,10,1);
  g.setColour(muted);g.setFont(10.0f);g.drawText("PITCH TRACKING  /  INPUT ANALYSIS",area.toNearestInt().reduced(16).removeFromTop(20),juce::Justification::left);
  // Subtle grid and pitch guide lines.
  for(int i=0;i<9;++i){float y=area.getY()+38.0f+i*(area.getHeight()-54.0f)/8.0f;g.setColour(i==4?juce::Colour(0xff344b58):line);g.drawHorizontalLine((int)y,area.getX()+14,area.getRight()-14);}
  for(int i=0;i<13;++i){float x=area.getX()+14.0f+i*(area.getWidth()-28.0f)/12.0f;g.setColour(line);g.drawVerticalLine((int)x,area.getY()+30,area.getBottom()-14);}
  const int pitchGuideNotes[]={48,60,72,84};
  const char* pitchGuideNames[]={"C3","C4","C5","C6"};
  for(int i=0;i<4;++i){
   const float y=area.getY()+38.0f+(area.getHeight()-70.0f)
      -(float)(pitchGuideNotes[i]-33)*(area.getHeight()-70.0f)/51.0f;
   g.setColour(muted);g.setFont(9.0f);g.drawText(pitchGuideNames[i],(int)area.getX()+4,(int)y-7,24,14,juce::Justification::left);
  }
  g.setColour(accent);g.setFont(juce::Font(juce::FontOptions(12.0f,juce::Font::bold)));g.drawText("ZMBTUNE",area.getX()+20,area.getY()+35,110,20,juce::Justification::left);
  // Draw the actual recent pitch history, leaving gaps where the input is unvoiced.
  juce::Path pitchPath;bool traceStarted=false;
  const int traceCount=processor.getPitchTraceCount();
  const float traceLeft=area.getX()+16.0f, traceWidth=area.getWidth()-32.0f;
  const float traceTop=area.getY()+38.0f, traceHeight=area.getHeight()-70.0f;
  float lastTraceX=traceLeft,lastTraceY=area.getCentreY();
  for(int i=0;i<traceCount;++i){
   const float pitch=processor.getPitchTracePoint(i);
   if(pitch<0.0f){traceStarted=false;continue;}
   const float x=traceLeft+(traceCount>1?(float)i/(float)(traceCount-1):1.0f)*traceWidth;
   const float y=traceTop+traceHeight-(juce::jlimit(33.0f,84.0f,pitch)-33.0f)*(traceHeight/51.0f);
   if(!traceStarted){pitchPath.startNewSubPath(x,y);traceStarted=true;}else pitchPath.lineTo(x,y);
   lastTraceX=x;lastTraceY=y;
  }
  g.setColour(juce::Colour(0x55ff3048));g.strokePath(pitchPath,juce::PathStrokeType(6.0f));
  g.setColour(accent);g.strokePath(pitchPath,juce::PathStrokeType(2.0f));
  if(processor.getDetectedMidiNote()>=0.0f){g.setColour(panel);g.fillEllipse(lastTraceX-6.0f,lastTraceY-6.0f,12.0f,12.0f);g.setColour(accent);g.fillEllipse(lastTraceX-3.5f,lastTraceY-3.5f,7.0f,7.0f);}
  g.setColour(muted);g.setFont(10.0f);g.drawText("LOWER PITCH",(int)area.getX()+16,(int)area.getBottom()-24,100,14,juce::Justification::left);g.drawText("HIGHER PITCH",(int)area.getRight()-116,(int)area.getBottom()-24,100,14,juce::Justification::right);
  g.setColour(panel2);g.fillRoundedRectangle(36,444,(float)getWidth()-72,64,10);g.setColour(muted);g.setFont(10.0f);g.drawText("CORRECTION SETTINGS",50,452,240,14,juce::Justification::left);
  // Piano strip: one label per key; visual reference keyboard for scale selection.
  const float kx=36.0f, ky=(float)getHeight()-124.0f, kw=((float)getWidth()-72.0f)/24.0f, kh=82.0f;
  g.setColour(muted);g.setFont(10.0f);g.drawText("PIANO / NOTE REFERENCE",36,(int)ky-20,getWidth()-72,14,juce::Justification::left);
  static const bool scaleNotes[13][12] = {
   {1,1,1,1,1,1,1,1,1,1,1,1},
   {1,0,1,0,1,1,0,1,0,1,0,1},
   {1,0,1,1,0,1,0,1,1,0,1,0},
   {1,0,1,1,0,1,0,1,1,0,0,1},
   {1,0,1,1,0,1,0,1,0,1,0,1},
   {1,0,1,1,0,1,0,1,0,1,1,0},
   {1,1,0,1,0,1,0,1,1,0,1,0},
   {1,0,1,0,1,0,1,1,0,1,0,1},
   {1,0,1,0,1,1,0,1,0,1,1,0},
   {1,1,0,1,0,1,1,0,1,0,1,0},
   {1,0,1,0,1,0,0,1,0,1,0,0},
   {1,0,0,1,0,1,0,1,0,0,1,0},
   {1,0,0,1,0,1,1,1,0,0,1,0}
  };
  const int root=(int)processor.state.getRawParameterValue("key")->load();
  const int scale=juce::jlimit(0,12,(int)processor.state.getRawParameterValue("scale")->load());
  const int active=processor.getDetectedMidiNote()>=0.0f?(int)std::lround(processor.getDetectedMidiNote())%12:-1;
  for(int i=0;i<24;++i){
   float x=kx+i*kw;const int pc=i%12;const bool black=(pc==1||pc==3||pc==6||pc==8||pc==10);
   const bool inScale=scaleNotes[scale][(pc-root+12)%12];
   const bool isActive=(pc==active);
   juce::Colour keyColour=black?juce::Colour(0xff202633):juce::Colour(0xffdce4ee);
   if(inScale) keyColour=black?juce::Colour(0xff4a2029):juce::Colour(0xffffa0aa);
   if(isActive) keyColour=accent;
   g.setColour(keyColour);g.fillRoundedRectangle(x,ky,kw-1,kh,3);
   g.setColour(black?muted:juce::Colour(0xff394253));
   const juce::StringArray names{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
   if(!black)g.drawText(names[pc],(int)x,(int)ky+kh-20,(int)kw-1,15,juce::Justification::centred);
  }
 }else{
  g.setColour(text);g.setFont(juce::Font(juce::FontOptions(17.0f,juce::Font::bold)));g.drawText(tabNames[selectedTab],36,158,400,28,juce::Justification::left);
  g.setColour(muted);g.setFont(11.0f);g.drawText("Adjust the module controls below. Parameters are automatable and saved with the session.",36,186,getWidth()-72,20,juce::Justification::left);
  if(selectedTab==0){
   const juce::Rectangle<float> graph(36.0f,238.0f,(float)getWidth()-72.0f,122.0f);
   g.setColour(panel2);g.fillRoundedRectangle(graph,10.0f);g.setColour(line);g.drawRoundedRectangle(graph,10.0f,1.0f);
   for(int db=-12;db<=12;db+=6){float y=graph.getCentreY()-db*(graph.getHeight()-18.0f)/26.0f;g.setColour(db==0?juce::Colour(0xff465365):line);g.drawHorizontalLine((int)y,graph.getX()+8,graph.getRight()-8);g.setColour(muted);g.setFont(9.0f);g.drawText(juce::String(db), (int)graph.getX()+7,(int)y-9,26,16,juce::Justification::left);}
   for(int i=0;i<9;++i){float x=graph.getX()+10.0f+i*(graph.getWidth()-20.0f)/8.0f;g.setColour(line);g.drawVerticalLine((int)x,graph.getY()+8,graph.getBottom()-8);}
   // Live pre-EQ input spectrum, refreshed from a fixed-size FFT in the audio engine.
   const float spectrumStep=(graph.getWidth()-20.0f)/(float)ZMBFRMAudioProcessor::spectrumBinCount;
   g.setColour(juce::Colour(0x334f7895));
   for(int bin=0;bin<ZMBFRMAudioProcessor::spectrumBinCount;++bin){
    const float level=processor.getSpectrumBin(bin);
    const float barHeight=level*(graph.getHeight()-24.0f)*0.78f;
    const float x=graph.getX()+10.0f+(float)bin*spectrumStep;
    g.fillRoundedRectangle(x,graph.getBottom()-10.0f-barHeight,juce::jmax(1.0f,spectrumStep-1.0f),barHeight,1.5f);
   }
   g.setColour(muted);g.setFont(9.0f);g.drawText("LIVE INPUT SPECTRUM  /  EQ RESPONSE",graph.getX()+12.0f,graph.getY()+5.0f,250.0f,12.0f,juce::Justification::left);
   auto* low=processor.state.getRawParameterValue("low");auto* mid=processor.state.getRawParameterValue("mid");auto* high=processor.state.getRawParameterValue("high");
   auto* lf=processor.state.getRawParameterValue("lowFreq");auto* mf=processor.state.getRawParameterValue("midFreq");auto* mq=processor.state.getRawParameterValue("midQ");auto* hf=processor.state.getRawParameterValue("highFreq");
   const float lg=low->load(),mg=mid->load(),hg=high->load(),q=mq->load();
   const double sr=juce::jmax(22050.0,processor.getSampleRate());
   const float nyquist=(float)sr*0.49f;
   const float lfr=juce::jlimit(20.0f,nyquist,lf->load());
   const float mfr=juce::jlimit(20.0f,nyquist,mf->load());
   const float hfr=juce::jlimit(20.0f,nyquist,hf->load());
   auto lowCoeffs=juce::dsp::IIR::Coefficients<float>::makeLowShelf(sr,lfr,0.707,juce::Decibels::decibelsToGain(lg));
   auto midCoeffs=juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr,mfr,q,juce::Decibels::decibelsToGain(mg));
   auto highCoeffs=juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr,hfr,0.707,juce::Decibels::decibelsToGain(hg));
   std::array<juce::dsp::IIR::Coefficients<float>::Ptr,5> extraCoeffs;
   for(int band=4;band<=8;++band){
    const auto suffix=juce::String(band);
    const float bgain=processor.state.getRawParameterValue(("band"+suffix+"Gain").toRawUTF8())->load();
    const float bfreq=juce::jlimit(20.0f,nyquist,processor.state.getRawParameterValue(("band"+suffix+"Freq").toRawUTF8())->load());
    const float bq=processor.state.getRawParameterValue(("band"+suffix+"Q").toRawUTF8())->load();
    extraCoeffs[(size_t)(band-4)]=juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr,bfreq,bq,juce::Decibels::decibelsToGain(bgain));
   }
   juce::Path curve;bool started=false;
   for(int px=0;px<(int)graph.getWidth()-20;++px){
    const float norm=(float)px/(graph.getWidth()-20.0f);
    const float hz=20.0f*std::pow(1000.0f,norm);
    const float x=graph.getX()+10.0f+px;
    double magnitude=(double)lowCoeffs->getMagnitudeForFrequency(hz,sr)
                         * (double)midCoeffs->getMagnitudeForFrequency(hz,sr)
                         * (double)highCoeffs->getMagnitudeForFrequency(hz,sr);
    for(const auto& bc:extraCoeffs) magnitude *= (double)bc->getMagnitudeForFrequency(hz,sr);
    const float db=juce::jlimit(-18.0f,18.0f,(float)juce::Decibels::gainToDecibels((float)magnitude,-36.0f));
    const float y=graph.getCentreY()-db*(graph.getHeight()-18.0f)/26.0f;
    if(!started){curve.startNewSubPath(x,y);started=true;}else curve.lineTo(x,y);
   }
   g.setColour(juce::Colour(0x44ff3048));g.strokePath(curve,juce::PathStrokeType(7.0f));g.setColour(accent);g.strokePath(curve,juce::PathStrokeType(2.0f));
   const float freqs[]={lfr,mfr,hfr,
    processor.state.getRawParameterValue("band4Freq")->load(),processor.state.getRawParameterValue("band5Freq")->load(),
    processor.state.getRawParameterValue("band6Freq")->load(),processor.state.getRawParameterValue("band7Freq")->load(),processor.state.getRawParameterValue("band8Freq")->load()};
   const float gains[]={lg,mg,hg,
    processor.state.getRawParameterValue("band4Gain")->load(),processor.state.getRawParameterValue("band5Gain")->load(),
    processor.state.getRawParameterValue("band6Gain")->load(),processor.state.getRawParameterValue("band7Gain")->load(),processor.state.getRawParameterValue("band8Gain")->load()};
   for(int i=0;i<8;++i){float nx=std::log(juce::jlimit(20.0f,20000.0f,freqs[i])/20.0f)/std::log(1000.0f);float x=graph.getX()+10.0f+nx*(graph.getWidth()-20.0f);float y=graph.getCentreY()-gains[i]*(graph.getHeight()-18.0f)/26.0f;g.setColour(panel);g.fillEllipse(x-6,y-6,12,12);g.setColour(i<3?accent:juce::Colour(0xfff2b66d));g.drawEllipse(x-6,y-6,12,12,2.0f);}
   g.setColour(muted);g.setFont(9.0f);
   const int axisY=(int)graph.getBottom()-17;
   g.drawText("20 Hz",(int)graph.getX()+8,axisY,50,12,juce::Justification::left);
   g.drawText("1 kHz",(int)graph.getCentreX()-25,axisY,50,12,juce::Justification::centred);
   g.drawText("20 kHz",(int)graph.getRight()-58,axisY,50,12,juce::Justification::right);
  }
  if(selectedTab==4){g.setColour(panel2);g.fillRoundedRectangle(36.0f,220.0f,(float)getWidth()-72.0f,100.0f,10.0f);g.setColour(accent);g.drawText("SIBILANCE CONTROL  /  HIGH-FREQUENCY DYNAMIC REDUCTION",52,244,getWidth()-104,24,juce::Justification::left);}
 }
}

void ZMBFRMAudioProcessorEditor::resized()
{
 title.setBounds(24,10,280,38);subtitle.setBounds(26,49,500,20);presetBox.setBounds(330,14,160,26);savePresetButton.setBounds(500,14,54,26);loadPresetButton.setBounds(560,14,54,26);liveNote.setBounds(630,10,getWidth()-660,24);
 inputMeter.setBounds(getWidth()-300,38,120,20);outputMeter.setBounds(getWidth()-170,38,140,20);
 const int tabW=(getWidth()-44)/9;for(int i=0;i<9;++i)tabs[i].setBounds(22+i*tabW,91,tabW-4,30);
 if(selectedTab==1){keyBox.setBounds(48,472,100,26);scaleBox.setBounds(158,472,190,26);toggles[7].setBounds(690,472,220,26);const int baseY=514;juce::Slider* ss[]={&speed,&humanize,&detune};for(int i=0;i<3;++i){int x=50+i*150;tuneLabels[i].setBounds(x,baseY,136,18);ss[i]->setBounds(x+6,baseY+14,124,72);}}
 else{
  const int topY=(selectedTab==0?208:(selectedTab==4?300:230));int visibleIndex=0;
  for(int i=0;i<8;++i)if(toggles[i].isVisible()){toggles[i].setBounds(42+(visibleIndex%4)*220,topY+(visibleIndex/4)*32,210,24);++visibleIndex;}
  if(selectedTab==0){
   const int eqIds[]={0,1,2,13,14,15,16,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39};
   const int left=30, usable=getWidth()-60, cell=usable/11;
   for(int j=0;j<22;++j){
    const int i=eqIds[j], row=j/11, col=j%11, x=left+col*cell, y=row==0?374:524;
    labels[i].setBounds(x,y,cell-3,18);
    sliders[i].setBounds(x+2,y+18,cell-7,112);
   }
  } else if(selectedTab==2) {
   const int ids[]={3,4,17,18};
   for(int j=0;j<4;++j){int i=ids[j],x=50+j*175;labels[i].setBounds(x,360,160,20);sliders[i].setBounds(x+4,380,145,140);}
  } else {
   visibleIndex=0;
   for(int i=0;i<45;++i)if(sliders[i].isVisible()){
    const int col=visibleIndex%4, row=visibleIndex/4; ++visibleIndex;
    const int cellW=(getWidth()-100)/4, x=40+col*cellW, y=350+row*150;
    labels[i].setBounds(x,y,cellW-8,20); sliders[i].setBounds(x+4,y+22,cellW-16,124);
   }
  }
 }
}
void ZMBFRMAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
 if(selectedTab==1){
  const float ky=(float)getHeight()-124.0f;
  if(e.y>=ky && e.y<=ky+82.0f && e.x>=36.0f && e.x<=getWidth()-36.0f){
   const int index=juce::jlimit(0,23,(int)((e.x-36.0f)/((float(getWidth())-72.0f)/24.0f)));
   keyBox.setSelectedItemIndex(index%12,juce::sendNotification);
  }
  return;
 }
 if(selectedTab==0){
  selectedEQNode=-1;
  const juce::Rectangle<float> graph(36.0f,238.0f,(float)getWidth()-72.0f,122.0f);
  if(!graph.contains(e.position)) return;
  const char* freqIDs[]={"lowFreq","midFreq","highFreq","band4Freq","band5Freq","band6Freq","band7Freq","band8Freq"};
  const char* gainIDs[]={"low","mid","high","band4Gain","band5Gain","band6Gain","band7Gain","band8Gain"};
  float best=22.0f;selectedEQNode=-1;
  for(int i=0;i<8;++i){
   const float freq=processor.state.getRawParameterValue(freqIDs[i])->load();
   const float gain=processor.state.getRawParameterValue(gainIDs[i])->load();
   const float nx=std::log(juce::jlimit(20.0f,20000.0f,freq)/20.0f)/std::log(1000.0f);
   const float x=graph.getX()+10.0f+nx*(graph.getWidth()-20.0f);
   const float y=graph.getCentreY()-gain*(graph.getHeight()-18.0f)/26.0f;
   const float dist=e.position.getDistanceFrom(juce::Point<float>(x,y));
   if(dist<best){best=dist;selectedEQNode=i;}
  }
  if(selectedEQNode>=0) setMouseCursor(juce::MouseCursor::DraggingHandCursor);
 }
}

void ZMBFRMAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e)
{
 if(selectedTab!=0 || selectedEQNode<0) return;
 const juce::Rectangle<float> graph(36.0f,238.0f,(float)getWidth()-72.0f,122.0f);
 const float x=juce::jlimit(graph.getX()+10.0f,graph.getRight()-10.0f,e.position.x);
 const float y=juce::jlimit(graph.getY()+12.0f,graph.getBottom()-12.0f,e.position.y);
 const float norm=(x-(graph.getX()+10.0f))/(graph.getWidth()-20.0f);
 const float hz=20.0f*std::pow(1000.0f,norm);
 const float gain=juce::jlimit(-12.0f,12.0f,(graph.getCentreY()-y)*26.0f/(graph.getHeight()-18.0f));
 const char* gainIDs[]={"low","mid","high","band4Gain","band5Gain","band6Gain","band7Gain","band8Gain"};
 const char* freqIDs[]={"lowFreq","midFreq","highFreq","band4Freq","band5Freq","band6Freq","band7Freq","band8Freq"};
 auto* gp=processor.state.getParameter(gainIDs[selectedEQNode]);
 auto* fp=processor.state.getParameter(freqIDs[selectedEQNode]);
 if(gp){gp->beginChangeGesture();gp->setValueNotifyingHost(gp->convertTo0to1(gain));gp->endChangeGesture();}
 if(fp){fp->beginChangeGesture();fp->setValueNotifyingHost(fp->convertTo0to1(hz));fp->endChangeGesture();}
 repaint();
}
void ZMBFRMAudioProcessorEditor::mouseUp(const juce::MouseEvent& e)
{
 juce::ignoreUnused(e);
 selectedEQNode=-1;
 setMouseCursor(juce::MouseCursor::NormalCursor);
}
