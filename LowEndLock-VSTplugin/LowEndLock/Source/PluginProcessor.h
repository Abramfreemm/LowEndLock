/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <array>
#include <atomic>
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/**
*/
class LowEndLockAudioProcessor  : public juce::AudioProcessor,
                                  private juce::Thread
{
public:
    //==============================================================================
    LowEndLockAudioProcessor();
    ~LowEndLockAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    float getMainLowRms() const { return mainLowRms.load(); }
    float getSidechainLowRms() const { return sideLowRms.load(); }
    float getSuggestedDelaySamples() const { return suggestedDelaySamples.load(); }
    int   getSuggestedPolarity() const { return suggestedPolarity.load(); }
    float getAnalysisConfidence() const { return analysisConfidence.load(); }

    void requestAnalysis();

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    void run() override;
    void captureAnalysisData (const juce::AudioBuffer<float>& main,
                              const juce::AudioBuffer<float>& side);

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
    juce::LinearSmoothedValue<float> gainSmoother;
    std::atomic<float> mainLowRms { 0.0f };
    std::atomic<float> sideLowRms { 0.0f };

    juce::AudioBuffer<float> analysisMainBuffer;
    juce::AudioBuffer<float> analysisSideBuffer;
    int analysisBufferSize = 0;
    int analysisWriteIndex = 0;
    double currentSampleRate = 48000.0;

    std::atomic<bool> analysisRequested { false };
    std::atomic<bool> analysisDataReady { false };
    std::atomic<bool> analysisResultReady { false };
    std::atomic<int> suggestedPolarity { 1 };
    std::atomic<float> suggestedDelaySamples { 0.0f };
    std::atomic<float> analysisConfidence { 0.0f };

    std::array<juce::dsp::IIR::Filter<float>, 2> mainHighPass;
    std::array<juce::dsp::IIR::Filter<float>, 2> mainLowPass;
    std::array<juce::dsp::IIR::Filter<float>, 2> sideHighPass;
    std::array<juce::dsp::IIR::Filter<float>, 2> sideLowPass;
    std::array<juce::dsp::IIR::Filter<float>, 1> analysisMainHighPass;
    std::array<juce::dsp::IIR::Filter<float>, 1> analysisMainLowPass;
    std::array<juce::dsp::IIR::Filter<float>, 1> analysisSideHighPass;
    std::array<juce::dsp::IIR::Filter<float>, 1> analysisSideLowPass;

    juce::LinearSmoothedValue<float> polaritySmoother;
    juce::LinearSmoothedValue<float> delaySmoother;
    std::array<juce::dsp::IIR::Filter<float>, 2> correctionLowPass;
    std::unique_ptr<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>> correctionDelay;
    float delayCenterSamples = 0.0f;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowEndLockAudioProcessor)
};
