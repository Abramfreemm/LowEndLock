/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/** Sonible-inspired flat dark look: minimal rotary knobs with a value arc and
    pointer dot, a deep near-black panel and a single teal accent colour.
*/
class LowEndLookAndFeel : public juce::LookAndFeel_V4
{
public:
    LowEndLookAndFeel()
    {
        const auto bg       = juce::Colour (0xff14161a);
        const auto panel    = juce::Colour (0xff1b1e23);
        const auto outline  = juce::Colour (0xff2a2e35);
        const auto text     = juce::Colour (0xffe8eaed);
        const auto muted    = juce::Colour (0xff8a9099);
        const auto accent   = juce::Colour (0xff2dd4bf);

        setColour (juce::ResizableWindow::backgroundColourId, bg);
        setColour (juce::Slider::rotarySliderFillColourId, accent);
        setColour (juce::Slider::rotarySliderOutlineColourId, outline);
        setColour (juce::Slider::thumbColourId, accent);
        setColour (juce::Slider::textBoxTextColourId, text);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Label::textColourId, text);
        setColour (juce::Label::textWhenEditingColourId, text);
        setColour (juce::TextButton::buttonColourId, panel);
        setColour (juce::TextButton::buttonOnColourId, accent);
        setColour (juce::TextButton::textColourOffId, text);
        setColour (juce::TextButton::textColourOnId, juce::Colour (0xff0b0d0f));
        setColour (juce::ComboBox::backgroundColourId, panel);
        setColour (juce::ComboBox::textColourId, text);
        setColour (juce::ComboBox::outlineColourId, outline);
        setColour (juce::ComboBox::arrowColourId, accent);
        setColour (juce::PopupMenu::backgroundColourId, panel);
        setColour (juce::PopupMenu::textColourId, text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, accent);
        setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (0xff0b0d0f));
        setColour (juce::ToggleButton::tickColourId, accent);
        setColour (juce::ToggleButton::tickDisabledColourId, muted);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        const auto radius = juce::jmin (width, height) * 0.5f - 5.0f;
        const auto centreX = x + width * 0.5f;
        const auto centreY = y + height * 0.5f;

        if (radius <= 4.0f)
            return;

        g.setColour (juce::Colour (0xff1b1e23));
        g.fillEllipse (centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

        g.setColour (juce::Colour (0xff2a2e35));
        g.drawEllipse (centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        const auto arcRadius = radius - 3.5f;
        const auto angleStart = rotaryStartAngle;
        const auto angleEnd = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        juce::Path arc;
        constexpr int segments = 48;
        const auto step = (angleEnd - angleStart) / static_cast<float> (segments);
        for (int i = 0; i <= segments; ++i)
        {
            const auto a = angleStart + static_cast<float> (i) * step;
            const auto px = centreX + arcRadius * std::sin (a);
            const auto py = centreY - arcRadius * std::cos (a);
            if (i == 0) arc.startNewSubPath (px, py);
            else        arc.lineTo (px, py);
        }

        g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
        g.strokePath (arc, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const auto dotRadius = arcRadius;
        const auto dotX = centreX + dotRadius * std::sin (angleEnd);
        const auto dotY = centreY - dotRadius * std::cos (angleEnd);
        g.fillEllipse (dotX - 2.5f, dotY - 2.5f, 5.0f, 5.0f);
    }
};

//==============================================================================
/** A live low-band scope showing Bass (main) and Kick (sidechain) waveforms,
    plus the corrected Bass trace when the correction is locked in.
*/
class WaveformScope : public juce::Component
{
public:
    explicit WaveformScope (LowEndLockAudioProcessor& p) : processor (p) {}

    void paint (juce::Graphics& g) override
    {
        const auto panel = getLocalBounds().toFloat().reduced (2.0f);
        const auto corner = 12.0f;

        juce::ColourGradient panelGrad (juce::Colour (0xff1c2026), 0.0f, panel.getY(),
                                        juce::Colour (0xff14171c), 0.0f, panel.getBottom(), false);
        g.setGradientFill (panelGrad);
        g.fillRoundedRectangle (panel, corner);

        juce::ColourGradient highlight (juce::Colour (0x16ffffff), 0.0f, panel.getY(),
                                        juce::Colours::transparentBlack, 0.0f, panel.getY() + panel.getHeight() * 0.4f, false);
        g.setGradientFill (highlight);
        g.fillRoundedRectangle (panel, corner);

        g.setColour (juce::Colour (0x16ffffff));
        g.drawRoundedRectangle (panel.reduced (0.5f), corner, 1.0f);

        std::vector<float> main, side;
        processor.getScopeData (main, side);

        const auto w = static_cast<float> (getWidth());
        const auto h = static_cast<float> (getHeight());

        if (w < 4.0f || h < 4.0f || main.empty() || main.size() != side.size())
            return;

        const auto midY = h * 0.5f;

        g.setColour (juce::Colours::darkgrey);
        g.drawLine (0.0f, midY, w, midY, 1.0f);

        auto peak = 0.0f;
        for (const auto v : main) peak = std::max (peak, std::abs (v));
        for (const auto v : side) peak = std::max (peak, std::abs (v));

        const auto scale = peak > 1e-6f ? (h * 0.42f) / peak : 0.0f;
        const auto n = static_cast<int> (main.size());

        g.setColour (juce::Colours::cyan.withAlpha (0.9f));
        drawTrace (g, side, midY, scale, w, n, 0.0f, 1.0f);

        g.setColour (juce::Colours::orange.withAlpha (0.9f));
        drawTrace (g, main, midY, scale, w, n, 0.0f, 1.0f);

        const auto corrected = processor.isCorrectionActive();
        if (corrected)
        {
            const auto polarity = processor.getEffectivePolarity();
            const auto delay = processor.getEffectiveDelaySamples();

            g.setColour (juce::Colours::limegreen.withAlpha (0.9f));
            drawTrace (g, main, midY, scale, w, n, delay, polarity);
        }

        g.setFont (juce::FontOptions (11.0f));
        g.setColour (juce::Colours::cyan);
        g.drawText ("Kick", 4, 4, 40, 14, juce::Justification::left, false);
        g.setColour (juce::Colours::orange);
        g.drawText ("Bass", 4, 18, 40, 14, juce::Justification::left, false);
        if (corrected)
        {
            g.setColour (juce::Colours::limegreen);
            g.drawText ("Fixed", 4, 32, 48, 14, juce::Justification::left, false);
        }
    }

private:
    LowEndLockAudioProcessor& processor;

    static void drawTrace (juce::Graphics& g,
                           const std::vector<float>& data,
                           float midY, float scale, float w, int n,
                           float shiftSamples, float gain)
    {
        juce::Path path;
        bool first = true;

        for (int x = 0; x < static_cast<int> (w); ++x)
        {
            const auto idx = static_cast<int> ((static_cast<float> (x) / w) * static_cast<float> (n));
            const auto src = idx - static_cast<int> (std::round (shiftSamples));

            auto v = 0.0f;
            if (src >= 0 && src < n)
                v = data[static_cast<size_t> (src)] * gain;

            const auto y = midY - v * scale;

            if (first)
            {
                path.startNewSubPath (static_cast<float> (x), y);
                first = false;
            }
            else
            {
                path.lineTo (static_cast<float> (x), y);
            }
        }

        g.strokePath (path, juce::PathStrokeType (1.0f));
    }
};

//==============================================================================
/** A small status LED that glows green when the sidechain signal is present. */
class StatusLED : public juce::Component
{
public:
    void setActive (bool a)
    {
        if (a == active)
            return;
        active = a;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto centre = getLocalBounds().toFloat().getCentre();
        const auto r = juce::jmin (getWidth(), getHeight()) * 0.5f;

        if (active)
        {
            g.setColour (juce::Colours::limegreen.withAlpha (0.22f));
            g.fillEllipse (centre.x - r, centre.y - r, r * 2.0f, r * 2.0f);
        }

        g.setColour (active ? juce::Colours::limegreen : juce::Colour (0xff3a3f46));
        g.fillEllipse (centre.x - r * 0.55f, centre.y - r * 0.55f, r * 1.1f, r * 1.1f);
    }

private:
    bool active = false;
};

//==============================================================================
/**
*/
class LowEndLockAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    LowEndLockAudioProcessorEditor (LowEndLockAudioProcessor&);
    ~LowEndLockAudioProcessorEditor() override;

    void applyPreset (int presetId);
    void switchToManual();

    //==============================================================================
    void timerCallback() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LowEndLockAudioProcessor& audioProcessor;

    LowEndLookAndFeel lookAndFeel;
    juce::Label presetLabel;
    juce::ComboBox presetBox;
    juce::Label gainLabel;
    juce::Label bassLowLabel;
    juce::Label kickLowLabel;
    StatusLED sidechainLed;
    juce::Label savedCaptionLabel;
    juce::Label savedValueLabel;
    juce::Label detailLabel;
    juce::Slider gainSlider;
    juce::TextButton lockButton;
    juce::TextButton abButton;
    juce::TextButton manualButton;
    juce::ToggleButton invertButton;
    juce::Label delayLabel;
    juce::Label mixLabel;
    juce::Label lowCutLabel;
    juce::Slider delaySlider;
    juce::Slider mixSlider;
    juce::Slider lowCutSlider;
    WaveformScope waveformScope;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> manualAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> invertAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowCutAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowEndLockAudioProcessorEditor)
};
