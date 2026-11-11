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
 // Hosts should provide valid values, but guard the DSP setup against
 // malformed/offline configurations before calculating buffer sizes.
 if (!std::isfinite(sr) || sr < 8000.0)
  sr = 44100.0;
 blockSize = juce::jmax(1, blockSize);
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
 pitchShiftBuffer.setSize(2,(int)(sr*0.065)+blockSize+1); pitchShiftBuffer.clear();
 pitchRatioBuffer.setSize(1,juce::jmax(blockSize,65536)); pitchRatioBuffer.clear(); delayWritePosition=0; pitchWritePosition=0;
 pitchHistory.fill(0.0f); pitchHistoryWrite=0; pitchHistorySamples=0; pitchSamplesSinceAnalysis=0; pitchAnalysisHop=juce::jlimit(256,2048,(int)std::lround(sr*0.010)); smoothedPitchRatio=1.0f; targetPitchRatio=1.0f; smoothedDetuneCents=0.0f; pitchPhase=0.0f; trackedPitchMidi=-1.0f; pendingPitchMidi=-1.0f; pendingPitchFrames=0; pitchUnvoicedFrames=0; trackedScaleNote=-1;
 detectedMidiNote.store(-1.0f); correctionCents.store(0.0f); inputPeak.store(0.0f); outputPeak.store(0.0f);
}

