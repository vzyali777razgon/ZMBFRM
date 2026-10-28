#include "PluginEditor.h"

namespace {
const juce::Colour bg(0xff0b0d12), panel(0xff131722), panel2(0xff191f2c), line(0xff293244), text(0xffedf2fb), muted(0xff8793a8), accent(0xff7ce7d4);
const juce::StringArray tabNames { "ZMBEQ", "ZMBTUNE", "ZMBCOMP", "ZMBSATURATION", "ZMBDEESSER", "ZMBREVERB", "ZMBDELAY", "ZMBCHORUS", "ZMBMASTER" };
}

ZMBFRMAudioProcessorEditor::ZMBFRMAudioProcessorEditor(ZMBFRMAudioProcessor& p) : AudioProcessorEditor(&p), processor(p)
{
 setSize(1120, 760); setResizable(true, true); setResizeLimits(900, 640, 1500, 1000);
 title.setText("ZMBFRM", juce::dontSendNotification); title.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold))); title.setColour(juce::Label::textColourId, text); addAndMakeVisible(title);
 subtitle.setText("VERSION 1.3   /   VOCAL PRODUCTION SUITE   /   REAL-TIME PITCH ENGINE", juce::dontSendNotification); subtitle.setFont(juce::Font(juce::FontOptions(10.0f))); subtitle.setColour(juce::Label::textColourId, muted); addAndMakeVisible(subtitle);
 liveNote.setText("WAITING FOR VOICE", juce::dontSendNotification); liveNote.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold))); liveNote.setColour(juce::Label::textColourId, accent); liveNote.setJustificationType(juce::Justification::centredRight); addAndMakeVisible(liveNote);
 inputMeter.setText("IN  -inf dB", juce::dontSendNotification);
 outputMeter.setText("OUT  -inf dB", juce::dontSendNotification);
 for (auto* meter : { &inputMeter, &outputMeter }) {
  meter->setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
  meter->setColour(juce::Label::textColourId, accent);
  meter->setJustificationType(juce::Justification::centredRight);
  addAndMakeVisible(*meter);
 }
 for (int i=0;i<9;++i) { tabs[i].setButtonText(tabNames[i]); tabs[i].setClickingTogglesState(false); tabs[i].setColour(juce::TextButton::buttonColourId, panel2); tabs[i].setColour(juce::TextButton::textColourOffId, muted); tabs[i].setColour(juce::TextButton::textColourOnId, accent); tabs[i].onClick=[this,i]{selectTab(i);}; addAndMakeVisible(tabs[i]); }
 const char* ids[]={"low","mid","high","threshold","ratio","sat","deessAmount","deessFreq","reverb","delay","chorus","mix","out","lowFreq","midFreq","midQ","highFreq","compAttack","compRelease","satMix","reverbDamping","delayFeedback","delayMix","chorusRate","deessThreshold"};
 const char* names[]={"LOW GAIN","MID GAIN","HIGH GAIN","THRESHOLD","RATIO","DRIVE","DE-ESS AMOUNT","DE-ESS FREQ","ROOM","DELAY TIME","CHORUS DEPTH","WET / DRY","OUTPUT","LOW FREQ","MID FREQ","MID Q","HIGH FREQ","ATTACK","RELEASE","SAT MIX","REVERB DAMPING","DELAY FEEDBACK","DELAY MIX","CHORUS RATE","DE-ESS THRESHOLD"};
 for(int i=0;i<25;++i){ sliders[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); sliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow,false,68,20); sliders[i].setColour(juce::Slider::rotarySliderFillColourId,accent); sliders[i].setColour(juce::Slider::rotarySliderOutlineColourId,line); sliders[i].setColour(juce::Slider::thumbColourId,text); sliders[i].setColour(juce::Slider::textBoxTextColourId,text); sliders[i].setColour(juce::Slider::textBoxBackgroundColourId,panel2); addAndMakeVisible(sliders[i]); sliders[i].setTooltip(juce::String(names[i]) + " — double-click to reset"); labels[i].setText(names[i],juce::dontSendNotification); labels[i].setJustificationType(juce::Justification::centred); labels[i].setColour(juce::Label::textColourId,muted); addAndMakeVisible(labels[i]); sliderAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,ids[i],sliders[i]); }
 const char* bids[]={"eqOn","compOn","satOn","deesserOn","reverbOn","delayOn","chorusOn","tuneOn"};
 const char* bnames[]={"EQ ACTIVE","COMP ACTIVE","SATURATION","DE-ESSER ACTIVE","REVERB ACTIVE","DELAY ACTIVE","CHORUS ACTIVE","ZMBTUNE ACTIVE"};
 for(int i=0;i<8;++i){toggles[i].setButtonText(bnames[i]);toggles[i].setTooltip("Enable or bypass this processing module");toggles[i].setColour(juce::ToggleButton::textColourId,text);toggles[i].setColour(juce::ToggleButton::tickColourId,accent);addAndMakeVisible(toggles[i]);buttonAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,bids[i],toggles[i]);}
 keyBox.addItemList(juce::StringArray{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},1);
 scaleBox.addItemList(juce::StringArray{"Chromatic","Major","Natural Minor","Harmonic Minor","Melodic Minor","Dorian","Phrygian","Lydian","Mixolydian","Locrian","Major Pentatonic","Minor Pentatonic","Blues"},1);
 for(auto* box:{&keyBox,&scaleBox}) { box->setTooltip("Select the musical key or scale used by pitch correction"); box->setColour(juce::ComboBox::backgroundColourId,panel2);box->setColour(juce::ComboBox::textColourId,text);box->setColour(juce::ComboBox::outlineColourId,line);box->setColour(juce::ComboBox::arrowColourId,accent);addAndMakeVisible(*box); }
 comboAttachments[0]=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"key",keyBox);comboAttachments[1]=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"scale",scaleBox);
 juce::Slider* tuneSliders[]={&speed,&humanize,&detune};const char* tuneIds[]={"speed","humanize","detune"};const char* tuneNames[]={"RETUNE SPEED","HUMANIZE","DETUNE / CENTS"};
 for(int i=0;i<3;++i){tuneSliders[i]->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);tuneSliders[i]->setTextBoxStyle(juce::Slider::TextBoxBelow,false,74,20);tuneSliders[i]->setColour(juce::Slider::rotarySliderFillColourId,accent);tuneSliders[i]->setColour(juce::Slider::rotarySliderOutlineColourId,line);tuneSliders[i]->setColour(juce::Slider::thumbColourId,text);tuneSliders[i]->setColour(juce::Slider::textBoxTextColourId,text);tuneSliders[i]->setColour(juce::Slider::textBoxBackgroundColourId,panel2);addAndMakeVisible(*tuneSliders[i]);tuneLabels[i].setText(tuneNames[i],juce::dontSendNotification);tuneLabels[i].setJustificationType(juce::Justification::centred);tuneLabels[i].setColour(juce::Label::textColourId,muted);addAndMakeVisible(tuneLabels[i]);tuneAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,tuneIds[i],*tuneSliders[i]);}
 selectTab(1); startTimerHz(15);
}

