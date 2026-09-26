/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    float calculateBandLimitedRms (const juce::AudioBuffer<float>& buffer,
                                   std::array<juce::dsp::IIR::Filter<float>, 2>& highPass,
                                   std::array<juce::dsp::IIR::Filter<float>, 2>& lowPass)
    {
        double sumSquares = 0.0;
        int sampleCount = 0;

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            const auto* channelData = buffer.getReadPointer (channel);

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                const auto filtered = lowPass[channel].processSample (
                    highPass[channel].processSample (channelData[sample]));

                sumSquares += static_cast<double> (filtered) * filtered;
                ++sampleCount;
            }
        }

        return sampleCount > 0
            ? static_cast<float> (std::sqrt (sumSquares / static_cast<double> (sampleCount)))
            : 0.0f;
    }

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "gain", 1 },
            "Gain",
            juce::NormalisableRange<float> (-60.0f, 12.0f, 0.1f),
            0.0f));

        return layout;
    }
}

//==============================================================================
LowEndLockAudioProcessor::LowEndLockAudioProcessor()
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       .withInput  ("Sidechain", juce::AudioChannelSet::stereo())),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

LowEndLockAudioProcessor::~LowEndLockAudioProcessor()
{
}

//==============================================================================
const juce::String LowEndLockAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LowEndLockAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LowEndLockAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LowEndLockAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LowEndLockAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LowEndLockAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LowEndLockAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LowEndLockAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LowEndLockAudioProcessor::getProgramName (int index)
{
    return {};
}

void LowEndLockAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LowEndLockAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    gainSmoother.reset (sampleRate, 0.02);

    const auto initialGainDb = apvts.getRawParameterValue ("gain")->load();
    gainSmoother.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (initialGainDb));

    const auto highPassCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 20.0f);
    const auto lowPassCoefficients  = juce::dsp::IIR::Coefficients<float>::makeLowPass  (sampleRate, 150.0f);

    for (int channel = 0; channel < 2; ++channel)
    {
        mainHighPass[channel].coefficients = highPassCoefficients;
        mainLowPass [channel].coefficients = lowPassCoefficients;
        sideHighPass[channel].coefficients = highPassCoefficients;
        sideLowPass [channel].coefficients = lowPassCoefficients;

        mainHighPass[channel].reset();
        mainLowPass [channel].reset();
        sideHighPass[channel].reset();
        sideLowPass [channel].reset();
    }
}

void LowEndLockAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LowEndLockAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainInputChannelSet().isDisabled())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
  #endif
}
#endif

void LowEndLockAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto mainInputOutput = getBusBuffer (buffer, true, 0);
    auto sidechainInput  = getBusBuffer (buffer, true, 1);

    mainLowRms.store (calculateBandLimitedRms (mainInputOutput, mainHighPass, mainLowPass));
    sideLowRms.store (calculateBandLimitedRms (sidechainInput, sideHighPass, sideLowPass));

    const auto gainDb = apvts.getRawParameterValue ("gain")->load();
    gainSmoother.setTargetValue (juce::Decibels::decibelsToGain (gainDb));

    for (int channel = 0; channel < mainInputOutput.getNumChannels(); ++channel)
    {
        auto* channelData = mainInputOutput.getWritePointer (channel);

        for (int sample = 0; sample < mainInputOutput.getNumSamples(); ++sample)
            channelData[sample] *= gainSmoother.getNextValue();
    }

}

//==============================================================================
bool LowEndLockAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LowEndLockAudioProcessor::createEditor()
{
    return new LowEndLockAudioProcessorEditor (*this);
}

//==============================================================================
void LowEndLockAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void LowEndLockAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LowEndLockAudioProcessor();
}
