#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FlangerEditor::FlangerEditor (FlangerProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    wetSlider.setRange (0.0, 1.0);
    wetSlider.setTextBoxStyle (Slider::TextBoxRight, false, 100, 20);
    wetSlider.addListener(this);
    wetLabel.setText ("Wet Level", dontSendNotification);

    addAndMakeVisible (wetSlider);
    addAndMakeVisible (wetLabel);

    drySlider.setRange (0.0, 1.0);
    drySlider.setTextBoxStyle (Slider::TextBoxRight, false, 100, 20);
    drySlider.addListener(this);
    dryLabel.setText ("Dry Level", dontSendNotification);

    addAndMakeVisible (drySlider);
    addAndMakeVisible (dryLabel);

    timeSlider.setRange (500, 50000, 100);
    timeSlider.setTextBoxStyle (Slider::TextBoxRight, false, 100, 20);
    timeSlider.addListener(this);
    timeLabel.setText ("Time", dontSendNotification);

    addAndMakeVisible (timeSlider);
    addAndMakeVisible (timeLabel);

    setSize (400, 300);
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

    wetLabel.setBounds (10, 10, 90, 20);
    wetSlider.setBounds (100, 10, getWidth() - 110, 20);

    dryLabel.setBounds (10, 50, 90, 20);
    drySlider.setBounds (100, 50, getWidth() - 110, 20);

    timeLabel.setBounds (10, 90, 90, 20);
    timeSlider.setBounds (100, 90, getWidth() - 110, 20);
}

void FlangerEditor::sliderValueChanged(Slider *slider)
{
    if (slider == &wetSlider)
        processor.set_wet(wetSlider.getValue());
    else if (slider == &drySlider)
        processor.set_dry(drySlider.getValue());
    else if (slider == &timeSlider)
        processor.set_ds(timeSlider.getValue());
}