void ZMBFRMAudioProcessorEditor::selectTab(int index){selectedTab=juce::jlimit(0,8,index);for(int i=0;i<9;++i){tabs[i].setToggleState(i==selectedTab,juce::dontSendNotification);tabs[i].setColour(juce::TextButton::buttonColourId,i==selectedTab?juce::Colour(0xff203b3a):panel2);tabs[i].setColour(juce::TextButton::textColourOffId,i==selectedTab?accent:muted);}updateVisibility();repaint();}
void ZMBFRMAudioProcessorEditor::updateVisibility(){
 const bool tune=selectedTab==1;
 for(auto& s:sliders)s.setVisible(false); for(auto& l:labels)l.setVisible(false); for(auto& t:toggles)t.setVisible(false);
 keyBox.setVisible(tune); scaleBox.setVisible(tune); speed.setVisible(tune); humanize.setVisible(tune); detune.setVisible(tune); for(auto& l:tuneLabels)l.setVisible(tune); liveNote.setVisible(true);
 auto ss=[this](int i){sliders[i].setVisible(true);labels[i].setVisible(true);};
 auto tt=[this](int i){toggles[i].setVisible(true);};
 switch(selectedTab){
 case 0: ss(0);ss(1);ss(2);ss(13);ss(14);ss(15);ss(16);tt(0);break;
 case 1: tt(7); break;
 case 2: ss(3);ss(4);ss(17);ss(18);tt(1);break;
 case 3: ss(5);ss(19);tt(2);break;
 case 4: ss(6);ss(7);ss(24);tt(3);break;
 case 5: ss(8);ss(20);tt(4);break;
 case 6: ss(9);ss(21);ss(22);tt(5);break;
 case 7: ss(10);ss(23);tt(6);break;
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
 repaint();
}

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
   if(inScale) keyColour=black?juce::Colour(0xff28504d):juce::Colour(0xffb7e9df);
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
   const juce::Rectangle<float> graph(36.0f,218.0f,(float)getWidth()-72.0f,132.0f);
   g.setColour(panel2);g.fillRoundedRectangle(graph,10.0f);g.setColour(line);g.drawRoundedRectangle(graph,10.0f,1.0f);
   for(int db=-12;db<=12;db+=6){float y=graph.getCentreY()-db*(graph.getHeight()-18.0f)/26.0f;g.setColour(db==0?juce::Colour(0xff465365):line);g.drawHorizontalLine((int)y,graph.getX()+8,graph.getRight()-8);g.setColour(muted);g.setFont(9.0f);g.drawText(juce::String(db), (int)graph.getX()+7,(int)y-9,26,16,juce::Justification::left);}
   for(int i=0;i<9;++i){float x=graph.getX()+10.0f+i*(graph.getWidth()-20.0f)/8.0f;g.setColour(line);g.drawVerticalLine((int)x,graph.getY()+8,graph.getBottom()-8);}
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
   g.setColour(juce::Colour(0x447ce7d4));g.strokePath(curve,juce::PathStrokeType(7.0f));g.setColour(accent);g.strokePath(curve,juce::PathStrokeType(2.0f));
   const float freqs[]={lfr,mfr,hfr,
    processor.state.getRawParameterValue("band4Freq")->load(),processor.state.getRawParameterValue("band5Freq")->load(),
    processor.state.getRawParameterValue("band6Freq")->load(),processor.state.getRawParameterValue("band7Freq")->load(),processor.state.getRawParameterValue("band8Freq")->load()};
   const float gains[]={lg,mg,hg,
    processor.state.getRawParameterValue("band4Gain")->load(),processor.state.getRawParameterValue("band5Gain")->load(),
    processor.state.getRawParameterValue("band6Gain")->load(),processor.state.getRawParameterValue("band7Gain")->load(),processor.state.getRawParameterValue("band8Gain")->load()};
   for(int i=0;i<8;++i){float nx=std::log(juce::jlimit(20.0f,20000.0f,freqs[i])/20.0f)/std::log(1000.0f);float x=graph.getX()+10.0f+nx*(graph.getWidth()-20.0f);float y=graph.getCentreY()-gains[i]*(graph.getHeight()-18.0f)/26.0f;g.setColour(panel);g.fillEllipse(x-6,y-6,12,12);g.setColour(i<3?accent:juce::Colour(0xfff2b66d));g.drawEllipse(x-6,y-6,12,12,2.0f);}
   g.setColour(muted);g.setFont(9.0f);g.drawText("20 Hz",graph.getX()+8,graph.getBottom()-17,50,12,juce::Justification::left);g.drawText("1 kHz",graph.getCentreX()-25,graph.getBottom()-17,50,12,juce::Justification::centred);g.drawText("20 kHz",graph.getRight()-58,graph.getBottom()-17,50,12,juce::Justification::right);
  }
  if(selectedTab==4){g.setColour(panel2);g.fillRoundedRectangle(36,220,getWidth()-72,100,10);g.setColour(accent);g.drawText("SIBILANCE CONTROL  /  HIGH-FREQUENCY DYNAMIC REDUCTION",52,244,getWidth()-104,24,juce::Justification::left);}
 }
}

