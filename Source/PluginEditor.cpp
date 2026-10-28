#include "PluginEditor.h"

namespace {
const juce::Colour bg(0xff0b0d12), panel(0xff131722), panel2(0xff191f2c), line(0xff293244), text(0xffedf2fb), muted(0xff8793a8), accent(0xff7ce7d4), blue(0xff82aaff);
const juce::StringArray tabNames { "ZMBEQ", "ZMBTUNE", "ZMBCOMP", "ZMBSATURATION", "ZMBDEESSER", "ZMBREVERB", "ZMBDELAY", "ZMBCHORUS", "ZMBMASTER" };
}

ZMBFRMAudioProcessorEditor::ZMBFRMAudioProcessorEditor(ZMBFRMAudioProcessor& p) : AudioProcessorEditor(&p), processor(p)
{
 setSize(1040, 720); setResizable(true, true); setResizeLimits(900, 640, 1500, 1000);
 title.setText("ZMBFRM", juce::dontSendNotification); title.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold))); title.setColour(juce::Label::textColourId, text); addAndMakeVisible(title);
 subtitle.setText("VOCAL PRODUCTION SUITE   /   REAL-TIME PITCH ENGINE", juce::dontSendNotification); subtitle.setFont(juce::Font(juce::FontOptions(10.0f))); subtitle.setColour(juce::Label::textColourId, muted); addAndMakeVisible(subtitle);
 liveNote.setText("WAITING FOR VOICE", juce::dontSendNotification); liveNote.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold))); liveNote.setColour(juce::Label::textColourId, accent); liveNote.setJustificationType(juce::Justification::centredRight); addAndMakeVisible(liveNote);
 for (int i=0;i<9;++i) { tabs[i].setButtonText(tabNames[i]); tabs[i].setClickingTogglesState(false); tabs[i].setColour(juce::TextButton::buttonColourId, panel2); tabs[i].setColour(juce::TextButton::textColourOffId, muted); tabs[i].setColour(juce::TextButton::textColourOnId, accent); tabs[i].onClick=[this,i]{selectTab(i);}; addAndMakeVisible(tabs[i]); }
 const char* ids[]={"low","mid","high","threshold","ratio","sat","chorus","reverb","delay","mix","out"};
 const char* names[]={"LOW SHELF","MID","HIGH SHELF","THRESHOLD","RATIO","DRIVE","DEPTH","ROOM","TIME","WET / DRY","OUTPUT"};
 for(int i=0;i<11;++i){ sliders[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); sliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow,false,68,20); sliders[i].setColour(juce::Slider::rotarySliderFillColourId,accent); sliders[i].setColour(juce::Slider::rotarySliderOutlineColourId,line); sliders[i].setColour(juce::Slider::thumbColourId,text); sliders[i].setColour(juce::Slider::textBoxTextColourId,text); sliders[i].setColour(juce::Slider::textBoxBackgroundColourId,panel2); addAndMakeVisible(sliders[i]); labels[i].setText(names[i],juce::dontSendNotification); labels[i].setJustificationType(juce::Justification::centred); labels[i].setColour(juce::Label::textColourId,muted); addAndMakeVisible(labels[i]); sliderAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,ids[i],sliders[i]); }
 const char* bids[]={"eqOn","compOn","satOn","chorusOn","reverbOn","delayOn","tuneOn"};
 const char* bnames[]={"EQ ACTIVE","COMP ACTIVE","SATURATION","CHORUS","REVERB","DELAY","ZMBTUNE ACTIVE"};
 for(int i=0;i<7;++i){toggles[i].setButtonText(bnames[i]);toggles[i].setColour(juce::ToggleButton::textColourId,text);toggles[i].setColour(juce::ToggleButton::tickColourId,accent);addAndMakeVisible(toggles[i]);buttonAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,bids[i],toggles[i]);}
 keyBox.addItemList(juce::StringArray{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},1);
 scaleBox.addItemList(juce::StringArray{"Chromatic","Major","Natural Minor","Harmonic Minor","Melodic Minor","Dorian","Phrygian","Lydian","Mixolydian","Locrian","Major Pentatonic","Minor Pentatonic","Blues"},1);
 for(auto* box:{&keyBox,&scaleBox}) { box->setColour(juce::ComboBox::backgroundColourId,panel2);box->setColour(juce::ComboBox::textColourId,text);box->setColour(juce::ComboBox::outlineColourId,line);box->setColour(juce::ComboBox::arrowColourId,accent);addAndMakeVisible(*box); }
 comboAttachments[0]=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"key",keyBox);comboAttachments[1]=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"scale",scaleBox);
 juce::Slider* tuneSliders[]={&speed,&humanize,&detune};const char* tuneIds[]={"speed","humanize","detune"};const char* tuneNames[]={"RETUNE SPEED","HUMANIZE","DETUNE / CENTS"};
 for(int i=0;i<3;++i){tuneSliders[i]->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);tuneSliders[i]->setTextBoxStyle(juce::Slider::TextBoxBelow,false,74,20);tuneSliders[i]->setColour(juce::Slider::rotarySliderFillColourId,accent);tuneSliders[i]->setColour(juce::Slider::rotarySliderOutlineColourId,line);tuneSliders[i]->setColour(juce::Slider::thumbColourId,text);tuneSliders[i]->setColour(juce::Slider::textBoxTextColourId,text);tuneSliders[i]->setColour(juce::Slider::textBoxBackgroundColourId,panel2);addAndMakeVisible(*tuneSliders[i]);tuneLabels[i].setText(tuneNames[i],juce::dontSendNotification);tuneLabels[i].setJustificationType(juce::Justification::centred);tuneLabels[i].setColour(juce::Label::textColourId,muted);addAndMakeVisible(tuneLabels[i]);tuneAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,tuneIds[i],*tuneSliders[i]);}
 selectTab(1); startTimerHz(15);
}

