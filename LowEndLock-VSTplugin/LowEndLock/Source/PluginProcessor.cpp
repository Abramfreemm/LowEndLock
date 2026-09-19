/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
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
                       ),
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
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LowEndLockAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const auto gainDb = apvts.getRawParameterValue ("gain")->load();
    gainSmoother.setTargetValue (juce::Decibels::decibelsToGain (gainDb));

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
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
