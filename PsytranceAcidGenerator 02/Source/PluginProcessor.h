#pragma once
#include <JuceHeader.h>

class PsytranceAcidGeneratorAudioProcessor : public juce::AudioProcessor
{
public:
    PsytranceAcidGeneratorAudioProcessor();
    ~PsytranceAcidGeneratorAudioProcessor() override = default;
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Psytrance Acid Generator"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }
    int getNumPrograms() override { return 100; }
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    void loadPreset(int index);
    void setDrawStep(int index, float value);
    std::array<float,16> stepFilter, stepResonance, stepGate, stepVocoder;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
private:
    int currentPreset = 0, step = 0;
    double sampleRate = 44100.0;
    juce::AudioProcessorValueTreeState::Listener* unused = nullptr;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay { 192000 };
    juce::AudioBuffer<float> delayBuffer;
    double phase = 0.0, lfoPhase = 0.0;
    int samplesToNextStep = 0;
    float env = 0.0f, lastOut = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PsytranceAcidGeneratorAudioProcessor)
};
