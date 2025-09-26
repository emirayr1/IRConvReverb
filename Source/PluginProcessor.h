/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class IRConvReverbAudioProcessor  : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    IRConvReverbAudioProcessor();
    ~IRConvReverbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void updateFilter();

    juce::File root, savedFile;
    juce::dsp::Convolution irLoader;

    juce::AudioProcessorValueTreeState treeState;

private:
    //==============================================================================
    float mix = 0.5f;
    float cutOffFrequency = 600.0f;
    float resonanceValue = 1.0f;
    int filterTypeValue = 0;

    float sampleRateCopy;

    juce::dsp::ProcessSpec spec;
    juce::dsp::ProcessorDuplicator<juce::dsp::StateVariableFilter::Filter<float>,
        juce::dsp::StateVariableFilter::Parameters<float>> stateVariableFilter;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void parameterChanged(const juce::String& parameterID, float newValue);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IRConvReverbAudioProcessor)
};