void ZMBFRMAudioProcessorEditor::selectTab(int index){selectedTab=juce::jlimit(0,8,index);for(int i=0;i<9;++i){tabs[i].setToggleState(i==selectedTab,juce::dontSendNotification);tabs[i].setColour(juce::TextButton::buttonColourId,i==selectedTab?juce::Colour(0xff203b3a):panel2);tabs[i].setColour(juce::TextButton::textColourOffId,i==selectedTab?accent:muted);}updateVisibility();repaint();}
void ZMBFRMAudioProcessorEditor::updateVisibility(){
 const bool tune=selectedTab==1;for(auto& s:sliders)s.setVisible(false);for(auto& l:labels)l.setVisible(false);for(auto& t:toggles)t.setVisible(false);
 keyBox.setVisible(tune);scaleBox.setVisible(tune);speed.setVisible(tune);humanize.setVisible(tune);detune.setVisible(tune);for(auto& l:tuneLabels)l.setVisible(tune);liveNote.setVisible(tune);
 auto showSlider=[this](int i){sliders[i].setVisible(true);labels[i].setVisible(true);};
 switch(selectedTab){case 0:showSlider(0);showSlider(1);showSlider(2);toggles[0].setVisible(true);break;case 2:showSlider(3);showSlider(4);toggles[1].setVisible(true);break;case 3:showSlider(5);toggles[2].setVisible(true);break;case 4:break;case 5:showSlider(7);toggles[4].setVisible(true);break;case 6:showSlider(8);toggles[5].setVisible(true);break;case 7:showSlider(6);toggles[3].setVisible(true);break;case 8:showSlider(9);showSlider(10);toggles[0].setVisible(true);toggles[1].setVisible(true);toggles[2].setVisible(true);toggles[3].setVisible(true);toggles[4].setVisible(true);toggles[5].setVisible(true);toggles[6].setVisible(true);break;default:break;}
}
void ZMBFRMAudioProcessorEditor::timerCallback(){const float note=processor.getDetectedMidiNote();if(note<0.0f)liveNote.setText("WAITING FOR VOICE",juce::dontSendNotification);else{const juce::StringArray notes{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};int midi=(int)std::lround(note);liveNote.setText(notes[midi%12]+juce::String(midi/12-1)+"   "+juce::String(processor.getCorrectionCents(),1)+" cents",juce::dontSendNotification);}repaint();}

