/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    struct PhaseAnalysisResult
    {
        bool valid = false;
        bool invertPolarity = false;
        float delaySamples = 0.0f;
        float confidence = 0.0f;
    };

    PhaseAnalysisResult calculatePhaseAnalysis (const float* bass,
                                                const float* kick,
                                                int length,
                                                double sampleRate)
    {
        PhaseAnalysisResult result;

        if (length < 256)
            return result;

        double bassSum = 0.0;
        double kickSum = 0.0;
        double bassEnergy = 0.0;
        double kickEnergy = 0.0;

        for (int i = 0; i < length; ++i)
        {
            bassSum += bass[i];
            kickSum += kick[i];
            bassEnergy += bass[i] * bass[i];
            kickEnergy += kick[i] * kick[i];
        }

        const auto bassRms = std::sqrt (bassEnergy / static_cast<double> (length));
        const auto kickRms = std::sqrt (kickEnergy / static_cast<double> (length));

        if (bassRms < 1e-4 || kickRms < 1e-4)
            return result;

        const auto bassMean = bassSum / static_cast<double> (length);
        const auto kickMean = kickSum / static_cast<double> (length);
        const auto maxLag = std::min (static_cast<int> (sampleRate * 0.02), length / 2);

        int bestLag = 0;
        float bestCorrelation = 0.0f;

        for (int lag = -maxLag; lag <= maxLag; ++lag)
        {
            double crossSum = 0.0;
            double bassSquares = 0.0;
            double kickSquares = 0.0;
            int count = 0;

            for (int i = 0; i < length; ++i)
            {
                const auto kickIndex = i + lag;

                if (kickIndex < 0 || kickIndex >= length)
                    continue;

                const auto bassCentered = bass[i] - bassMean;
                const auto kickCentered = kick[kickIndex] - kickMean;

                crossSum += bassCentered * kickCentered;
                bassSquares += bassCentered * bassCentered;
                kickSquares += kickCentered * kickCentered;
                ++count;
            }

            if (count == 0)
                continue;

            const auto denominator = std::sqrt (bassSquares * kickSquares);
            const auto correlation = denominator > 1e-12
                ? static_cast<float> (crossSum / denominator)
                : 0.0f;

            if (std::abs (correlation) > std::abs (bestCorrelation) + 1e-9f
                || (std::abs (correlation - bestCorrelation) < 1e-9f
                    && std::abs (lag) < std::abs (bestLag)))
            {
                bestCorrelation = correlation;
                bestLag = lag;
            }
        }

        result.valid = true;
        result.invertPolarity = bestCorrelation < 0.0f;
        result.delaySamples = static_cast<float> (-bestLag);
        result.confidence = std::clamp (std::abs (bestCorrelation), 0.0f, 1.0f);
        return result;
    }

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

        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "mix", 1 },
            "Mix",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
            1.0f));

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
      Thread ("LowEndAnalysis"),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

LowEndLockAudioProcessor::~LowEndLockAudioProcessor()
{
    stopThread (1000);
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
    currentSampleRate = sampleRate;

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

    analysisMainHighPass[0].coefficients = highPassCoefficients;
    analysisMainLowPass [0].coefficients = lowPassCoefficients;
    analysisSideHighPass[0].coefficients = highPassCoefficients;
    analysisSideLowPass [0].coefficients = lowPassCoefficients;

    analysisMainHighPass[0].reset();
    analysisMainLowPass [0].reset();
    analysisSideHighPass[0].reset();
    analysisSideLowPass [0].reset();

    analysisBufferSize = static_cast<int> (sampleRate * 2.0);
    analysisMainBuffer.setSize (1, analysisBufferSize);
    analysisSideBuffer.setSize (1, analysisBufferSize);
    analysisMainBuffer.clear();
    analysisSideBuffer.clear();
    analysisWriteIndex = 0;

    const auto maxDelaySamples = static_cast<int> (sampleRate * 0.04) + 8;
    correctionDelay = std::make_unique<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>> (
        static_cast<size_t> (maxDelaySamples));
    correctionDelay->prepare ({ sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 });

    const auto correctionLowPassCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 150.0f);
    for (int channel = 0; channel < 2; ++channel)
    {
        correctionLowPass[channel].coefficients = correctionLowPassCoefficients;
        correctionLowPass[channel].reset();
    }

    polaritySmoother.reset (sampleRate, 0.02);
    polaritySmoother.setCurrentAndTargetValue (1.0f);

    delayCenterSamples = static_cast<float> (maxDelaySamples) * 0.5f;
    delaySmoother.reset (sampleRate, 0.05);
    delaySmoother.setCurrentAndTargetValue (delayCenterSamples);

    if (! isThreadRunning())
        startThread();
}

