#pragma once
#include <JuceHeader.h>
#include <array>
class NeonKeysProcessor final : public juce::AudioProcessor {
public:
 NeonKeysProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout& b) const override {return b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo() || b.getMainOutputChannelSet()==juce::AudioChannelSet::mono();}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 const juce::String getName() const override {return "Neon Keys";}
 bool acceptsMidi() const override {return true;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return 3.0;}
 int getNumPrograms() override {return 1;}
 int getCurrentProgram() override {return 0;}
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override {return {};}
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState apvts;
 static juce::AudioProcessorValueTreeState::ParameterLayout makeParams();
private:
 struct Voice {int note=-1; bool held=false; double env=0,phase[3]={0,0,0},velocity=0; int stage=0; unsigned long age=0;};
 std::array<Voice,3> voices{};
 double sr=44100,filter1=0,filter2=0,lfoPhase=0,delayPos=0; unsigned long counter=0;
 std::vector<float> delayL,delayR;
 void noteOn(int,float,int); void noteOff(int);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeonKeysProcessor)
};
