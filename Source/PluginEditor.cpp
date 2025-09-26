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

    // ATTACHMENTS
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.treeState, "mix", mixSlider
    );

    cutOffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.treeState, "cutoff", cutOffSlider
    );

    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.treeState, "resonance", resonanceSlider
    );

    

    // Sliders
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    cutOffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    resonanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    //mixSlider.setTextValueSuffix(" %");
    mixSlider.textFromValueFunction = [](double value)
        {
            return juce::String(juce::roundToInt(value * 100.0)) + " %";
        };

    cutOffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    cutOffSlider.setNumDecimalPlacesToDisplay(0);
    cutOffSlider.setRange(20.0, 20000.0);
    cutOffSlider.setSkewFactorFromMidPoint(1000.0);
    cutOffSlider.setTextValueSuffix(" Hz ");

    resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

    addAndMakeVisible(mixSlider);
    addAndMakeVisible(cutOffSlider);
    addAndMakeVisible(resonanceSlider);

    // ComboBox
    filterBox.setJustificationType(juce::Justification::centred);
    filterBox.addItemList(juce::StringArray{ "No Filter", "Low Pass", "Band Pass", "High Pass" }, 1);
    addAndMakeVisible(filterBox);

    filterTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.treeState, "filterType", filterBox
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
    mixSlider.setBounds(getWidth() / 2 + 100, getHeight() / 2 - 50, 100, 100);
    cutOffSlider.setBounds(getWidth() / 2 + 10, getHeight() / 2 - 80, 100, 100);
    resonanceSlider.setBounds(getWidth() / 2 - 80, getHeight() / 2 - 110, 100, 100);
    filterBox.setBounds(getWidth() / 2 - 150, getHeight() / 2 - 140, 100, 100);

}
