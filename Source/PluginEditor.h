/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class IRConvReverbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    IRConvReverbAudioProcessorEditor (IRConvReverbAudioProcessor&);
    ~IRConvReverbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    IRConvReverbAudioProcessor& audioProcessor;

    juce::TextButton loadBtn;
    std::unique_ptr<juce::FileChooser> fileChooser;

    //Slider
    juce::Slider mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IRConvReverbAudioProcessorEditor)
};
