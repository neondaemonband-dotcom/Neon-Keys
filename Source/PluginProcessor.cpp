#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>
NeonKeysProcessor::NeonKeysProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),apvts(*this,nullptr,"PARAMETERS",makeParams()){}
juce::AudioProcessorValueTreeState::ParameterLayout NeonKeysProcessor::makeParams(){
 std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
 auto add=[&](const char* id,float lo,float hi,float def){p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},id,juce::NormalisableRange<float>(lo,hi),def));};
 add("cutoff",50,12000,1800);add("peak",0,0.95f,0.3f);add("attack",0.001f,2,0.01f);add("decay",0.01f,3,0.4f);add("sustain",0,1,0.55f);add("release",0.01f,4,0.35f);add("filtereg",-5,5,1.8f);add("pitcheg",-12,12,0);add("lforate",0.05f,20,3);add("lfopitch",0,4,0);add("lfocut",0,4,0);add("pitch2",-24,24,0);add("pitch3",-24,24,0);add("ring",0,1,0);add("wave",0,1,0);add("mode",0,5,0);add("delaytime",0.04f,0.9f,0.27f);add("feedback",0,0.85f,0.3f);add("delaymix",0,0.65f,0.15f);add("volume",0,0.7f,0.25f);return {p.begin(),p.end()};
}
void NeonKeysProcessor::prepareToPlay(double sampleRate,int){sr=sampleRate;voices={};filter1=filter2=lfoPhase=0;delayL.assign(static_cast<size_t>(sr*2),0);delayR.assign(static_cast<size_t>(sr*2),0);delayPos=0;counter=0;}
void NeonKeysProcessor::noteOn(int note,float vel,int mode){
 int index=0;
 if(mode==0){for(int i=0;i<3;++i)if(voices[i].note<0 || voices[i].env<0.0001){index=i;break;}else if(voices[i].age<voices[index].age)index=i;}
 else index=0;
 auto& v=voices[index];v.note=note;v.held=true;v.env=0;v.stage=1;v.velocity=vel;v.age=++counter;v.phase[0]=v.phase[1]=v.phase[2]=0;
 if(mode!=0)for(int i=1;i<3;++i)voices[i]=Voice{};
}
void NeonKeysProcessor::noteOff(int note){for(auto& v:voices)if(v.note==note){v.held=false;v.stage=3;}}
void NeonKeysProcessor::processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals nd;audio.clear();if(delayL.empty())return;
 auto get=[&](const char* id){return static_cast<double>(apvts.getRawParameterValue(id)->load());};
 const double cutoff=get("cutoff"),peak=get("peak"),attack=get("attack"),decay=get("decay"),sustain=get("sustain"),release=get("release"),feg=get("filtereg"),peg=get("pitcheg"),rate=get("lforate"),lp=get("lfopitch"),lc=get("lfocut"),ring=get("ring"),wave=get("wave"),vol=get("volume"),dt=get("delaytime"),fb=get("feedback"),dm=get("delaymix");
 const int mode=juce::jlimit(0,5,static_cast<int>(std::round(get("mode"))));
 const double pitch[3]={0,get("pitch2"),get("pitch3")};
 const int size=static_cast<int>(delayL.size()),steps=juce::jlimit(1,size-1,static_cast<int>(dt*sr));
 juce::MidiBuffer::Iterator it(midi);juce::MidiMessage msg;int eventSample=0;bool has=it.getNextEvent(msg,eventSample);
 for(int s=0;s<audio.getNumSamples();++s){
  while(has && eventSample<=s){if(msg.isNoteOn())noteOn(msg.getNoteNumber(),msg.getFloatVelocity(),mode);else if(msg.isNoteOff())noteOff(msg.getNoteNumber());else if(msg.isAllNotesOff()||msg.isAllSoundOff())for(auto& v:voices){v.held=false;v.stage=3;}has=it.getNextEvent(msg,eventSample);}
  lfoPhase+=rate/sr;lfoPhase-=std::floor(lfoPhase);const double lfo=std::sin(2.0*juce::MathConstants<double>::pi*lfoPhase);
  double sum=0;
  for(auto& v:voices){if(v.note<0)continue;
   if(v.stage==1){v.env+=1.0/(std::max(0.001,attack)*sr);if(v.env>=1){v.env=1;v.stage=2;}}
   else if(v.stage==2){v.env+=(sustain-v.env)*(1-std::exp(-1.0/(std::max(0.001,decay)*sr)));}
   else if(v.stage==3){v.env*=std::exp(-1.0/(std::max(0.001,release)*sr));if(v.env<0.00005){v=Voice{};continue;}}
   double osc[3]{};
   for(int j=0;j<3;++j){double interval=pitch[j];if(mode==2)interval+=j*12;else if(mode==3)interval+=j*7;else if(mode==1)interval+=(j-1)*0.07;
    const double hz=juce::MidiMessage::getMidiNoteInHertz(v.note)*std::pow(2.0,(interval+lp*lfo+peg*v.env)/12.0);
    v.phase[j]+=juce::jlimit(0.0,0.45,hz/sr);v.phase[j]-=std::floor(v.phase[j]);osc[j]=wave<0.5?2*v.phase[j]-1:(v.phase[j]<0.5?1:-1);
   }
   double combined=(osc[0]+osc[1]+osc[2])/3.0;
   const double ringSignal=(osc[0]*osc[1]+osc[1]*osc[2])*0.5;
   const double ringAmount=(mode==4||mode==5)?1.0:ring;
   combined=(1-ringAmount)*combined+ringAmount*ringSignal;
   sum+=combined*v.env*v.velocity;
  }
  const double fc=juce::jlimit(30.0,sr*0.20,cutoff*std::pow(2.0,feg*std::min(1.0,std::abs(sum))+lc*lfo));
  const double a=1-std::exp(-2*juce::MathConstants<double>::pi*fc/sr);
  filter1+=a*(std::tanh(sum-peak*3.2*filter2)-filter1);filter2+=a*(filter1-filter2);
  const float dry=static_cast<float>(std::tanh(filter2*1.6)*vol);
  const int pos=static_cast<int>(delayPos),read=(pos-steps+size)%size;
  const float left=delayL[read],right=delayR[read];
  delayL[pos]=dry+right*static_cast<float>(fb);delayR[pos]=dry+left*static_cast<float>(fb);
  const float outL=dry*(1-static_cast<float>(dm))+left*static_cast<float>(dm),outR=dry*(1-static_cast<float>(dm))+right*static_cast<float>(dm);
  audio.setSample(0,s,outL);if(audio.getNumChannels()>1)audio.setSample(1,s,outR);
  delayPos=static_cast<double>((pos+1)%size);
 }
}
void NeonKeysProcessor::getStateInformation(juce::MemoryBlock& dest){auto xml=apvts.copyState().createXml();copyXmlToBinary(*xml,dest);}
void NeonKeysProcessor::setStateInformation(const void* data,int bytes){auto xml=getXmlFromBinary(data,bytes);if(xml&&xml->hasTagName(apvts.state.getType()))apvts.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessorEditor* NeonKeysProcessor::createEditor(){return new NeonKeysEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new NeonKeysProcessor();}