void ZMBFRMAudioProcessorEditor::paint(juce::Graphics& g)
{
 g.fillAll(bg);g.setColour(panel);g.fillRoundedRectangle(18,84,(float)getWidth()-36,44,10);g.setColour(panel);g.fillRoundedRectangle(18,142,(float)getWidth()-36,(float)getHeight()-160,14);
 g.setColour(line);g.drawRoundedRectangle(18,84,(float)getWidth()-36,44,10,1);g.drawRoundedRectangle(18,142,(float)getWidth()-36,(float)getHeight()-160,14,1);
 g.setColour(muted);g.setFont(10.0f);g.drawText("ZMB AUDIO  /  VST3",getWidth()-190,34,165,18,juce::Justification::centredRight);
 if(selectedTab==1){
  auto area=juce::Rectangle<float>(36,160,(float)getWidth()-72,270);g.setColour(panel2);g.fillRoundedRectangle(area,10);g.setColour(line);g.drawRoundedRectangle(area,10,1);
  g.setColour(muted);g.setFont(10.0f);g.drawText("PITCH TRACKING  /  INPUT ANALYSIS",area.toNearestInt().reduced(16).removeFromTop(20),juce::Justification::left);
  // Subtle grid and pitch guide lines.
  for(int i=0;i<9;++i){float y=area.getY()+38.0f+i*(area.getHeight()-54.0f)/8.0f;g.setColour(i==4?juce::Colour(0xff344b58):line);g.drawHorizontalLine((int)y,area.getX()+14,area.getRight()-14);}
  for(int i=0;i<13;++i){float x=area.getX()+14.0f+i*(area.getWidth()-28.0f)/12.0f;g.setColour(line);g.drawVerticalLine((int)x,area.getY()+30,area.getBottom()-14);}
  g.setColour(accent);g.setFont(juce::Font(juce::FontOptions(12.0f,juce::Font::bold)));g.drawText("ZMBTUNE",area.getX()+20,area.getY()+35,110,20,juce::Justification::left);
  const float note=processor.getDetectedMidiNote();if(note>=0.0f){float y=area.getCentreY()-(note-60.0f)*5.0f;y=juce::jlimit(area.getY()+40.0f,area.getBottom()-20.0f,y);g.setColour(juce::Colour(0x557ce7d4));g.fillRoundedRectangle(area.getX()+15,y-10,area.getWidth()-30,20,5);g.setColour(accent);g.fillEllipse(area.getCentreX()-5,y-5,10,10);}
  g.setColour(muted);g.setFont(10.0f);g.drawText("LOWER PITCH",(int)area.getX()+16,(int)area.getBottom()-24,100,14,juce::Justification::left);g.drawText("HIGHER PITCH",(int)area.getRight()-116,(int)area.getBottom()-24,100,14,juce::Justification::right);
  g.setColour(panel2);g.fillRoundedRectangle(36,444,(float)getWidth()-72,64,10);g.setColour(muted);g.setFont(10.0f);g.drawText("CORRECTION SETTINGS",50,452,240,14,juce::Justification::left);
  // Piano strip: one label per key; visual reference keyboard for scale selection.
  const float kx=36.0f, ky=(float)getHeight()-124.0f, kw=((float)getWidth()-72.0f)/24.0f, kh=82.0f;
  g.setColour(muted);g.setFont(10.0f);g.drawText("PIANO / NOTE REFERENCE",36,(int)ky-20,getWidth()-72,14,juce::Justification::left);
  for(int i=0;i<24;++i){float x=kx+i*kw;bool black=false;const int pc=i%12;black=(pc==1||pc==3||pc==6||pc==8||pc==10);g.setColour(black?juce::Colour(0xff202633):juce::Colour(0xffdce4ee));g.fillRoundedRectangle(x,ky,kw-1,kh,3);g.setColour(black?muted:juce::Colour(0xff394253));const juce::StringArray names{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};if(!black)g.drawText(names[pc],(int)x,(int)ky+kh-20,(int)kw-1,15,juce::Justification::centred);}
 }else{
  g.setColour(text);g.setFont(juce::Font(juce::FontOptions(17.0f,juce::Font::bold)));g.drawText(tabNames[selectedTab],36,158,400,28,juce::Justification::left);
  g.setColour(muted);g.setFont(11.0f);g.drawText("Adjust the module controls below. Parameters are automatable and saved with the session.",36,186,getWidth()-72,20,juce::Justification::left);
  if(selectedTab==0){g.setColour(panel2);g.fillRoundedRectangle(36,220,getWidth()-72,120,10);g.setColour(line);for(int i=0;i<9;++i)g.drawHorizontalLine(232+i*12,48,getWidth()-48);g.setColour(accent);g.drawText("LOW      MID      HIGH",52,228,320,16,juce::Justification::left);}
  if(selectedTab==4){g.setColour(panel2);g.fillRoundedRectangle(36,220,getWidth()-72,100,10);g.setColour(muted);g.drawText("DE-ESSER MODULE  /  UI PLACEHOLDER — PROCESSING NOT YET IMPLEMENTED",52,244,getWidth()-104,24,juce::Justification::left);}
 }
}

void ZMBFRMAudioProcessorEditor::resized()
{
 title.setBounds(24,10,280,38);subtitle.setBounds(26,49,500,20);liveNote.setBounds(getWidth()-330,28,300,30);
 const int tabW=(getWidth()-44)/9;for(int i=0;i<9;++i)tabs[i].setBounds(22+i*tabW,91,tabW-4,30);
 if(selectedTab==1){keyBox.setBounds(48,472,100,26);scaleBox.setBounds(158,472,190,26);const int baseY=514;juce::Slider* ss[]={&speed,&humanize,&detune};for(int i=0;i<3;++i){int x=50+i*150;tuneLabels[i].setBounds(x,baseY,136,18);ss[i]->setBounds(x+6,baseY+14,124,72);}}
 else{
  const int topY=(selectedTab==4?300:230);int visibleIndex=0;
  for(int i=0;i<7;++i)if(toggles[i].isVisible()){toggles[i].setBounds(42+(visibleIndex%4)*220,topY+(visibleIndex/4)*32,210,24);++visibleIndex;}
  visibleIndex=0;
  for(int i=0;i<11;++i)if(sliders[i].isVisible()){int col=visibleIndex++;int x=50+col*150;labels[i].setBounds(x,360,140,20);sliders[i].setBounds(x+4,380,132,140);}
 }
}
void ZMBFRMAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
 if(selectedTab!=1)return;const float ky=(float)getHeight()-124.0f;if(e.y>=ky && e.y<=ky+82.0f){const int index=juce::jlimit(0,23,(int)((e.x-36.0f)/((float(getWidth())-72.0f)/24.0f)));keyBox.setSelectedItemIndex(index%12,juce::sendNotification);}
}
