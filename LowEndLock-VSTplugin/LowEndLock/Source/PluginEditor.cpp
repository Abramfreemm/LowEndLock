/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LowEndLockAudioProcessorEditor::LowEndLockAudioProcessorEditor (LowEndLockAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), waveformScope (p)
{
    setSize (440, 680);

    gainLabel.setText ("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (gainLabel);

    gainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    gainSlider.setTextValueSuffix (" dB");
    addAndMakeVisible (gainSlider);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getAPVTS(), "gain", gainSlider);

    savedCaptionLabel.setText ("CANCELLATION SAVED", juce::dontSendNotification);
    savedCaptionLabel.setJustificationType (juce::Justification::centred);
    savedCaptionLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    savedCaptionLabel.setColour (juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible (savedCaptionLabel);

    savedValueLabel.setText ("--", juce::dontSendNotification);
    savedValueLabel.setJustificationType (juce::Justification::centred);
    savedValueLabel.setFont (juce::FontOptions (46.0f, juce::Font::bold));
    savedValueLabel.setColour (juce::Label::textColourId, juce::Colours::limegreen);
    addAndMakeVisible (savedValueLabel);

    detailLabel.setText ("Press Lock to analyze", juce::dontSendNotification);
    detailLabel.setJustificationType (juce::Justification::centred);
    detailLabel.setFont (juce::FontOptions (14.0f));
    addAndMakeVisible (detailLabel);

    lockButton.setButtonText ("Lock");
    lockButton.setClickingTogglesState (true);
    lockButton.onClick = [this]
    {
        const auto locked = lockButton.getToggleState();
        audioProcessor.setLockEngaged (locked);
        lockButton.setButtonText (locked ? "Locked" : "Lock");
    };
    addAndMakeVisible (lockButton);

    abButton.setButtonText ("B (fix)");
    abButton.setClickingTogglesState (true);
    abButton.onClick = [this]
    {
        audioProcessor.setBypassCorrection (abButton.getToggleState());
    };
    addAndMakeVisible (abButton);

    addAndMakeVisible (waveformScope);

    manualButton.setButtonText ("Auto");
    manualButton.setClickingTogglesState (true);
    manualButton.onClick = [this]
    {
        manualButton.setButtonText (manualButton.getToggleState() ? "Manual" : "Auto");
    };
    manualAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        audioProcessor.getAPVTS(), "manual", manualButton);
    addAndMakeVisible (manualButton);

    invertButton.setButtonText ("Invert Polarity");
    invertAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        audioProcessor.getAPVTS(), "invert", invertButton);
    addAndMakeVisible (invertButton);

    delayLabel.setText ("Delay", juce::dontSendNotification);
    delayLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (delayLabel);
    delaySlider.setSliderStyle (juce::Slider::LinearHorizontal);
    delaySlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 54, 20);
    delaySlider.setTextValueSuffix (" ms");
    delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getAPVTS(), "delayMs", delaySlider);
    addAndMakeVisible (delaySlider);

    mixLabel.setText ("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (mixLabel);
    mixSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 54, 20);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getAPVTS(), "mix", mixSlider);
    addAndMakeVisible (mixSlider);

    lowCutLabel.setText ("Low Cut", juce::dontSendNotification);
    lowCutLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (lowCutLabel);
    lowCutSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    lowCutSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 54, 20);
    lowCutSlider.setTextValueSuffix (" Hz");
    lowCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getAPVTS(), "lowCut", lowCutSlider);
    addAndMakeVisible (lowCutSlider);

    bassLowLabel.setJustificationType (juce::Justification::centred);
    kickLowLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (bassLowLabel);
    addAndMakeVisible (kickLowLabel);

    startTimerHz (10);
}

