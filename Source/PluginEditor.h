#pragma once
#include "PluginProcessor.h"
class NeonKeysEditor final : public juce::AudioProcessorEditor {
public:
 explicit NeonKeysEditor(NeonKeysProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 NeonKeysProcessor& processor;
 std::vector<std::unique_ptr<juce::Slider>> knobs;
 std::vector<std::unique_ptr<juce::Label>> labels;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> links;
};
