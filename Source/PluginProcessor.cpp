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
 // ZMBTune controls are exposed for UI/automation; pitch correction DSP is not yet implemented.
 p.add(std::make_unique<B>("tuneOn","ZMBTune",false));
 p.add(std::make_unique<C>("key","Key",juce::StringArray{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},0));
 p.add(std::make_unique<C>("scale","Scale",juce::StringArray{"Chromatic","Major","Minor"},2));
 p.add(std::make_unique<F>("speed","Speed",juce::NormalisableRange<float>(0.f,100.f,1.f),50.f));
 p.add(std::make_unique<F>("humanize","Humanize",juce::NormalisableRange<float>(0.f,100.f,1.f),20.f));
 return p;
}

void ZMBFRMAudioProcessor::prepareToPlay(double sr,int blockSize)
{
 sampleRateHz=sr;
 juce::dsp::ProcessSpec spec{sr,(juce::uint32)blockSize,(juce::uint32)getTotalNumOutputChannels()};
 for(int ch=0;ch<2;++ch){lowFilter[ch].prepare(spec);midFilter[ch].prepare(spec);highFilter[ch].prepare(spec);lowFilter[ch].reset();midFilter[ch].reset();highFilter[ch].reset();}
 compressor.prepare(spec); compressor.setAttack(8.f); compressor.setRelease(100.f);
 chorus.prepare(spec); chorus.setRate(.25f); chorus.setDepth(.2f);
 reverb.setSampleRate(sr);
 delayBuffer.setSize(2,(int)(sr*2.0)+blockSize+1); delayBuffer.clear(); delayWritePosition=0;
}

void ZMBFRMAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
 juce::ScopedNoDenormals noDenormals;
 const int n=b.getNumSamples(), chans=juce::jmin(2,b.getNumChannels());
 static_assert(sizeof(float) == 4, "Expected 32-bit float audio samples");
 juce::AudioBuffer<float> dry(b);
 auto v=[this](const char* id){return state.getRawParameterValue(id)->load();};
 // ZMBTune controls are saved and automatable; no pitch-shifting is applied in this build.
 if(v("eqOn")>.5f){
  auto lc=juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRateHz,160.0,.707,juce::Decibels::decibelsToGain(v("low")));
  auto mc=juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRateHz,1000.0,.8,juce::Decibels::decibelsToGain(v("mid")));
  auto hc=juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRateHz,6500.0,.707,juce::Decibels::decibelsToGain(v("high")));
  for(int ch=0;ch<chans;++ch){lowFilter[ch].coefficients=lc;midFilter[ch].coefficients=mc;highFilter[ch].coefficients=hc;
   auto block=juce::dsp::AudioBlock<float>(b).getSingleChannelBlock((size_t)ch);juce::dsp::ProcessContextReplacing<float> ctx(block);
   lowFilter[ch].process(ctx);midFilter[ch].process(ctx);highFilter[ch].process(ctx);}
 }
 if(v("compOn")>.5f){compressor.setThreshold(v("threshold"));compressor.setRatio(v("ratio"));juce::dsp::AudioBlock<float> block(b);compressor.process(juce::dsp::ProcessContextReplacing<float>(block));}
 if(v("satOn")>.5f){float drive=1.f+v("sat")*9.f, norm=std::tanh(drive);for(int ch=0;ch<chans;++ch){auto*x=b.getWritePointer(ch);for(int i=0;i<n;++i)x[i]=std::tanh(x[i]*drive)/norm;}}
 if(v("chorusOn")>.5f){chorus.setDepth(v("chorus"));juce::dsp::AudioBlock<float> block(b);chorus.process(juce::dsp::ProcessContextReplacing<float>(block));}
 if(v("reverbOn")>.5f && chans==2){juce::dsp::Reverb::Parameters rp;rp.roomSize=.2f+v("reverb")*.7f;rp.damping=.45f;rp.wetLevel=v("reverb")*.45f;rp.dryLevel=0.f;rp.width=.9f;reverb.setParameters(rp);reverb.processStereo(b.getWritePointer(0),b.getWritePointer(1),n);}
 if(v("delayOn")>.5f){int cap=delayBuffer.getNumSamples();int dly=juce::jlimit(1,cap-1,(int)(sampleRateHz*(.08+v("delay")*.55)));for(int i=0;i<n;++i){int rp=(delayWritePosition-dly+cap)%cap;for(int ch=0;ch<chans;++ch){auto*x=b.getWritePointer(ch);auto*d=delayBuffer.getWritePointer(ch);float z=d[rp];d[delayWritePosition]=x[i]+z*.32f;x[i]+=z*.35f;}delayWritePosition=(delayWritePosition+1)%cap;}}
 float mix=v("mix"),gain=juce::Decibels::decibelsToGain(v("out"));
 for(int ch=0;ch<b.getNumChannels();++ch){auto*x=b.getWritePointer(ch);auto*d=dry.getReadPointer(ch);for(int i=0;i<n;++i)x[i]=(d[i]*(1.f-mix)+x[i]*mix)*gain;}
}
juce::AudioProcessorEditor* ZMBFRMAudioProcessor::createEditor(){return new ZMBFRMAudioProcessorEditor(*this);}
void ZMBFRMAudioProcessor::getStateInformation(juce::MemoryBlock& d){auto copy=state.copyState();std::unique_ptr<juce::XmlElement>x(copy.createXml());copyXmlToBinary(*x,d);}
void ZMBFRMAudioProcessor::setStateInformation(const void*d,int n){std::unique_ptr<juce::XmlElement>x(getXmlFromBinary(d,n));if(x&&x->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ZMBFRMAudioProcessor();}
