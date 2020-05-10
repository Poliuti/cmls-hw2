#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FlangerEditor::FlangerEditor (FlangerProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    freqOscSlider.setRange(0.0, 3.0);
    freqOscSlider.setValue(processor.get_freqOsc());
    freqOscSlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 20);
    freqOscSlider.addListener(this);
    freqOscLabel.setText("Frequency", dontSendNotification);

    addAndMakeVisible(freqOscSlider);
    addAndMakeVisible(freqOscLabel);

    sweepWidthSlider.setRange(0.0, 25e-3);
    sweepWidthSlider.setValue(processor.get_sweepWidth());
    sweepWidthSlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 20);
    sweepWidthSlider.addListener(this);
    sweepWidthLabel.setText("Sweep Width", dontSendNotification);

    addAndMakeVisible(sweepWidthSlider);
    addAndMakeVisible(sweepWidthLabel);

    depthSlider.setRange(0.0, 1.0);
    depthSlider.setValue(processor.get_depth());
    depthSlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 20);
    depthSlider.addListener(this);
    depthLabel.setText("Depth", dontSendNotification);

    addAndMakeVisible(depthSlider);
    addAndMakeVisible(depthLabel);

    fbackSlider.setRange(0.0, 1.0);
    fbackSlider.setValue(processor.get_fb());
    fbackSlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 20);
    fbackSlider.addListener(this);
    fbackLabel.setText("Feedback", dontSendNotification);

    addAndMakeVisible(fbackSlider);
    addAndMakeVisible(fbackLabel);

    setSize(400, 300);
}

FlangerEditor::~FlangerEditor()
{
}

//==============================================================================
void FlangerEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void FlangerEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    freqOscLabel.setBounds (10, 10, 90, 20);
    freqOscSlider.setBounds (100, 10, getWidth() - 110, 20);

    sweepWidthLabel.setBounds (10, 50, 90, 20);
    sweepWidthSlider.setBounds (100, 50, getWidth() - 110, 20);

    depthLabel.setBounds (10, 90, 90, 20);
    depthSlider.setBounds (100, 90, getWidth() - 110, 20);

    fbackLabel.setBounds(10, 130, 90, 20);
    fbackSlider.setBounds(100, 130, getWidth() - 110, 20);
}

void FlangerEditor::sliderValueChanged(Slider *slider)
{
    if (slider == &freqOscSlider)
        processor.set_freqOsc(freqOscSlider.getValue());
    else if (slider == &sweepWidthSlider)
        processor.set_sweepWidth(sweepWidthSlider.getValue());
    else if (slider == &depthSlider)
        processor.set_depth(depthSlider.getValue());
    else if (slider == &fbackSlider)
        processor.set_fb(fbackSlider.getValue());
}

