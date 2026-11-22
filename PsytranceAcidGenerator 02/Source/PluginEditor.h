#pragma once
#include "PluginProcessor.h"
class PsytranceAcidGeneratorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PsytranceAcidGeneratorAudioProcessorEditor(PsytranceAcidGeneratorAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void drawAt(juce::Point<int>);
private:
    using Slider=juce::Slider;
    PsytranceAcidGeneratorAudioProcessor& processor;
    juce::ComboBox presetBox;
    juce::Label title;
    std::array<std::unique_ptr<Slider>,20> knobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,20> attachments;
    std::array<juce::Label,20> labels;
    juce::TextButton randomButton{"Randomize"}, syncButton{"DAW SYNC"};
    const std::array<const char*,20> paramIDs {{
      "cutoff","res","env","decay","drive","wave","sub","accent","tempoDiv","steps",
      "gate","delayMix","delayTime","feedback","panWidth","lfoRate","lfoDepth","vocoder","master","pattern"
    }};
    const std::array<const char*,20> names {{
      "CUTOFF","RESONANCE","ENV AMT","DECAY","DISTORTION","WAVE","SUB","ACCENT","STEP DIV","STEPS",
      "GATE","PAN DELAY","DELAY SYNC","FEEDBACK","PAN WIDTH","LFO RATE","LFO DEPTH","STEP VOCODER","MASTER","PATTERN"
    }};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PsytranceAcidGeneratorAudioProcessorEditor)
};
