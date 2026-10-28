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
 p.add(std::make_unique<F>("lowFreq","Low Frequency",juce::NormalisableRange<float>(40.f,500.f,1.f,0.35f),160.f));
 p.add(std::make_unique<F>("midFreq","Mid Frequency",juce::NormalisableRange<float>(150.f,8000.f,1.f,0.35f),1000.f));
 p.add(std::make_unique<F>("midQ","Mid Q",juce::NormalisableRange<float>(0.2f,8.f,0.01f,0.5f),0.8f));
 p.add(std::make_unique<F>("highFreq","High Frequency",juce::NormalisableRange<float>(2000.f,18000.f,1.f,0.35f),6500.f));
 for (int band=4; band<=8; ++band) {
  const auto suffix=juce::String(band);
  p.add(std::make_unique<F>("band"+suffix+"Gain","EQ Band "+suffix+" Gain",juce::NormalisableRange<float>(-18.f,18.f,.1f),0.f));
  p.add(std::make_unique<F>("band"+suffix+"Freq","EQ Band "+suffix+" Frequency",juce::NormalisableRange<float>(20.f,20000.f,1.f,0.35f), 250.f * std::pow(2.0f,(float)(band-4))));
  p.add(std::make_unique<F>("band"+suffix+"Q","EQ Band "+suffix+" Q",juce::NormalisableRange<float>(0.2f,10.f,.01f,.5f),1.0f));
 }
 p.add(std::make_unique<B>("compOn","ZMBComp",true));
 p.add(std::make_unique<F>("threshold","Threshold",juce::NormalisableRange<float>(-48.f,0.f,.1f),-18.f));
 p.add(std::make_unique<F>("ratio","Ratio",juce::NormalisableRange<float>(1.f,12.f,.1f),3.f));
 p.add(std::make_unique<F>("compAttack","Compressor Attack",juce::NormalisableRange<float>(0.1f,100.f,0.1f,0.5f),8.f));
 p.add(std::make_unique<F>("compRelease","Compressor Release",juce::NormalisableRange<float>(10.f,500.f,1.f,0.5f),100.f));
 p.add(std::make_unique<B>("satOn","ZMBSaturation",true));
 p.add(std::make_unique<F>("sat","Saturation",juce::NormalisableRange<float>(0.f,1.f,.01f),.15f));
 p.add(std::make_unique<F>("satMix","Saturation Mix",juce::NormalisableRange<float>(0.f,1.f,.01f),1.f));
 p.add(std::make_unique<B>("chorusOn","ZMBChorus",false));
 p.add(std::make_unique<F>("chorus","Chorus Depth",juce::NormalisableRange<float>(0.f,1.f,.01f),.2f));
 p.add(std::make_unique<F>("chorusRate","Chorus Rate",juce::NormalisableRange<float>(0.05f,5.0f,.01f),0.25f));
 p.add(std::make_unique<B>("reverbOn","ZMBReverb",true));
 p.add(std::make_unique<F>("reverb","Reverb",juce::NormalisableRange<float>(0.f,1.f,.01f),.15f));
 p.add(std::make_unique<F>("reverbDamping","Reverb Damping",juce::NormalisableRange<float>(0.f,1.f,.01f),.45f));
 p.add(std::make_unique<B>("deesserOn","ZMBDeEsser",true));
 p.add(std::make_unique<F>("deessAmount","De-esser Amount",juce::NormalisableRange<float>(0.f,1.f,.01f),.35f));
 p.add(std::make_unique<F>("deessFreq","De-esser Frequency",juce::NormalisableRange<float>(4000.f,11000.f,10.f),6500.f));
 p.add(std::make_unique<F>("deessThreshold","De-esser Threshold",juce::NormalisableRange<float>(0.005f,0.12f,0.001f),0.035f));
 p.add(std::make_unique<B>("delayOn","ZMBDelay",false));
 p.add(std::make_unique<F>("delay","Delay Time",juce::NormalisableRange<float>(0.f,1.f,.01f),.2f));
 p.add(std::make_unique<F>("delayFeedback","Delay Feedback",juce::NormalisableRange<float>(0.f,0.85f,.01f),0.32f));
 p.add(std::make_unique<F>("delayMix","Delay Mix",juce::NormalisableRange<float>(0.f,1.f,.01f),0.35f));
 p.add(std::make_unique<F>("mix","Wet",juce::NormalisableRange<float>(0.f,1.f,.01f),1.f));
 p.add(std::make_unique<F>("out","Output",juce::NormalisableRange<float>(-24.f,12.f,.1f),0.f));
 p.add(std::make_unique<B>("tuneOn","ZMBTune",true));
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
 masterMixSmoother.reset(sr, 0.02);
 outputGainSmoother.reset(sr, 0.02);
 masterMixSmoother.setCurrentAndTargetValue(state.getRawParameterValue("mix")->load());
 outputGainSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("out")->load()));
 juce::dsp::ProcessSpec spec{sr,(juce::uint32)blockSize,(juce::uint32)getTotalNumOutputChannels()};
 for(int ch=0;ch<2;++ch){lowFilter[ch].prepare(spec);midFilter[ch].prepare(spec);highFilter[ch].prepare(spec);lowFilter[ch].reset();midFilter[ch].reset();highFilter[ch].reset(); for(int band=0;band<5;++band){extraEQFilters[band][ch].prepare(spec);extraEQFilters[band][ch].reset();}}
 compressor.prepare(spec); compressor.setAttack(8.f); compressor.setRelease(100.f);
 chorus.prepare(spec); chorus.setRate(.25f); chorus.setDepth(.2f);
 reverb.prepare(spec); reverb.reset();
 delayBuffer.setSize(2,(int)(sr*2.0)+blockSize+1); delayBuffer.clear();
 dryBuffer.setSize(2,juce::jmax(blockSize,65536)); dryBuffer.clear();
 pitchShiftBuffer.setSize(2,8192); pitchShiftBuffer.clear(); delayWritePosition=0; pitchWritePosition=0;
 pitchHistory.fill(0.0f); pitchHistoryWrite=0; pitchSamplesSinceAnalysis=0; smoothedPitchRatio=1.0f; pitchPhase=0.0f;
 detectedMidiNote.store(-1.0f); correctionCents.store(0.0f); inputPeak.store(0.0f); outputPeak.store(0.0f);
}

