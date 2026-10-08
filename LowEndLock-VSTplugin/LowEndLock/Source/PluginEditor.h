/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

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
        g.fillAll (juce::Colours::black);

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

        const auto locked = processor.isLockEngaged()
                            && processor.isAnalysisReady()
                            && ! processor.isBypassCorrection();
        if (locked)
        {
            const auto polarity = processor.getSuggestedPolarity() < 0 ? -1.0f : 1.0f;
            const auto delay = processor.getSuggestedDelaySamples();

            g.setColour (juce::Colours::limegreen.withAlpha (0.9f));
            drawTrace (g, main, midY, scale, w, n, delay, polarity);
        }

        g.setFont (juce::FontOptions (11.0f));
        g.setColour (juce::Colours::cyan);
        g.drawText ("Kick", 4, 4, 40, 14, juce::Justification::left, false);
        g.setColour (juce::Colours::orange);
        g.drawText ("Bass", 4, 18, 40, 14, juce::Justification::left, false);
        if (locked)
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
/**
*/
class LowEndLockAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    LowEndLockAudioProcessorEditor (LowEndLockAudioProcessor&);
    ~LowEndLockAudioProcessorEditor() override;

    //==============================================================================
    void timerCallback() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LowEndLockAudioProcessor& audioProcessor;

    juce::Label gainLabel;
    juce::Label bassLowLabel;
    juce::Label kickLowLabel;
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