LowEndLockAudioProcessorEditor::~LowEndLockAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void LowEndLockAudioProcessorEditor::timerCallback()
{
    const auto mainRms = audioProcessor.getMainLowRms();
    const auto sideRms = audioProcessor.getSidechainLowRms();

    bassLowLabel.setText (mainRms > 0.001f
        ? juce::String ("Bass Low: ") + juce::String (juce::Decibels::gainToDecibels (mainRms), 1) + " dB"
        : juce::String ("Bass Low: no signal"),
        juce::dontSendNotification);

    const auto sideConnected = sideRms > 0.001f;
    kickLowLabel.setText (sideConnected
        ? juce::String ("Sidechain (Kick): Connected  ") + juce::String (juce::Decibels::gainToDecibels (sideRms), 1) + " dB"
        : juce::String ("Sidechain (Kick): No Signal"),
        juce::dontSendNotification);
    kickLowLabel.setColour (juce::Label::textColourId,
                            sideConnected ? juce::Colours::limegreen : juce::Colours::grey);

    const auto locked = audioProcessor.isLockEngaged();
    const auto ready = audioProcessor.isAnalysisReady();
    const auto bypassed = audioProcessor.isBypassCorrection();

    if (locked && ready)
    {
        const auto savedDb = audioProcessor.getCancellationSavedDb();
        savedValueLabel.setText (juce::String (savedDb >= 0.0f ? "+" : "")
                                 + juce::String (savedDb, 1) + " dB",
                                 juce::dontSendNotification);
        savedValueLabel.setColour (juce::Label::textColourId,
                                   bypassed ? juce::Colours::grey
                                            : (savedDb >= 0.0f ? juce::Colours::limegreen : juce::Colours::orange));

        const auto invert = audioProcessor.getSuggestedPolarity() < 0;
        const auto delayMs = audioProcessor.getSuggestedDelaySamples()
                             / audioProcessor.getCurrentSampleRate() * 1000.0;
        const auto confidence = audioProcessor.getAnalysisConfidence() * 100.0f;
        const auto applied = confidence > 15.0f && ! bypassed;

        detailLabel.setText (juce::String ("Polarity: ") + (invert ? "Invert" : "Normal")
                             + "    Delay: " + juce::String (delayMs, 2) + " ms"
                             + "    Confidence: " + juce::String (confidence, 0) + "%"
                             + (applied ? "" : "   [not applied]"),
                             juce::dontSendNotification);
    }
    else if (locked)
    {
        savedValueLabel.setText ("Analyzing...", juce::dontSendNotification);
        savedValueLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
        detailLabel.setText ("Capturing 2 s of low band", juce::dontSendNotification);
    }
    else
    {
        savedValueLabel.setText ("--", juce::dontSendNotification);
        savedValueLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
        detailLabel.setText ("Press Lock to analyze", juce::dontSendNotification);
    }

    lockButton.setToggleState (locked, juce::dontSendNotification);
    lockButton.setButtonText (locked ? "Locked" : "Lock");
    abButton.setToggleState (bypassed, juce::dontSendNotification);
    abButton.setButtonText (bypassed ? "A (orig)" : "B (fix)");
    const auto manual = audioProcessor.getAPVTS().getRawParameterValue ("manual")->load() > 0.5f;
    manualButton.setButtonText (manual ? "Manual" : "Auto");

    waveformScope.repaint();
}

void LowEndLockAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (getLookAndFeel().findColour (juce::Label::textColourId));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("Low-End Lock", getLocalBounds().removeFromTop (40), juce::Justification::centred, true);
}

void LowEndLockAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (20);

    bounds.removeFromTop (40);                                   // title

    savedCaptionLabel.setBounds (bounds.removeFromTop (20));
    savedValueLabel.setBounds (bounds.removeFromTop (52));
    detailLabel.setBounds (bounds.removeFromTop (26));
    bounds.removeFromTop (8);
    waveformScope.setBounds (bounds.removeFromTop (110));
    bounds.removeFromTop (8);

    auto buttonsRow = bounds.removeFromTop (42);
    const auto buttonWidth = (buttonsRow.getWidth() - 16) / 3;
    lockButton.setBounds (buttonsRow.removeFromLeft (buttonWidth));
    buttonsRow.removeFromLeft (8);
    abButton.setBounds (buttonsRow.removeFromLeft (buttonWidth));
    buttonsRow.removeFromLeft (8);
    manualButton.setBounds (buttonsRow);

    bounds.removeFromTop (8);

    auto manualRow1 = bounds.removeFromTop (32);
    invertButton.setBounds (manualRow1.removeFromLeft (110));
    manualRow1.removeFromLeft (8);
    delayLabel.setBounds (manualRow1.removeFromLeft (40));
    delaySlider.setBounds (manualRow1);

    bounds.removeFromTop (4);

    auto manualRow2 = bounds.removeFromTop (32);
    const auto half = manualRow2.getWidth() / 2;
    auto mixHalf = manualRow2.removeFromLeft (half);
    mixLabel.setBounds (mixHalf.removeFromLeft (44));
    mixSlider.setBounds (mixHalf);
    lowCutLabel.setBounds (manualRow2.removeFromLeft (44));
    lowCutSlider.setBounds (manualRow2);

    bounds.removeFromTop (8);

    auto metersBounds = bounds.removeFromBottom (56);
    bassLowLabel.setBounds (metersBounds.removeFromTop (26));
    kickLowLabel.setBounds (metersBounds.removeFromTop (26));

    auto controlsBounds = bounds.withSizeKeepingCentre (120, 140);
    gainLabel.setBounds (controlsBounds.removeFromTop (24));
    gainSlider.setBounds (controlsBounds);
}