float ZMBFRMAudioProcessor::detectPitchMidi() const noexcept
{
 // Lightweight normalized autocorrelation with a coarse scan and local parabolic
 // refinement. The input is read oldest-to-newest from the circular history.
 // This is still a monophonic-vocal estimator, not a replacement for a dedicated
 // PSOLA/phase-vocoder tuner, but it reduces lag quantization and octave instability.
 constexpr float minConfidence = 0.58f;
 const int minLag = juce::jmax(2, (int)(sampleRateHz / 1000.0));
 const int maxLag = juce::jmin(pitchWindowSize / 2, (int)(sampleRateHz / 75.0));
 if (maxLag <= minLag + 2)
     return -1.0f;

 // Reject silence and very low-level noise before doing the lag search.
 double energy = 0.0;
 for (int i = 0; i < pitchWindowSize; i += 4)
 {
     const float x = pitchHistory[(pitchHistoryWrite + i) % pitchWindowSize];
     energy += (double)x * (double)x;
 }
 if (energy / (pitchWindowSize / 4) < 1.0e-7)
     return -1.0f;

 auto correlationAt = [this](int lag) noexcept
 {
     double cross = 0.0, e1 = 0.0, e2 = 0.0;
     const int end = pitchWindowSize - lag;
     // Decimation keeps the analysis cost bounded; local refinement is done below.
     for (int i = 0; i < end; i += 4)
     {
         const float a = pitchHistory[(pitchHistoryWrite + i) % pitchWindowSize];
         const float b = pitchHistory[(pitchHistoryWrite + i + lag) % pitchWindowSize];
         cross += (double)a * b;
         e1 += (double)a * a;
         e2 += (double)b * b;
     }
     return (float)(cross / (std::sqrt(e1 * e2) + 1.0e-15));
 };

 int bestLag = 0;
 float bestScore = -1.0f;
 // Prefer a clear local peak rather than the first broadly correlated low-frequency lag.
 for (int lag = minLag; lag <= maxLag; ++lag)
 {
     const float score = correlationAt(lag);
     if (score > bestScore && (lag == minLag || score >= correlationAt(lag - 1)))
     {
         bestScore = score;
         bestLag = lag;
     }
 }
 if (bestLag == 0 || bestScore < minConfidence)
     return -1.0f;

 // Parabolic interpolation around the correlation maximum gives sub-sample lag precision.
 const float left = correlationAt(juce::jmax(minLag, bestLag - 1));
 const float centre = correlationAt(bestLag);
 const float right = correlationAt(juce::jmin(maxLag, bestLag + 1));
 const float curvature = left - 2.0f * centre + right;
 float offset = 0.0f;
 if (std::abs(curvature) > 1.0e-6f)
     offset = juce::jlimit(-0.5f, 0.5f, 0.5f * (left - right) / curvature);
 const float refinedLag = (float)bestLag + offset;
 const float hz = (float)sampleRateHz / refinedLag;
 if (hz < 65.0f || hz > 1000.0f)
     return -1.0f;
 return 69.0f + 12.0f * std::log2(hz / 440.0f);
}

