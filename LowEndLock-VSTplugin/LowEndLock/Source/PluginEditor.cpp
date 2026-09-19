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
    gainLabel.setText ("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (gainLabel);

    gainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    gainSlider.setTextValueSuffix (" dB");
    addAndMakeVisible (gainSlider);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getAPVTS(), "gain", gainSlider);

    setSize (420, 240);
}

LowEndLockAudioProcessorEditor::~LowEndLockAudioProcessorEditor()
{
}

//==============================================================================
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

    auto controlsBounds = bounds.withSizeKeepingCentre (120, 140);

    gainLabel.setBounds (controlsBounds.removeFromTop (24));
    gainSlider.setBounds (controlsBounds);
}
