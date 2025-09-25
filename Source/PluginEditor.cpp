/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
IRConvReverbAudioProcessorEditor::IRConvReverbAudioProcessorEditor (IRConvReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(loadBtn);
    loadBtn.setButtonText("Load IR");
    loadBtn.onClick = [this]()
        {
            fileChooser = std::make_unique<juce::FileChooser>("Choose File",
                audioProcessor.root,
                "*");

            const auto fileChooserFlags = juce::FileBrowserComponent::openMode |
                juce::FileBrowserComponent::canSelectFiles |
                juce::FileBrowserComponent::canSelectDirectories;

            fileChooser->launchAsync(fileChooserFlags, [this](const juce::FileChooser& chooser) // launch fileChooser
            {
                    juce::File result = chooser.getResult();

                    if (result.getFileExtension() == ".wav" | result.getFileExtension() == ".mp3")
                    {
                        audioProcessor.savedFile = result;
                        audioProcessor.root = result.getParentDirectory().getFullPathName(); 
                        audioProcessor.irLoader.reset(); //why?
                        audioProcessor.irLoader.loadImpulseResponse(result, juce::dsp::Convolution::Stereo::yes,
                            juce::dsp::Convolution::Trim::yes, 0);
                    }
            });
    };

    // Slider 
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(mixSlider);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.treeState, "mix", mixSlider
    );



    setSize (400, 300);
}

IRConvReverbAudioProcessorEditor::~IRConvReverbAudioProcessorEditor()
{
}

//==============================================================================
void IRConvReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);

}

void IRConvReverbAudioProcessorEditor::resized()
{
    const auto btnX = getWidth() * 0.35;
    const auto btnY = getHeight() * 0.5;
    const auto btnWidth = getWidth() * 0.15;
    const auto btnHeight = btnWidth * 0.5;

    loadBtn.setBounds(btnX, btnY, btnWidth, btnHeight);
    mixSlider.setBounds(getWidth() / 2 + 50, getHeight() / 2 - 50, 100, 100);
}