int ZMBFRMAudioProcessor::getScaleNote(int midiNote,int key,int scale) const noexcept
{
 static const bool scales[13][12] = {
  // Chromatic
  {true,true,true,true,true,true,true,true,true,true,true,true},
  // Major / Ionian
  {true,false,true,false,true,true,false,true,false,true,false,true},
  // Natural minor / Aeolian
  {true,false,true,true,false,true,false,true,true,false,true,false},
  // Harmonic minor
  {true,false,true,true,false,true,false,true,true,false,false,true},
  // Melodic minor (ascending)
  {true,false,true,true,false,true,false,true,false,true,false,true},
  // Dorian
  {true,false,true,true,false,true,false,true,false,true,true,false},
  // Phrygian
  {true,true,false,true,false,true,false,true,true,false,true,false},
  // Lydian
  {true,false,true,false,true,false,true,true,false,true,false,true},
  // Mixolydian
  {true,false,true,false,true,true,false,true,false,true,true,false},
  // Locrian
  {true,true,false,true,false,true,true,false,true,false,true,false},
  // Major pentatonic
  {true,false,true,false,true,false,false,true,false,true,false,false},
  // Minor pentatonic
  {true,false,false,true,false,true,false,true,false,false,true,false},
  // Blues
  {true,false,false,true,false,true,true,true,false,false,true,false}
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
 juce::ignoreUnused(delta);
 return out;
}

void ZMBFRMAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
 juce::ScopedNoDenormals noDenormals;
 const int n=b.getNumSamples(), chans=juce::jmin(2,b.getNumChannels());
 // A prepared scratch buffer avoids allocating memory in the real-time audio callback.
 // JUCE hosts normally respect the prepared block size; the oversized reserve covers common
 // offline/render blocks without resizing in processBlock.
 if (n > dryBuffer.getNumSamples())
 {
  // Never resize a scratch buffer on the audio thread. For unusually large offline blocks,
  // safely pass audio through rather than risk an out-of-bounds read.
  return;
 }
 float blockInputPeak = 0.0f;
 for (int ch=0; ch<chans; ++ch)
 {
  dryBuffer.copyFrom(ch, 0, b.getReadPointer(ch), n);
  blockInputPeak = juce::jmax(blockInputPeak, dryBuffer.getMagnitude(ch, 0, n));
 }
 inputPeak.store(blockInputPeak);
 auto v=[this](const char* id){return state.getRawParameterValue(id)->load();};
 const bool tune=v("tuneOn")>.5f;
 const int key=(int)v("key"), scale=(int)v("scale");
 const float speed=v("speed"), human=v("humanize"), detune=v("detune");
 // Update pitch history using the input before any effects.
 for(int i=0;i<n;++i){
  float mono=0.0f;for(int ch=0;ch<chans;++ch)mono+=dryBuffer.getReadPointer(ch)[i];
  mono/=juce::jmax(1,chans);pitchHistory[pitchHistoryWrite]=mono;pitchHistoryWrite=(pitchHistoryWrite+1)%pitchWindowSize;
  if(++pitchSamplesSinceAnalysis>=512){
   pitchSamplesSinceAnalysis=0;float detected=detectPitchMidi();
   if(detected>=0.0f){
    detectedMidiNote.store(detected);
    const int nearest=getScaleNote((int)std::lround(detected),key,scale);
    float cents=((float)nearest-detected)*100.0f+detune;
    // Humanize eases correction; Speed controls response smoothing/strength.
    const float correction= cents*(1.0f-human*0.0025f);
    correctionCents.store(correction);
    const float target=std::pow(2.0f,correction/1200.0f);
    const float smoothing=juce::jlimit(0.02f,0.95f, 0.02f + speed * 0.0092f);
    smoothedPitchRatio+=smoothing*(target-smoothedPitchRatio);
   } else {
    // Unvoiced frames must not erase the independent audio meters. Ease the tuner
    // back toward unity instead of abruptly changing its target on breaths/noise.
    detectedMidiNote.store(-1.0f);
    correctionCents.store(0.0f);
    smoothedPitchRatio += 0.035f * (1.0f - smoothedPitchRatio);
   }
  }
 }
 if(tune){
  // Apply pitch shift before the rest of the vocal chain. Each channel has its own ring data;
  // shared phase keeps stereo image stable.
  for(int i=0;i<n;++i){
   const float ratio=juce::jlimit(0.5f,2.0f,smoothedPitchRatio);
   for(int ch=0;ch<chans;++ch)b.getWritePointer(ch)[i]=pitchShiftSample(ch,dryBuffer.getReadPointer(ch)[i],ratio);
   const float phaseDelta=juce::jlimit(-0.0012f,0.0012f,(1.0f-ratio)/900.0f);
   pitchPhase += phaseDelta;
   if (pitchPhase >= 1.0f) pitchPhase -= 1.0f;
   if (pitchPhase < 0.0f) pitchPhase += 1.0f;
   pitchWritePosition=(pitchWritePosition+1)%pitchShiftBuffer.getNumSamples();
  }
 } else {
  // Keep the circular buffer primed to reduce the click when enabling tuning.
  for(int i=0;i<n;++i){for(int ch=0;ch<chans;++ch)pitchShiftBuffer.getWritePointer(ch)[pitchWritePosition]=dryBuffer.getReadPointer(ch)[i];pitchWritePosition=(pitchWritePosition+1)%pitchShiftBuffer.getNumSamples();}
 }
 if(v("eqOn")>.5f){
  const float lowGain=v("low"), midGain=v("mid"), highGain=v("high");
  const float nyquist = (float)sampleRateHz * 0.49f;
  const float lowFreq=juce::jlimit(20.0f, nyquist, v("lowFreq"));
  const float midFreq=juce::jlimit(20.0f, nyquist, v("midFreq"));
  const float midQ=v("midQ");
  const float highFreq=juce::jlimit(20.0f, nyquist, v("highFreq"));
  // Rebuild coefficient objects only when a control actually changes. This avoids
  // repeated shared-pointer allocations on every audio callback.
  if (lowGain!=lastLowGain || lowFreq!=lastLowFreq) {
   auto c=juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRateHz,lowFreq,.707f,juce::Decibels::decibelsToGain(lowGain));
   for(int ch=0;ch<2;++ch) lowFilter[ch].coefficients=c;
   lastLowGain=lowGain; lastLowFreq=lowFreq;
  }
  if (midGain!=lastMidGain || midFreq!=lastMidFreq || midQ!=lastMidQ) {
   auto c=juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRateHz,midFreq,midQ,juce::Decibels::decibelsToGain(midGain));
   for(int ch=0;ch<2;++ch) midFilter[ch].coefficients=c;
   lastMidGain=midGain; lastMidFreq=midFreq; lastMidQ=midQ;
  }
  if (highGain!=lastHighGain || highFreq!=lastHighFreq) {
   auto c=juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRateHz,highFreq,.707f,juce::Decibels::decibelsToGain(highGain));
   for(int ch=0;ch<2;++ch) highFilter[ch].coefficients=c;
   lastHighGain=highGain; lastHighFreq=highFreq;
  }
  for(int ch=0;ch<chans;++ch){auto block=juce::dsp::AudioBlock<float>(b).getSingleChannelBlock((size_t)ch);juce::dsp::ProcessContextReplacing<float> ctx(block);lowFilter[ch].process(ctx);midFilter[ch].process(ctx);highFilter[ch].process(ctx);}
  for(int band=0;band<5;++band){
   const auto suffix=juce::String(band+4);
   const float gain=v(("band"+suffix+"Gain").toRawUTF8());
   const float freq=juce::jlimit(20.0f,nyquist,v(("band"+suffix+"Freq").toRawUTF8()));
   const float q=v(("band"+suffix+"Q").toRawUTF8());
   if(gain!=lastExtraGain[band] || freq!=lastExtraFreq[band] || q!=lastExtraQ[band]){
    auto c=juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRateHz,freq,q,juce::Decibels::decibelsToGain(gain));
    for(int ch=0;ch<2;++ch) extraEQFilters[band][ch].coefficients=c;
    lastExtraGain[band]=gain;lastExtraFreq[band]=freq;lastExtraQ[band]=q;
   }
   for(int ch=0;ch<chans;++ch){auto block=juce::dsp::AudioBlock<float>(b).getSingleChannelBlock((size_t)ch);juce::dsp::ProcessContextReplacing<float> ctx(block);extraEQFilters[band][ch].process(ctx);}
  }
 }
 if(v("compOn")>.5f){compressor.setThreshold(v("threshold"));compressor.setRatio(v("ratio"));compressor.setAttack(v("compAttack"));compressor.setRelease(v("compRelease"));juce::dsp::AudioBlock<float> block(b);compressor.process(juce::dsp::ProcessContextReplacing<float>(block));}
 if(v("satOn")>.5f){float drive=1.f+v("sat")*9.f,norm=std::tanh(drive),satMix=juce::jlimit(0.0f,1.0f,v("satMix"));for(int ch=0;ch<chans;++ch){auto*x=b.getWritePointer(ch);for(int i=0;i<n;++i){const float dry=x[i];const float wet=std::tanh(dry*drive)/juce::jmax(0.001f,norm);x[i]=dry+(wet-dry)*satMix;}}}
 if(v("deesserOn")>.5f){
  const float amount=v("deessAmount");
  const float cutoff=juce::jlimit(1000.0f,(float)sampleRateHz*0.48f,v("deessFreq"));
  const float hpCoeff=std::exp(-juce::MathConstants<float>::twoPi*cutoff/(float)sampleRateHz);
  const float attackCoeff=std::exp(-1.0f/(0.0015f*(float)sampleRateHz));
  const float releaseCoeff=std::exp(-1.0f/(0.060f*(float)sampleRateHz));
  for(int ch=0;ch<chans;++ch){auto*x=b.getWritePointer(ch);for(int i=0;i<n;++i){
   const float low=hpCoeff*deesserLowState[ch]+(1.0f-hpCoeff)*x[i];
   const float high=x[i]-low;deesserLowState[ch]=low;
   const float mag=std::abs(high);const float coeff=mag>deesserEnvelope[ch]?attackCoeff:releaseCoeff;
   deesserEnvelope[ch]=coeff*deesserEnvelope[ch]+(1.0f-coeff)*mag;
   const float threshold=v("deessThreshold");const float reduction=1.0f/(1.0f+amount*12.0f*juce::jmax(0.0f,deesserEnvelope[ch]-threshold));
   x[i]=low+high*reduction;
  }}
 }
 if(v("chorusOn")>.5f){chorus.setRate(v("chorusRate"));chorus.setDepth(v("chorus"));juce::dsp::AudioBlock<float> block(b);chorus.process(juce::dsp::ProcessContextReplacing<float>(block));}
 if(v("reverbOn")>.5f){juce::dsp::Reverb::Parameters rp;rp.roomSize=.2f+v("reverb")*.7f;rp.damping=v("reverbDamping");rp.wetLevel=v("reverb")*.45f;rp.dryLevel=1.0f;rp.width=.9f;reverb.setParameters(rp);juce::dsp::AudioBlock<float> block(b);reverb.process(juce::dsp::ProcessContextReplacing<float>(block));}
 if(v("delayOn")>.5f){
  const int cap=delayBuffer.getNumSamples();
  const int dly=juce::jlimit(1,cap-1,(int)(sampleRateHz*(.08+v("delay")*.55)));
  const float feedback=juce::jlimit(0.0f,0.85f,v("delayFeedback"));
  const float delayMix=juce::jlimit(0.0f,1.0f,v("delayMix"));
  for(int i=0;i<n;++i){
   const int rp=(delayWritePosition-dly+cap)%cap;
   for(int ch=0;ch<chans;++ch){auto*x=b.getWritePointer(ch);auto*d=delayBuffer.getWritePointer(ch);const float z=d[rp];d[delayWritePosition]=x[i]+z*feedback;x[i]+=z*delayMix;}
   delayWritePosition=(delayWritePosition+1)%cap;
  }
 }
 // Smooth master controls sample-by-sample to avoid clicks when automating Wet/Dry or Output.
 masterMixSmoother.setTargetValue(juce::jlimit(0.0f, 1.0f, v("mix")));
 outputGainSmoother.setTargetValue(juce::Decibels::decibelsToGain(v("out")));
 const int processChannels = juce::jmin(b.getNumChannels(), dryBuffer.getNumChannels());
 for(int i=0; i<n; ++i)
 {
  const float mix = masterMixSmoother.getNextValue();
  const float gain = outputGainSmoother.getNextValue();
  for(int ch=0; ch<processChannels; ++ch)
  {
   auto* x = b.getWritePointer(ch);
   const auto* d = dryBuffer.getReadPointer(ch);
   x[i] = (d[i] * (1.0f - mix) + x[i] * mix) * gain;
  }
 }
 // Clear any host-provided output channels that have no matching input/scratch channel.
 for(int ch=dryBuffer.getNumChannels(); ch<b.getNumChannels(); ++ch)
  b.clear(ch, 0, n);
 float blockOutputPeak = 0.0f;
 for (int ch=0; ch<processChannels; ++ch)
  blockOutputPeak = juce::jmax(blockOutputPeak, b.getMagnitude(ch, 0, n));
 outputPeak.store(blockOutputPeak);
}
juce::AudioProcessorEditor* ZMBFRMAudioProcessor::createEditor(){return new ZMBFRMAudioProcessorEditor(*this);}
void ZMBFRMAudioProcessor::getStateInformation(juce::MemoryBlock& d)
{
    const auto copy = state.copyState();
    if (auto xml = copy.createXml())
        copyXmlToBinary(*xml, d);
}

void ZMBFRMAudioProcessor::setStateInformation(const void* data, int size)
{
    if (data == nullptr || size <= 0)
        return;

    if (auto xml = getXmlFromBinary(data, size))
    {
        if (xml->hasTagName(state.state.getType()))
        {
            const auto restored = juce::ValueTree::fromXml(*xml);
            if (restored.isValid())
                state.replaceState(restored);
        }
    }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ZMBFRMAudioProcessor();}