float ZMBFRMAudioProcessor::detectPitchMidi() const noexcept
{
    // YIN/CMND detector. A sample-rate-scaled active window preserves enough
    // low-frequency periods at 88.2/96/192 kHz without forcing a long window
    // at ordinary rates. Storage is fixed at prepare time; no audio-thread
    // allocation is performed.
    constexpr float yinThreshold = 0.18f;
    constexpr float fallbackThreshold = 0.34f;
    constexpr float minimumRms = 0.00020f;
    constexpr int decimation = 2;
    constexpr int analysisCapacity = pitchWindowSize / decimation;

    const int activeWindow = juce::jlimit(2048, pitchWindowSize,
        (int) std::ceil(sampleRateHz * 0.045));
    const int analysisSize = activeWindow / decimation;
    if (pitchHistorySamples < activeWindow)
        return -1.0f;
    const int oldest = (pitchHistoryWrite + pitchWindowSize - activeWindow) % pitchWindowSize;

    const int minLag = juce::jmax(2, (int) std::floor(sampleRateHz / 1100.0));
    const int maxLag = (int) std::ceil(sampleRateHz / 55.0);
    const int minDecimatedLag = juce::jmax(2, minLag / decimation);
    const int maxDecimatedLag = juce::jmin(analysisSize / 2, maxLag / decimation);
    if (maxDecimatedLag <= minDecimatedLag + 2)
        return -1.0f;

    double mean = 0.0;
    for (int i = 0; i < activeWindow; ++i)
        mean += pitchHistory[(oldest + i) % pitchWindowSize];
    mean /= activeWindow;

    std::array<float, analysisCapacity> x {};
    double energy = 0.0;
    for (int i = 0; i < analysisSize; ++i)
    {
        const int centre = (oldest + i * decimation) % pitchWindowSize;
        const int prev = (centre + pitchWindowSize - 1) % pitchWindowSize;
        const int next = (centre + 1) % pitchWindowSize;
        const float filtered = 0.25f * pitchHistory[(size_t) prev]
                             + 0.5f * pitchHistory[(size_t) centre]
                             + 0.25f * pitchHistory[(size_t) next]
                             - (float) mean;
        const float window = 0.5f - 0.5f * std::cos(
            juce::MathConstants<float>::twoPi * (float) i / (float) (analysisSize - 1));
        x[(size_t) i] = filtered * window;
        energy += (double) filtered * filtered;
    }

    const float rms = (float) std::sqrt(energy / analysisSize);
    if (!std::isfinite(rms) || rms < minimumRms)
        return -1.0f;

    std::array<float, analysisCapacity / 2 + 1> cmnd {};
    double runningSum = 0.0;
    for (int tau = 1; tau <= maxDecimatedLag; ++tau)
    {
        double sum = 0.0;
        const int limit = analysisSize - tau;
        for (int i = 0; i < limit; i += 2)
        {
            const float d = x[(size_t) i] - x[(size_t) (i + tau)];
            sum += (double) d * d;
        }
        runningSum += sum;
        cmnd[(size_t) tau] = runningSum > 1.0e-15
            ? (float) (sum * tau / runningSum) : 1.0f;
    }

    int bestTau = 0;
    for (int tau = minDecimatedLag; tau <= maxDecimatedLag; ++tau)
    {
        if (cmnd[(size_t) tau] < yinThreshold)
        {
            while (tau + 1 <= maxDecimatedLag
                   && cmnd[(size_t) (tau + 1)] < cmnd[(size_t) tau])
                ++tau;
            bestTau = tau;
            break;
        }
    }

    if (bestTau == 0)
    {
        float best = 1.0f;
        for (int tau = minDecimatedLag; tau <= maxDecimatedLag; ++tau)
        {
            if (cmnd[(size_t) tau] < best)
            {
                best = cmnd[(size_t) tau];
                bestTau = tau;
            }
        }
        if (bestTau == 0 || cmnd[(size_t) bestTau] > fallbackThreshold)
            return -1.0f;
    }

    const float left = cmnd[(size_t) juce::jmax(1, bestTau - 1)];
    const float centre = cmnd[(size_t) bestTau];
    const float right = cmnd[(size_t) juce::jmin(maxDecimatedLag, bestTau + 1)];
    const float curvature = left - 2.0f * centre + right;
    float offset = 0.0f;
    if (std::abs(curvature) > 1.0e-7f)
        offset = juce::jlimit(-0.5f, 0.5f, 0.5f * (left - right) / curvature);

    const float period = ((float) bestTau + offset) * (float) decimation;
    const float hz = (float) sampleRateHz / period;
    if (!std::isfinite(hz) || hz < 55.0f || hz > 1100.0f)
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
 // Two continuously overlapping delay heads. A Hann crossfade avoids the
 // level ripple of a triangular fade, while cubic interpolation reduces
 // high-frequency aliasing and the grainy edge of linear delay reads.
 const int cap=pitchShiftBuffer.getNumSamples();
 auto* ring=pitchShiftBuffer.getWritePointer(channel);
 ring[pitchWritePosition]=input;
 const float depth=juce::jmin((float)(cap-128),(float)sampleRateHz*0.040f);
 const float phaseA=pitchPhase;
 const float phaseB=phaseA<0.5f ? phaseA+0.5f : phaseA-0.5f;

 auto readTap=[&](float phase) noexcept
 {
  const float delay=64.0f+phase*depth;
  float pos=(float)pitchWritePosition-delay;
  while(pos<0.0f) pos+=(float)cap;
  while(pos>=(float)cap) pos-=(float)cap;

  const int i1=(int)pos;
  const int i0=(i1+cap-1)%cap;
  const int i2=(i1+1)%cap;
  const int i3=(i1+2)%cap;
  const float t=pos-(float)i1;
  const float y0=ring[i0], y1=ring[i1], y2=ring[i2], y3=ring[i3];

  // Catmull-Rom cubic interpolation; bounded to avoid overshoot on sharp
  // vocal transients while retaining a smoother slope than linear reads.
  const float a0=-0.5f*y0+1.5f*y1-1.5f*y2+0.5f*y3;
  const float a1=y0-2.5f*y1+2.0f*y2-0.5f*y3;
  const float a2=-0.5f*y0+0.5f*y2;
  const float cubic=((a0*t+a1)*t+a2)*t+y1;
  const float linear=y1+(y2-y1)*t;
  return juce::jlimit(juce::jmin(y1,y2),juce::jmax(y1,y2),cubic*0.85f+linear*0.15f);
 };

 const float a=readTap(phaseA);
 const float b=readTap(phaseB);
 const float wa=0.5f-0.5f*std::cos(juce::MathConstants<float>::twoPi*phaseA);
 const float wb=1.0f-wa;
 juce::ignoreUnused(ratio); // Phase increment is applied once per sample in processBlock().
 return a*wa+b*wb;
}

void ZMBFRMAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
 juce::ScopedNoDenormals noDenormals;
 const int n=b.getNumSamples(), chans=juce::jmin(2,b.getNumChannels());
 // A prepared scratch buffer avoids allocating memory in the real-time audio callback.
 // JUCE hosts normally respect the prepared block size; the oversized reserve covers common
 // offline/render blocks without resizing in processBlock.
 if (n > dryBuffer.getNumSamples() || n > pitchRatioBuffer.getNumSamples())
 {
  // Never resize a scratch buffer on the audio thread. For unusually large offline blocks,
  // safely pass audio through rather than risk an out-of-bounds read.
  return;
 }
 float blockInputPeak = 0.0f;
 for (int ch=0; ch<chans; ++ch)
 {
  dryBuffer.copyFrom(ch, 0, b.getReadPointer(ch), n);
  auto* cleanInput = dryBuffer.getWritePointer(ch);
  for (int i=0; i<n; ++i)
  {
   // Prevent a single invalid host/input sample from poisoning pitch history,
   // delay lines, filter states, and the complete output block.
   if (!std::isfinite(cleanInput[i]))
    cleanInput[i] = 0.0f;
  }
  blockInputPeak = juce::jmax(blockInputPeak, dryBuffer.getMagnitude(ch, 0, n));
 }
 inputPeak.store(blockInputPeak);
 auto v=[this](const char* id){return state.getRawParameterValue(id)->load();};
 const bool tune=v("tuneOn")>.5f;
 const int key=(int)v("key"), scale=(int)v("scale");
 const float speed=v("speed"), human=v("humanize"), detune=v("detune");
 // Update pitch history using the input before any effects.
 // Prefer a dominant channel if stereo phase cancellation would erase the
 // vocal fundamental in the mono detector feed.
 int pitchChannel=-1;
 if(chans>1 && n>0){
  double channelEnergy[2]={0.0,0.0}, monoEnergy=0.0;
  for(int i=0;i<n;++i){
   const float l=dryBuffer.getReadPointer(0)[i], r=dryBuffer.getReadPointer(1)[i];
   channelEnergy[0]+=(double)l*l; channelEnergy[1]+=(double)r*r;
   const float m=0.5f*(l+r); monoEnergy+=(double)m*m;
  }
  const double strongest=juce::jmax(channelEnergy[0],channelEnergy[1]);
  if(strongest>1.0e-12 && monoEnergy<strongest*0.25)
   pitchChannel=channelEnergy[0]>=channelEnergy[1]?0:1;
 }
 const float speedNorm=juce::jlimit(0.0f,1.0f,speed*0.01f);
 const float humanNorm=juce::jlimit(0.0f,1.0f,human*0.01f);
 const float responseSeconds=0.008f+0.442f*std::pow(1.0f-speedNorm,2.0f);
 const float humanizedSeconds=responseSeconds*(1.0f+4.0f*humanNorm);
 const float correctionAlpha=1.0f-std::exp(-1.0f/((float)sampleRateHz*juce::jmax(0.001f,humanizedSeconds)));
 const float releaseSeconds=0.14f+0.46f*humanNorm;
 const float releaseAlpha=1.0f-std::exp(-1.0f/((float)sampleRateHz*releaseSeconds));
 const int releaseGrace=juce::jmax(2,(int)std::ceil(0.050f*(float)sampleRateHz/(float)pitchAnalysisHop));
 for(int i=0;i<n;++i){
  float mono=0.0f;
  if(pitchChannel>=0) mono=dryBuffer.getReadPointer(pitchChannel)[i];
  else {for(int ch=0;ch<chans;++ch)mono+=dryBuffer.getReadPointer(ch)[i];mono/=juce::jmax(1,chans);}
  pitchHistory[pitchHistoryWrite]=mono;pitchHistoryWrite=(pitchHistoryWrite+1)%pitchWindowSize;
  pitchHistorySamples=juce::jmin(pitchWindowSize,pitchHistorySamples+1);
  if(++pitchSamplesSinceAnalysis>=pitchAnalysisHop){
   pitchSamplesSinceAnalysis=0;float detected=detectPitchMidi();
   if(detected>=0.0f){
    pitchUnvoicedFrames=0;
    // Accept a stable pitch estimate before using it for scale quantisation.
    // Large leaps, including octave changes, need repeated evidence;
    // ordinary note motion is smoothed exactly once.
    float candidate=detected;
    bool acceptCandidate=true;

    if(trackedPitchMidi>=0.0f){
     const float jump=candidate-trackedPitchMidi;
     if(std::abs(jump)>7.0f){
      // Do not automatically fold octave-sized changes back to the old
      // note: that would suppress genuine octave leaps. Require a stable
      // repeated observation for every large transition instead.
      if(pendingPitchMidi>=0.0f && std::abs(candidate-pendingPitchMidi)<1.5f)
       ++pendingPitchFrames;
      else {
       pendingPitchMidi=candidate;
       pendingPitchFrames=1;
      }

      // Express confirmation in time, not in an arbitrary number of analysis
      // hops, so the tracker behaves consistently at different sample rates.
      const float confirmSeconds=std::abs(std::abs(jump)-12.0f)<2.5f ? 0.045f : 0.032f;
      const int requiredFrames=juce::jlimit(2,8,(int)std::ceil(confirmSeconds*(float)sampleRateHz/(float)pitchAnalysisHop));
      if(pendingPitchFrames<requiredFrames)
       acceptCandidate=false;
      else {
       // Confirmed note change; the pitch-ratio smoother performs the
       // audible transition instead of abruptly switching the read ratio.
       trackedPitchMidi=candidate;
       pendingPitchMidi=-1.0f;
       pendingPitchFrames=0;
      }
     } else {
      trackedPitchMidi+=0.35f*(candidate-trackedPitchMidi);
      pendingPitchMidi=-1.0f;
      pendingPitchFrames=0;
     }
    } else {
     trackedPitchMidi=candidate;
     pendingPitchMidi=-1.0f;
     pendingPitchFrames=0;
    }

    if(!acceptCandidate)
     candidate=trackedPitchMidi;

    // All downstream decisions use the accepted tracker state, never the
    // unverified raw detector result.
    candidate=trackedPitchMidi;
    detectedMidiNote.store(candidate);
    int nearest=getScaleNote((int)std::lround(candidate),key,scale);

    // A quarter-semitone boundary margin prevents vibrato from toggling
    // the quantised destination between adjacent scale notes.
    if(trackedScaleNote>=0 && getScaleNote(trackedScaleNote,key,scale)==trackedScaleNote){
     if(nearest!=trackedScaleNote
        && std::abs(candidate-(float)nearest)+0.25f>=std::abs(candidate-(float)trackedScaleNote))
      nearest=trackedScaleNote;
    }
    trackedScaleNote=nearest;

    // Detune is a fine-pitch offset, independent of retune speed and
    // Humanize. Smooth host automation over 20 ms so changing calibration
    // cannot create a sudden pitch step, even with very fast correction.
    const float detuneTimeSeconds=0.020f;
    const float detuneAlpha=1.0f-std::exp(-((float)pitchAnalysisHop/(float)sampleRateHz)/detuneTimeSeconds);
    smoothedDetuneCents += juce::jlimit(0.001f,0.95f,detuneAlpha)*(juce::jlimit(-100.0f,100.0f,detune)-smoothedDetuneCents);
    const float cents=((float)nearest-candidate)*100.0f+smoothedDetuneCents;
    correctionCents.store(cents);
    const float target=std::pow(2.0f,cents/1200.0f);

    targetPitchRatio=target;
   } else {
    ++pitchUnvoicedFrames;
    detectedMidiNote.store(-1.0f);
    correctionCents.store(0.0f);
    // Brief unvoiced gaps are common between voiced frames and consonants.
    // Preserve the pending transition through short gaps; only clear it and
    // relax toward unity after roughly 50 ms without a usable pitch estimate.
    const int unvoicedGraceFrames=juce::jmax(2,(int)std::ceil(0.050f*(float)sampleRateHz/(float)pitchAnalysisHop));
    if(pitchUnvoicedFrames>unvoicedGraceFrames){
     pendingPitchMidi=-1.0f;
     pendingPitchFrames=0;
     // Switch the target to unity; the per-sample smoother below performs
     // the actual release without 10 ms control-rate stair-steps.
     targetPitchRatio=1.0f;
    }
   }
  }
  // Preserve the ratio at the exact sample where it became valid. Applying
  // only the final block ratio would retroactively retune earlier samples
  // and make tracking depend on the host's audio block size.
  // Smooth the pitch ratio at audio rate. The target is refreshed by the
  // detector at its analysis cadence, but this one-pole runs every sample so
  // phase velocity changes continuously instead of stepping every 10 ms.
  const float alpha=pitchUnvoicedFrames>releaseGrace?releaseAlpha:correctionAlpha;
  smoothedPitchRatio+=alpha*(targetPitchRatio-smoothedPitchRatio);
  pitchRatioBuffer.getWritePointer(0)[i]=smoothedPitchRatio;
 }
 if(tune){
  // Apply pitch shift before the rest of the vocal chain. Each channel has its own ring data;
  // shared phase keeps stereo image stable.
  for(int i=0;i<n;++i){
   const float ratio=juce::jlimit(0.5f,2.0f,pitchRatioBuffer.getReadPointer(0)[i]);
   for(int ch=0;ch<chans;++ch)b.getWritePointer(ch)[i]=pitchShiftSample(ch,dryBuffer.getReadPointer(ch)[i],ratio);
   const float grainDepth=juce::jmin((float)(pitchShiftBuffer.getNumSamples()-128),(float)sampleRateHz*0.040f);
   const float phaseDelta=(1.0f-ratio)/juce::jmax(1.0f,grainDepth);
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
   // Final containment: invalid intermediate DSP state must never escape
   // the plug-in as NaN/Inf and poison the host's downstream signal chain.
   if (!std::isfinite(x[i]))
    x[i] = 0.0f;
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
