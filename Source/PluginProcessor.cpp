/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
IRConvReverbAudioProcessor::IRConvReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    treeState.addParameterListener("mix", this);
    treeState.addParameterListener("cutoff", this);
    treeState.addParameterListener("resonance", this);
    treeState.addParameterListener("filterType", this);

}

IRConvReverbAudioProcessor::~IRConvReverbAudioProcessor()
{
    treeState.removeParameterListener("mix", this);
    treeState.removeParameterListener("cutoff", this);
    treeState.removeParameterListener("resonance", this);
    treeState.removeParameterListener("filterType", this);

}

// PARAMETER LAYOUT
juce::AudioProcessorValueTreeState::ParameterLayout IRConvReverbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    juce::StringArray filterChoice{ "No Filter", "Low Pass", "Band Pass", "High Pass" };

    juce::NormalisableRange<float> cutOffRange (20.0f, 20000.0f);
    cutOffRange.setSkewForCentre(1000.0f);

    auto pMix = std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.5f);
    auto pCutOff = std::make_unique <juce::AudioParameterFloat>("cutoff", "CutOff", cutOffRange, 1000.0f);
    auto pResonance = std::make_unique <juce::AudioParameterFloat>("resonance", "Resonance", 1.0f, 5.0f, 1.0f);
    auto pFilterMenu = std::make_unique<juce::AudioParameterChoice>("filterType", "Filter Type", filterChoice, 0);

    params.push_back(std::move(pMix));
    params.push_back(std::move(pCutOff));
    params.push_back(std::move(pResonance));
    params.push_back(std::move(pFilterMenu));


    return { params.begin(), params.end() };
}

void IRConvReverbAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "mix")
    {
        mix = newValue;
    }
    if (parameterID == "cutoff")
    {
        cutOffFrequency = newValue;
    }
    if (parameterID == "resonance")
    {
        resonanceValue = newValue;
    }
    if (parameterID == "filterType")
    {
        filterTypeValue = static_cast<int>(newValue);
    }
}

//==============================================================================
const juce::String IRConvReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool IRConvReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool IRConvReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool IRConvReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double IRConvReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int IRConvReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int IRConvReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void IRConvReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String IRConvReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void IRConvReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void IRConvReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sampleRateCopy = sampleRate;

	spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
	spec.numChannels = getTotalNumOutputChannels();

    irLoader.reset(); // not sure why (it works)
    irLoader.prepare(spec);

    stateVariableFilter.reset();
    updateFilter();
    stateVariableFilter.prepare(spec);
}

void IRConvReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool IRConvReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void IRConvReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // dry buffer kayýt için
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    //DBG(filterTypeValue);

    juce::dsp::AudioBlock<float> block{ buffer };
    updateFilter();


    irLoader.process(juce::dsp::ProcessContextReplacing<float>(block)); // check what is processContextReplacing
    if (irLoader.getCurrentIRSize() > 1 && filterTypeValue != 0) // use filter if ir is uploaded
    {
        stateVariableFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    }

    for (int ch = 0; ch < totalNumInputChannels; ++ch) 
    {
        auto* dry = dryBuffer.getReadPointer(ch);
        auto* wet = buffer.getWritePointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            wet[i] = dry[i] * (1.0f - mix) + wet[i] * mix; // check
        }
    }
}

void IRConvReverbAudioProcessor::updateFilter()
{
    if (filterTypeValue == 1) // LOW PASS
    {
        stateVariableFilter.state->type = juce::dsp::StateVariableFilter::Parameters<float>::Type::lowPass;
        stateVariableFilter.state->setCutOffFrequency(sampleRateCopy, cutOffFrequency, resonanceValue);

    }

    if (filterTypeValue == 2) // BAND PASS
    {
        stateVariableFilter.state->type = juce::dsp::StateVariableFilter::Parameters<float>::Type::bandPass;
        stateVariableFilter.state->setCutOffFrequency(sampleRateCopy, cutOffFrequency, resonanceValue);

    }

    if (filterTypeValue == 3) // HIGH PASS
    {
        stateVariableFilter.state->type = juce::dsp::StateVariableFilter::Parameters<float>::Type::highPass;
        stateVariableFilter.state->setCutOffFrequency(sampleRateCopy, cutOffFrequency, resonanceValue);

    }
}


//==============================================================================
bool IRConvReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* IRConvReverbAudioProcessor::createEditor()
{
    return new IRConvReverbAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void IRConvReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void IRConvReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IRConvReverbAudioProcessor();
}
