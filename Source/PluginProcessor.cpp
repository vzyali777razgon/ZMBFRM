#include "PluginProcessor.h"
#include "PluginEditor.h"

ZMBFRMAudioProcessor::ZMBFRMAudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
                                  .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
   state(*this,nullptr,"ZMBFRM_STATE",createParameterLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout ZMBFRMAudioProcessor::createParameterLayout()
{
 using F=juce::AudioParameterFloat; using B=juce::AudioParameterBool; using C=juce::AudioParameterChoice;
 juce::AudioProcessorValueTreeState::ParameterLayout p;
 p.add(std::make_unique<B>("eqOn","ZMBEQ",true));
 p.add(std::make_unique<F>("low","Low",juce::NormalisableRange<float>(-12.f,12.f,.1f),0.f));
 p.add(std::make_unique<F>("mid","Mid",juce::NormalisableRange<float>(-12.f,12.f,.1f),0.f));
 p.add(std::make_unique<F>("high","High",juce::NormalisableRange<float>(-12.f,12.f,.1f),0.f));
 p.add(std::make_unique<B>("compOn","ZMBComp",true));
 p.add(std::make_unique<F>("threshold","Threshold",juce::NormalisableRange<float>(-48.f,0.f,.1f),-18.f));
 p.add(std::make_unique<F>("ratio","Ratio",juce::NormalisableRange<float>(1.f,12.f,.1f),3.f));
 p.add(std::make_unique<B>("satOn","ZMBSaturation",true));
 p.add(std::make_unique<F>("sat","Saturation",juce::NormalisableRange<float>(0.f,1.f,.01f),.15f));
 p.add(std::make_unique<B>("chorusOn","ZMBChorus",false));
 p.add(std::make_unique<F>("chorus","Chorus",juce::NormalisableRange<float>(0.f,1.f,.01f),.2f));
 p.add(std::make_unique<B>("reverbOn","ZMBReverb",true));
 p.add(std::make_unique<F>("reverb","Reverb",juce::NormalisableRange<float>(0.f,1.f,.01f),.15f));
 p.add(std::make_unique<B>("delayOn","ZMBDelay",false));
 p.add(std::make_unique<F>("delay","Delay",juce::NormalisableRange<float>(0.f,1.f,.01f),.2f));
 p.add(std::make_unique<F>("mix","Wet",juce::NormalisableRange<float>(0.f,1.f,.01f),1.f));
 p.add(std::make_unique<F>("out","Output",juce::NormalisableRange<float>(-24.f,12.f,.1f),0.f));
 p.add(std::make_unique<B>("tuneOn","ZMBTune",false));
 p.add(std::make_unique<C>("key","Key",juce::StringArray{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},0));
 p.add(std::make_unique<C>("scale","Scale",juce::StringArray{"Chromatic","Major","Natural Minor","Harmonic Minor","Melodic Minor","Dorian","Phrygian","Lydian","Mixolydian","Locrian","Major Pentatonic","Minor Pentatonic","Blues"},2));
 p.add(std::make_unique<F>("speed","Speed",juce::NormalisableRange<float>(0.f,100.f,1.f),50.f));
 p.add(std::make_unique<F>("humanize","Humanize",juce::NormalisableRange<float>(0.f,100.f,1.f),20.f));
 p.add(std::make_unique<F>("detune","Detune",juce::NormalisableRange<float>(-100.f,100.f,.1f),0.f));
 return p;
}

void ZMBFRMAudioProcessor::prepareToPlay(double sr,int blockSize)
{
 sampleRateHz=sr;
 juce::dsp::ProcessSpec spec{sr,(juce::uint32)blockSize,(juce::uint32)getTotalNumOutputChannels()};
 for(int ch=0;ch<2;++ch){lowFilter[ch].prepare(spec);midFilter[ch].prepare(spec);highFilter[ch].prepare(spec);lowFilter[ch].reset();midFilter[ch].reset();highFilter[ch].reset();}
 compressor.prepare(spec); compressor.setAttack(8.f); compressor.setRelease(100.f);
 chorus.prepare(spec); chorus.setRate(.25f); chorus.setDepth(.2f);
 reverb.prepare(spec); reverb.reset();
 delayBuffer.setSize(2,(int)(sr*2.0)+blockSize+1); delayBuffer.clear();
 pitchShiftBuffer.setSize(2,8192); pitchShiftBuffer.clear(); delayWritePosition=0; pitchWritePosition=0;
 pitchHistory.fill(0.0f); pitchHistoryWrite=0; pitchSamplesSinceAnalysis=0; smoothedPitchRatio=1.0f; pitchPhase=0.0f;
 detectedMidiNote.store(-1.0f); correctionCents.store(0.0f);
}

float ZMBFRMAudioProcessor::detectPitchMidi() const noexcept
{
 // Autocorrelation-style normalized difference, tuned for a monophonic vocal range.
 float bestScore=0.0f; int bestLag=0;
 const int minLag=juce::jmax(2,(int)(sampleRateHz/1000.0));
 const int maxLag=juce::jmin(pitchWindowSize/2,(int)(sampleRateHz/75.0));
 for(int lag=minLag;lag<=maxLag;++lag){
  double cross=0.0, e1=0.0, e2=0.0;
  for(int i=0;i<pitchWindowSize-lag;i+=4){
   const float a=pitchHistory[(pitchHistoryWrite+i)%pitchWindowSize];
   const float b=pitchHistory[(pitchHistoryWrite+i+lag)%pitchWindowSize];
   cross+=(double)a*b; e1+=(double)a*a; e2+=(double)b*b;
  }
  const double denom=std::sqrt(e1*e2)+1.0e-12;
  const float score=(float)(cross/denom);
  if(score>bestScore){bestScore=score;bestLag=lag;}
 }
 if(bestLag==0 || bestScore<0.55f) return -1.0f;
 const float hz=(float)sampleRateHz/(float)bestLag;
 if(hz<65.0f || hz>1000.0f) return -1.0f;
 return 69.0f+12.0f*std::log2(hz/440.0f);
}

int ZMBFRMAudioProcessor::getScaleNote(int midiNote,int key,int scale) const noexcept
{
 static const bool scales[13][12] = {
  {true,true,true,true,true,true,true,true,true,true,true,true},
  {true,false,true,false,true,true,false,true,false,true,false,true},
  {true,false,true,true,false,true,false,true,true,false,true,false},
  {true,false,true,true,false,true,false,true,true,false,false,true},
  {true,false,true,false,true,true,false,true,false,true,false,true},
  {true,false,true,true,false,true,false,true,true,false,true,false},
  {true,true,false,true,true,false,true,true,false,true,true,false},
  {true,false,true,false,true,false,true,true,false,true,false,true},
  {true,false,true,false,true,true,false,true,false,true,true,false},
  {true,true,false,true,false,true,false,true,true,false,true,false},
  {true,false,true,false,true,false,false,true,false,true,false,false},
  {true,false,false,true,false,true,false,true,true,false,true,false},
  {true,false,false,true,true,false,true,true,true,false,true,false}
 };
 if(scale<0 || scale>12) scale=0;
 int best=midiNote; int bestDistance=99;
 for(int n=midiNote-6;n<=midiNote+6;++n){
  int pc=((n-key)%12+12)%12;
  if(scales[scale][pc]){int d=std::abs(n-midiNote);if(d<bestDistance){bestDistance=d;best=n;}}
 }
 return best;
}

float ZMBFRMAudioProcessor::pitchShiftSample(int channel,float input,float ratio) noexcept
{
 // Dual overlapping delay taps create a lightweight real-time pitch shifter. This is an MVP;
 // transients can sound grainy compared with a dedicated phase-vocoder/PSOLA engine.
 const int cap=pitchShiftBuffer.getNumSamples();
 auto* ring=pitchShiftBuffer.getWritePointer(channel);
 ring[pitchWritePosition]=input;
 const float depth=900.0f;
 const float phaseA=pitchPhase;
 const float phaseB=std::fmod(pitchPhase+0.5f,1.0f);
 auto readTap=[&](float phase){
  const float delay=80.0f+phase*depth;
  float pos=(float)pitchWritePosition-delay;
  while(pos<0.0f)pos+=(float)cap;
  const int i0=(int)pos%cap;const int i1=(i0+1)%cap;const float frac=pos-std::floor(pos);
  return ring[i0]+(ring[i1]-ring[i0])*frac;
 };
 const float a=readTap(phaseA), b=readTap(phaseB);
 const float wa=1.0f-std::abs(2.0f*phaseA-1.0f), wb=1.0f-wa;
 const float out=a*wa+b*wb;
 const float delta=juce::jlimit(-0.0012f,0.0012f,(1.0f-ratio)/depth);
 if(channel == getTotalNumOutputChannels() - 1) { pitchPhase+=delta; if(pitchPhase>=1.0f)pitchPhase-=1.0f; if(pitchPhase<0.0f)pitchPhase+=1.0f; }
 return out;
}

void ZMBFRMAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
 juce::ScopedNoDenormals noDenormals;
 const int n=b.getNumSamples(), chans=juce::jmin(2,b.getNumChannels());
 juce::AudioBuffer<float> dry(b);
 auto v=[this](const char* id){return state.getRawParameterValue(id)->load();};
 const bool tune=v("tuneOn")>.5f;
 const int key=(int)v("key"), scale=(int)v("scale");
 const float speed=v("speed"), human=v("humanize"), detune=v("detune");
 // Update pitch history using the input before any effects.
 for(int i=0;i<n;++i){
  float mono=0.0f;for(int ch=0;ch<chans;++ch)mono+=dry.getReadPointer(ch)[i];
  mono/=juce::jmax(1,chans);pitchHistory[pitchHistoryWrite]=mono;pitchHistoryWrite=(pitchHistoryWrite+1)%pitchWindowSize;
  if(++pitchSamplesSinceAnalysis>=512){
   pitchSamplesSinceAnalysis=0;float detected=detectPitchMidi();
   if(detected>=0.0f){
    detectedMidiNote.store(detected);
    const int nearest=getScaleNote((int)std::lround(detected),key,scale);
    float cents=((float)nearest-detected)*100.0f+detune;
    // Humanize eases correction; Speed controls response smoothing/strength.
    const float correction= cents*(1.0f-human*0.0065f)*(speed*0.01f);
    correctionCents.store(correction);
    const float target=std::pow(2.0f,correction/1200.0f);
    const float smoothing=0.02f+(speed/100.0f)*0.45f;
    smoothedPitchRatio+=smoothing*(target-smoothedPitchRatio);
   } else { detectedMidiNote.store(-1.0f); correctionCents.store(0.0f); smoothedPitchRatio+=0.01f*(1.0f-smoothedPitchRatio); }
  }
 }
 if(tune){
  // Apply pitch shift before the rest of the vocal chain. Each channel has its own ring data;
  // shared phase keeps stereo image stable.
  for(int i=0;i<n;++i){
   const float ratio=juce::jlimit(0.5f,2.0f,smoothedPitchRatio);
   for(int ch=0;ch<chans;++ch)b.getWritePointer(ch)[i]=pitchShiftSample(ch,dry.getReadPointer(ch)[i],ratio);
   pitchWritePosition=(pitchWritePosition+1)%pitchShiftBuffer.getNumSamples();
  }
 } else {
  // Keep the circular buffer primed to reduce the click when enabling tuning.
  for(int i=0;i<n;++i){for(int ch=0;ch<chans;++ch)pitchShiftBuffer.getWritePointer(ch)[pitchWritePosition]=dry.getReadPointer(ch)[i];pitchWritePosition=(pitchWritePosition+1)%pitchShiftBuffer.getNumSamples();}
 }
 if(v("eqOn")>.5f){
  auto lc=juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRateHz,160.0,.707,juce::Decibels::decibelsToGain(v("low")));
  auto mc=juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRateHz,1000.0,.8,juce::Decibels::decibelsToGain(v("mid")));
  auto hc=juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRateHz,6500.0,.707,juce::Decibels::decibelsToGain(v("high")));
  for(int ch=0;ch<chans;++ch){lowFilter[ch].coefficients=lc;midFilter[ch].coefficients=mc;highFilter[ch].coefficients=hc;auto block=juce::dsp::AudioBlock<float>(b).getSingleChannelBlock((size_t)ch);juce::dsp::ProcessContextReplacing<float> ctx(block);lowFilter[ch].process(ctx);midFilter[ch].process(ctx);highFilter[ch].process(ctx);}
 }
 if(v("compOn")>.5f){compressor.setThreshold(v("threshold"));compressor.setRatio(v("ratio"));juce::dsp::AudioBlock<float> block(b);compressor.process(juce::dsp::ProcessContextReplacing<float>(block));}
 if(v("satOn")>.5f){float drive=1.f+v("sat")*9.f,norm=std::tanh(drive);for(int ch=0;ch<chans;++ch){auto*x=b.getWritePointer(ch);for(int i=0;i<n;++i)x[i]=std::tanh(x[i]*drive)/norm;}}
 if(v("chorusOn")>.5f){chorus.setDepth(v("chorus"));juce::dsp::AudioBlock<float> block(b);chorus.process(juce::dsp::ProcessContextReplacing<float>(block));}
 if(v("reverbOn")>.5f){juce::dsp::Reverb::Parameters rp;rp.roomSize=.2f+v("reverb")*.7f;rp.damping=.45f;rp.wetLevel=v("reverb")*.45f;rp.dryLevel=0.f;rp.width=.9f;reverb.setParameters(rp);juce::dsp::AudioBlock<float> block(b);reverb.process(juce::dsp::ProcessContextReplacing<float>(block));}
 if(v("delayOn")>.5f){int cap=delayBuffer.getNumSamples();int dly=juce::jlimit(1,cap-1,(int)(sampleRateHz*(.08+v("delay")*.55)));for(int i=0;i<n;++i){int rp=(delayWritePosition-dly+cap)%cap;for(int ch=0;ch<chans;++ch){auto*x=b.getWritePointer(ch);auto*d=delayBuffer.getWritePointer(ch);float z=d[rp];d[delayWritePosition]=x[i]+z*.32f;x[i]+=z*.35f;}delayWritePosition=(delayWritePosition+1)%cap;}}
 float mix=v("mix"),gain=juce::Decibels::decibelsToGain(v("out"));
 for(int ch=0;ch<b.getNumChannels();++ch){auto*x=b.getWritePointer(ch);auto*d=dry.getReadPointer(ch);for(int i=0;i<n;++i)x[i]=(d[i]*(1.f-mix)+x[i]*mix)*gain;}
}
juce::AudioProcessorEditor* ZMBFRMAudioProcessor::createEditor(){return new ZMBFRMAudioProcessorEditor(*this);}
void ZMBFRMAudioProcessor::getStateInformation(juce::MemoryBlock& d){auto copy=state.copyState();std::unique_ptr<juce::XmlElement>x(copy.createXml());copyXmlToBinary(*x,d);}
void ZMBFRMAudioProcessor::setStateInformation(const void*d,int n){std::unique_ptr<juce::XmlElement>x(getXmlFromBinary(d,n));if(x&&x->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ZMBFRMAudioProcessor();}