void ZMBFRMAudioProcessorEditor::resized()
{
 title.setBounds(24,10,280,38);subtitle.setBounds(26,49,500,20);liveNote.setBounds(getWidth()-470,10,440,24);
 inputMeter.setBounds(getWidth()-300,38,120,20);outputMeter.setBounds(getWidth()-170,38,140,20);
 const int tabW=(getWidth()-44)/9;for(int i=0;i<9;++i)tabs[i].setBounds(22+i*tabW,91,tabW-4,30);
 if(selectedTab==1){keyBox.setBounds(48,472,100,26);scaleBox.setBounds(158,472,190,26);toggles[7].setBounds(690,472,220,26);const int baseY=514;juce::Slider* ss[]={&speed,&humanize,&detune};for(int i=0;i<3;++i){int x=50+i*150;tuneLabels[i].setBounds(x,baseY,136,18);ss[i]->setBounds(x+6,baseY+14,124,72);}}
 else{
  const int topY=(selectedTab==0?332:(selectedTab==4?300:230));int visibleIndex=0;
  for(int i=0;i<8;++i)if(toggles[i].isVisible()){toggles[i].setBounds(42+(visibleIndex%4)*220,topY+(visibleIndex/4)*32,210,24);++visibleIndex;}
  if(selectedTab==0){
   const int ids[]={0,1,2,13,14,15,16};
   const int gap=8, left=32, usable=getWidth()-64, cell=(usable-gap*6)/7;
   for(int j=0;j<7;++j){int i=ids[j],x=left+j*(cell+gap);labels[i].setBounds(x,370,cell,22);sliders[i].setBounds(x,392,cell,150);}
  } else if(selectedTab==2) {
   const int ids[]={3,4,17,18};const int cell=155;
   for(int j=0;j<4;++j){int i=ids[j],x=50+j*175;labels[i].setBounds(x,360,160,20);sliders[i].setBounds(x+4,380,145,140);}
  } else {
   visibleIndex=0;
   for(int i=0;i<25;++i)if(sliders[i].isVisible()){
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
  if(e.y>=ky && e.y<=ky+82.0f){
   const int index=juce::jlimit(0,23,(int)((e.x-36.0f)/((float(getWidth())-72.0f)/24.0f)));
   keyBox.setSelectedItemIndex(index%12,juce::sendNotification);
  }
  return;
 }
 if(selectedTab==0){
  const juce::Rectangle<float> graph(36.0f,218.0f,(float)getWidth()-72.0f,132.0f);
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
 const juce::Rectangle<float> graph(36.0f,218.0f,(float)getWidth()-72.0f,132.0f);
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
