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
    setLookAndFeel (&lookAndFeel);

    setSize (440, 700);

    presetLabel.setText ("Preset", juce::dontSendNotification);
    presetLabel.setJustificationType (juce::Justification::right);
    addAndMakeVisible (presetLabel);

    presetBox.addItemList ({ "默认", "自动紧致", "极性翻转", "柔和混合", "宽低频" }, 1);
    presetBox.setSelectedId (1);
    presetBox.onChange = [this] { applyPreset (presetBox.getSelectedId()); };
    addAndMakeVisible (presetBox);

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
    invertButton.onClick = [this] { switchToManual(); };
    addAndMakeVisible (invertButton);

    delayLabel.setText ("Delay", juce::dontSendNotification);
    delayLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (delayLabel);
    delaySlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    delaySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 18);
    delaySlider.setTextValueSuffix (" ms");
    delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getAPVTS(), "delayMs", delaySlider);
    delaySlider.onDragStart = [this] { switchToManual(); };
    addAndMakeVisible (delaySlider);

    mixLabel.setText ("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (mixLabel);
    mixSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 18);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getAPVTS(), "mix", mixSlider);
    addAndMakeVisible (mixSlider);

    lowCutLabel.setText ("Low Band", juce::dontSendNotification);
    lowCutLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (lowCutLabel);
    lowCutSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    lowCutSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 18);
    lowCutSlider.setTextValueSuffix (" Hz");
    lowCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getAPVTS(), "lowCut", lowCutSlider);
    addAndMakeVisible (lowCutSlider);

    bassLowLabel.setJustificationType (juce::Justification::centred);
    kickLowLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (bassLowLabel);
    addAndMakeVisible (kickLowLabel);
    addAndMakeVisible (sidechainLed);

    startTimerHz (10);
}

LowEndLockAudioProcessorEditor::~LowEndLockAudioProcessorEditor()
{
    stopTimer();
}

void LowEndLockAudioProcessorEditor::applyPreset (int presetId)
{
    auto& apvts = audioProcessor.getAPVTS();

    auto set = [&apvts] (const juce::String& paramId, float value)
    {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (paramId)))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
    };

    switch (presetId)
    {
        case 2:  // Auto Tight
            set ("mix", 1.0f);
            set ("manual", 0.0f);
            set ("invert", 0.0f);
            set ("delayMs", 0.0f);
            set ("lowCut", 120.0f);
            break;

        case 3:  // Polarity Flip
            set ("mix", 1.0f);
            set ("manual", 1.0f);
            set ("invert", 1.0f);
            set ("delayMs", 0.0f);
            break;

        case 4:  // Deep Blend
            set ("mix", 0.6f);
            set ("manual", 0.0f);
            set ("invert", 0.0f);
            set ("delayMs", 0.0f);
            set ("lowCut", 180.0f);
            break;

        case 5:  // Wide Low
            set ("mix", 1.0f);
            set ("manual", 0.0f);
            set ("lowCut", 250.0f);
            break;

        case 1:  // Default
        default:
            set ("mix", 1.0f);
            set ("manual", 0.0f);
            set ("invert", 0.0f);
            set ("delayMs", 0.0f);
            set ("lowCut", 150.0f);
            break;
    }
}

void LowEndLockAudioProcessorEditor::switchToManual()
{
    if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.getAPVTS().getParameter ("manual")))
        p->setValueNotifyingHost (1.0f);
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
    sidechainLed.setActive (sideConnected);

    const auto locked = audioProcessor.isLockEngaged();
    const auto ready = audioProcessor.isAnalysisReady();
    const auto bypassed = audioProcessor.isBypassCorrection();
    const auto manual = audioProcessor.getAPVTS().getRawParameterValue ("manual")->load() > 0.5f;

    if (manual)
    {
        savedValueLabel.setText ("Manual", juce::dontSendNotification);
        savedValueLabel.setColour (juce::Label::textColourId,
                                   bypassed ? juce::Colours::grey : juce::Colours::limegreen);

        const auto invert = audioProcessor.getEffectivePolarity() < 0.0f;
        const auto delayMs = audioProcessor.getEffectiveDelaySamples()
                             / audioProcessor.getCurrentSampleRate() * 1000.0;
        detailLabel.setText (juce::String ("Polarity: ") + (invert ? "Invert" : "Normal")
                             + "    Delay: " + juce::String (delayMs, 2) + " ms"
                             + (bypassed ? "   [bypassed]" : ""),
                             juce::dontSendNotification);
    }
    else if (locked && ready)
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
    manualButton.setButtonText (manual ? "Manual" : "Auto");

    waveformScope.repaint();
}

void LowEndLockAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    juce::ColourGradient bgGradient (juce::Colour (0xff1d2026), 0.0f, 0.0f,
                                     juce::Colour (0xff0f1114), 0.0f, area.getHeight(), false);
    g.setGradientFill (bgGradient);
    g.fillAll();

    g.setColour (juce::Colour (0xffe8eaed));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("Low-End Lock", getLocalBounds().removeFromTop (40), juce::Justification::centred, true);
}

void LowEndLockAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (20);

    bounds.removeFromTop (40);                                   // title

    auto presetRow = bounds.removeFromTop (30);
    presetLabel.setBounds (presetRow.removeFromLeft (70));
    presetBox.setBounds (presetRow);
    bounds.removeFromTop (4);

    savedCaptionLabel.setBounds (bounds.removeFromTop (20));
    savedValueLabel.setBounds (bounds.removeFromTop (52));
    detailLabel.setBounds (bounds.removeFromTop (26));
    bounds.removeFromTop (8);
    waveformScope.setBounds (bounds.removeFromTop (170));
    bounds.removeFromTop (8);

    auto buttonsRow = bounds.removeFromTop (42);
    const auto buttonWidth = (buttonsRow.getWidth() - 16) / 3;
    lockButton.setBounds (buttonsRow.removeFromLeft (buttonWidth));
    buttonsRow.removeFromLeft (8);
    abButton.setBounds (buttonsRow.removeFromLeft (buttonWidth));
    buttonsRow.removeFromLeft (8);
    manualButton.setBounds (buttonsRow);

    bounds.removeFromTop (10);

    auto knobsRow = bounds.removeFromTop (120);
    const auto knobWidth = knobsRow.getWidth() / 4;

    auto gainKnob = knobsRow.removeFromLeft (knobWidth);
    gainLabel.setBounds (gainKnob.removeFromTop (18));
    gainSlider.setBounds (gainKnob);

    auto delayKnob = knobsRow.removeFromLeft (knobWidth);
    delayLabel.setBounds (delayKnob.removeFromTop (18));
    delaySlider.setBounds (delayKnob);

    auto mixKnob = knobsRow.removeFromLeft (knobWidth);
    mixLabel.setBounds (mixKnob.removeFromTop (18));
    mixSlider.setBounds (mixKnob);

    lowCutLabel.setBounds (knobsRow.removeFromTop (18));
    lowCutSlider.setBounds (knobsRow);

    bounds.removeFromTop (10);

    auto invertRow = bounds.removeFromTop (28);
    invertButton.setBounds (invertRow.removeFromLeft (150));

    bounds.removeFromTop (6);

    auto metersBounds = bounds.removeFromBottom (52);
    bassLowLabel.setBounds (metersBounds.removeFromTop (24));
    auto kickRow = metersBounds.removeFromTop (24);
    sidechainLed.setBounds (kickRow.removeFromLeft (16).withSizeKeepingCentre (12, 12));
    kickLowLabel.setBounds (kickRow);
}