void LowEndLockAudioProcessor::requestAnalysis()
{
    analysisWriteIndex = 0;
    analysisMainBuffer.clear();
    analysisSideBuffer.clear();
    analysisResultReady.store (false);
    analysisDataReady.store (false);
    analysisRequested.store (true);
    notify();
}

void LowEndLockAudioProcessor::captureAnalysisData (const juce::AudioBuffer<float>& main,
                                                    const juce::AudioBuffer<float>& side)
{
    if (! analysisRequested.load() || analysisWriteIndex >= analysisBufferSize)
        return;

    auto* mainWrite = analysisMainBuffer.getWritePointer (0);
    auto* sideWrite = analysisSideBuffer.getWritePointer (0);
    const auto numSamples = std::min (main.getNumSamples(), side.getNumSamples());

    for (int sample = 0; sample < numSamples; ++sample)
    {
        if (analysisWriteIndex >= analysisBufferSize)
            break;

        const auto mainSample = main.getNumChannels() > 0
            ? main.getReadPointer (0)[sample]
            : 0.0f;
        const auto sideSample = side.getNumChannels() > 0
            ? side.getReadPointer (0)[sample]
            : 0.0f;

        mainWrite[analysisWriteIndex] = analysisMainLowPass[0].processSample (
            analysisMainHighPass[0].processSample (mainSample));
        sideWrite[analysisWriteIndex] = analysisSideLowPass[0].processSample (
            analysisSideHighPass[0].processSample (sideSample));

        ++analysisWriteIndex;
    }

    if (analysisWriteIndex >= analysisBufferSize)
    {
        analysisRequested.store (false);
        analysisDataReady.store (true);
        notify();
    }
}

void LowEndLockAudioProcessor::run()
{
    while (! threadShouldExit())
    {
        if (analysisDataReady.exchange (false))
        {
            const auto* mainData = analysisMainBuffer.getReadPointer (0);
            const auto* sideData = analysisSideBuffer.getReadPointer (0);
            const auto result = calculatePhaseAnalysis (mainData, sideData, analysisBufferSize, currentSampleRate);

            suggestedPolarity.store (result.invertPolarity ? -1 : 1);
            suggestedDelaySamples.store (result.delaySamples);
            analysisConfidence.store (result.confidence);
            analysisResultReady.store (true);
        }
        else
        {
            wait (10);
        }
    }
}

void LowEndLockAudioProcessor::releaseResources()
{
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

    captureAnalysisData (mainInputOutput, sidechainInput);

    const auto mix = apvts.getRawParameterValue ("mix")->load();
    const auto analysisReady = analysisResultReady.load();
    const auto confidence = analysisConfidence.load();

    if (analysisReady && confidence > 0.15f)
    {
        polaritySmoother.setTargetValue (suggestedPolarity.load() < 0 ? -1.0f : 1.0f);
        delaySmoother.setTargetValue (delayCenterSamples + suggestedDelaySamples.load());
    }
    else
    {
        polaritySmoother.setTargetValue (1.0f);
        delaySmoother.setTargetValue (delayCenterSamples);
    }

    for (int sample = 0; sample < mainInputOutput.getNumSamples(); ++sample)
    {
        const auto polarity = polaritySmoother.getNextValue();
        const auto delaySamples = juce::jlimit (0.0f, delayCenterSamples * 2.0f, delaySmoother.getNextValue());
        correctionDelay->setDelay (delaySamples);

        for (int channel = 0; channel < mainInputOutput.getNumChannels(); ++channel)
        {
            const auto original = mainInputOutput.getReadPointer (channel)[sample];
            const auto lowBand = correctionLowPass[channel].processSample (original);

            correctionDelay->pushSample (channel, lowBand * polarity);
            const auto correctedLowBand = correctionDelay->popSample (channel);

            mainInputOutput.getWritePointer (channel)[sample] =
                original + mix * (correctedLowBand - lowBand);
        }
    }

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
