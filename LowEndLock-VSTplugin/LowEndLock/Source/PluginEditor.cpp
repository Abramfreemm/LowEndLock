/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LowEndLockAudioProcessorEditor::LowEndLockAudioProcessorEditor (LowEndLockAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (420, 300);

    gainLabel.setText ("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (gainLabel);

    gainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    gainSlider.setTextValueSuffix (" dB");
    addAndMakeVisible (gainSlider);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getAPVTS(), "gain", gainSlider);

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

    kickLowLabel.setText (sideRms > 0.001f
        ? juce::String ("Kick Low: ") + juce::String (juce::Decibels::gainToDecibels (sideRms), 1) + " dB"
        : juce::String ("Kick Low: no signal"),
        juce::dontSendNotification);
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

    bounds.removeFromTop (40);

    auto statusBounds = bounds.removeFromBottom (64);
    bassLowLabel.setBounds (statusBounds.removeFromTop (28));
    kickLowLabel.setBounds (statusBounds.removeFromTop (28));

    auto controlsBounds = bounds.withSizeKeepingCentre (120, 140);

    gainLabel.setBounds (controlsBounds.removeFromTop (24));
    gainSlider.setBounds (controlsBounds);
}
