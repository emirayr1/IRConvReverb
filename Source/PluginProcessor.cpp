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
}

IRConvReverbAudioProcessor::~IRConvReverbAudioProcessor()
{
    treeState.removeParameterListener("mix", this);
}

// PARAMETER LAYOUT
juce::AudioProcessorValueTreeState::ParameterLayout IRConvReverbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto pMix = std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.5f);

    params.push_back(std::move(pMix));

    return { params.begin(), params.end() };
}

void IRConvReverbAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "mix")
    {
        mix = newValue;
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
	spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
	spec.numChannels = getTotalNumOutputChannels();

    irLoader.reset(); // not sure why (it works)
    irLoader.prepare(spec);
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

    DBG(mix);

    juce::dsp::AudioBlock<float> block{ buffer };

    if (irLoader.getCurrentIRSize() > 0) 
    {
        irLoader.process(juce::dsp::ProcessContextReplacing<float>(block)); // check what is processContextReplacing
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
