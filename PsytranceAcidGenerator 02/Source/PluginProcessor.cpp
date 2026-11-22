#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::StringArray ids {
 "cutoff","res","env","decay","drive","wave","sub","accent","tempoDiv",
 "steps","gate","delayMix","delayTime","feedback","panWidth","lfoRate",
 "lfoDepth","vocoder","master","pattern"
};

juce::AudioProcessorValueTreeState::ParameterLayout PsytranceAcidGeneratorAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto add = [&p](const char* id, const char* name, float lo, float hi, float def, float step=0.0f) {
        p.add(std::make_unique<juce::AudioParameterFloat>(id,name,juce::NormalisableRange<float>(lo,hi,step),def));
    };
    add("cutoff","Cutoff",80,16000,1400); add("res","Resonance",0.1f,0.98f,0.78f);
    add("env","Env Amount",0,1,0.7f); add("decay","Decay",0.03f,1.5f,0.22f);
    add("drive","Distortion",0,18,5); add("wave","Wave",0,2,1,1);
    add("sub","Sub",0,1,0.25f); add("accent","Accent",0,1,0.65f);
    add("tempoDiv","Step Division",0,5,2,1); add("steps","Steps",1,16,16,1);
    add("gate","Gate",0.05f,1,0.72f); add("delayMix","Pan Delay Mix",0,1,0.28f);
    add("delayTime","Delay Sync",0,6,2,1); add("feedback","Feedback",0,0.92f,0.42f);
    add("panWidth","Pan Width",0,1,0.8f); add("lfoRate","LFO Rate",0.05f,16,4);
    add("lfoDepth","LFO Depth",0,1,0.22f); add("vocoder","Step Vocoder",0,1,0.35f);
    add("master","Master",0,1,0.75f); add("pattern","Pattern",0,15,0,1);
    return p;
}

PsytranceAcidGeneratorAudioProcessor::PsytranceAcidGeneratorAudioProcessor()
: AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),
  apvts(*this,nullptr,"PARAMETERS",createParameters())
{
    stepFilter.fill(0.55f); stepResonance.fill(0.72f);
    stepGate.fill(0.82f); stepVocoder.fill(0.25f);
    loadPreset(0);
}
void PsytranceAcidGeneratorAudioProcessor::prepareToPlay(double sr,int)
{
    sampleRate=sr; filter.reset(); filter.prepare({sr,(juce::uint32) getBlockSize(),2});
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    delay.prepare({sr,(juce::uint32)getBlockSize(),2}); delay.reset();
    samplesToNextStep=0; phase=0; lfoPhase=0;
}
void PsytranceAcidGeneratorAudioProcessor::releaseResources() {}
bool PsytranceAcidGeneratorAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{ return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo(); }

void PsytranceAcidGeneratorAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    b.clear();
    auto playHead=getPlayHead();
    double bpm=120.0; bool playing=true; double ppq=0.0;
    if(playHead) if(auto pos=playHead->getPosition()) {
        if(pos->getBpm()) bpm=*pos->getBpm();
        if(pos->getIsPlaying()) playing=*pos->getIsPlaying();
        if(pos->getPpqPosition()) ppq=*pos->getPpqPosition();
    }
    const float cutoff=*apvts.getRawParameterValue("cutoff");
    const float resonance=*apvts.getRawParameterValue("res");
    const float envAmt=*apvts.getRawParameterValue("env");
    const float decay=*apvts.getRawParameterValue("decay");
    const float drive=*apvts.getRawParameterValue("drive");
    const float sub=*apvts.getRawParameterValue("sub");
    const float accent=*apvts.getRawParameterValue("accent");
    playing = pos->getIsPlaying();
    const int nSteps=(int)*apvts.getRawParameterValue("steps");
    const float gate=*apvts.getRawParameterValue("gate");
    const float delayMix=*apvts.getRawParameterValue("delayMix");
    const int delayDiv=(int)*apvts.getRawParameterValue("delayTime");
    const float feedback=*apvts.getRawParameterValue("feedback");
    const float width=*apvts.getRawParameterValue("panWidth");
    const float lfoRate=*apvts.getRawParameterValue("lfoRate");
    const float lfoDepth=*apvts.getRawParameterValue("lfoDepth");
    const float voc=*apvts.getRawParameterValue("vocoder");
    const float master=*apvts.getRawParameterValue("master");
    const int wave=(int)*apvts.getRawParameterValue("wave");
    const double stepBeats=div==0?0.25:div==1?0.5:div==2?1.0:div==3?2.0:div==4?4.0:0.125;
    const int stepSamples=juce::jmax(1,(int)(sampleRate*60.0/bpm*stepBeats));
    const float delayBeats=delayDiv==0?0.25f:delayDiv==1?0.5f:delayDiv==2?1.f:delayDiv==3?2.f:delayDiv==4?4.f:delayDiv==5?0.75f:1.5f;
    const int delaySamples=juce::jlimit(1,191999,(int)(sampleRate*60.0/bpm*delayBeats));
    filter.setResonance(resonance);
    delay.setDelay((float)delaySamples);
    for(int i=0;i<b.getNumSamples();++i) {
        if(playing && samplesToNextStep<=0) {
            step=(int)std::floor(std::fmod(std::max(0.0,ppq+i/sampleRate*bpm/60.0)/stepBeats,(double)juce::jmax(1,nSteps)));
            env=1.0f; samplesToNextStep=stepSamples;
        }
        if(playing) --samplesToNextStep;
        env*=std::exp(-1.0f/(float)(sampleRate*decay));
        const float accentStep=((step%4)==0)?accent:0.0f;
        const float freq=110.0f*std::pow(2.0f,((step%7)-2)/12.0f);
        phase+=freq/sampleRate; if(phase>=1.0) phase-=1.0;
        float osc= wave==0 ? (float)(2.0*phase-1.0) : wave==1 ? (float)std::sin(phase*juce::MathConstants<double>::twoPi) : (phase<0.5?1.0f:-1.0f);
        float s=osc + sub*(float)std::sin(phase*juce::MathConstants<double>::twoPi*0.5);
        float lfo=(float)std::sin(lfoPhase*juce::MathConstants<double>::twoPi);
        lfoPhase+=lfoRate/sampleRate; if(lfoPhase>=1.0) lfoPhase-=1.0;
        float stepGate=((i%stepSamples)<stepSamples*gate)?1.0f:0.0f;
        float vocGate=1.0f-voc*(0.5f+0.5f*std::sin((float)(step*0.785398)+lfo*2.0f));
        float fc=juce::jlimit(80.0f,18000.0f,cutoff+env*envAmt*7000.0f+accentStep*3000.0f+lfo*lfoDepth*cutoff);
        filter.setCutoffFrequency(fc);
        float x=filter.processSample(0,s*stepGate*vocGate);
        x=std::tanh(x*(1.0f+drive));
        float dry=x*master;
        float dl=delay.popSample(0), dr=delay.popSample(1);
        float pan=(float)std::sin(lfoPhase*juce::MathConstants<double>::twoPi)*width;
        delay.pushSample(0,dry+dr*feedback);
        delay.pushSample(1,dry+dl*feedback);
        b.setSample(0,i,dry*(0.5f-0.25f*pan)+dr*delayMix);
        b.setSample(1,i,dry*(0.5f+0.25f*pan)+dl*delayMix);
    }
    juce::ignoreUnused(midi,delaySamples);
}
void PsytranceAcidGeneratorAudioProcessor::setCurrentProgram(int i){loadPreset(i);}
const juce::String PsytranceAcidGeneratorAudioProcessor::getProgramName(int i){return "Acid Preset "+juce::String(i+1).paddedLeft('0',3);}
void PsytranceAcidGeneratorAudioProcessor::loadPreset(int index)
{
    currentPreset=juce::jlimit(0,99,index);
    juce::Random r(0xAC1D+currentPreset*7919);
    auto set=[this](const char* id,float v){apvts.getParameter(id)->setValueNotifyingHost(apvts.getParameter(id)->convertTo0to1(v));};
    set("cutoff",350.0f+ r.nextFloat()*5000.0f); set("res",0.55f+r.nextFloat()*0.4f);
    set("env",0.3f+r.nextFloat()*0.7f); set("decay",0.06f+r.nextFloat()*0.65f);
    set("drive",2.0f+r.nextFloat()*12.0f); set("wave",(float)(currentPreset%3));
    set("sub",r.nextFloat()*0.6f); set("accent",0.3f+r.nextFloat()*0.7f);
    set("tempoDiv",(float)(currentPreset%6)); set("steps",8.0f+(currentPreset%9));
    set("gate",0.4f+r.nextFloat()*0.58f); set("delayMix",r.nextFloat()*0.55f);
    set("delayTime",(float)(currentPreset%6)); set("feedback",0.2f+r.nextFloat()*0.6f);
    set("panWidth",r.nextFloat()); set("lfoRate",0.2f+r.nextFloat()*8.0f);
    set("lfoDepth",r.nextFloat()*0.5f); set("vocoder",r.nextFloat()*0.8f);
    set("master",0.65f); set("pattern",(float)(currentPreset%16));
    for(int i=0;i<16;++i) {
        float v=0.18f+0.78f*r.nextFloat();
        stepFilter[(size_t)i]=v;
        stepResonance[(size_t)i]=0.15f+v*0.82f;
        stepGate[(size_t)i]=(i%4==0)?1.0f:(0.2f+0.75f*r.nextFloat());
        stepVocoder[(size_t)i]=r.nextFloat();
    }
}
void PsytranceAcidGeneratorAudioProcessor::getStateInformation(juce::MemoryBlock& d){auto s=apvts.copyState();std::unique_ptr<juce::XmlElement>x(s.createXml());copyXmlToBinary(*x,d);}
void PsytranceAcidGeneratorAudioProcessor::setStateInformation(const void* data,int size){std::unique_ptr<juce::XmlElement>x(getXmlFromBinary(data,size));if(x&&x->hasTagName(apvts.state.getType()))apvts.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessorEditor* PsytranceAcidGeneratorAudioProcessor::createEditor(){return new PsytranceAcidGeneratorAudioProcessorEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new PsytranceAcidGeneratorAudioProcessor();}
