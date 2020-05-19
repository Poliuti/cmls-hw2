#include "PluginProcessor.h"
#include "PluginEditor.h"

struct UISliders {
    String label, suffix;
    float range[2];
    float (FlangerProcessor::*get_func)(void);
    void (FlangerProcessor::*set_func)(float);
};

//==============================================================================
FlangerEditor::FlangerEditor(FlangerProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    // Describe GUI
    UISliders ui_elements[] = {
        {"LFO frequency", " Hz", {0.0, 10.0},  &FlangerProcessor::get_freqOsc,    &FlangerProcessor::set_freqOsc},
        {"Sweep width",   " s",  {0.0, 25e-3}, &FlangerProcessor::get_sweepWidth, &FlangerProcessor::set_sweepWidth},
        {"Depth",         " %",  {0.0, 1.0},   &FlangerProcessor::get_depth,      &FlangerProcessor::set_depth},
        {"Feedback",      " %",  {0.0, 1.0},   &FlangerProcessor::get_fb,         &FlangerProcessor::set_fb},
    };

    for (UISliders item : ui_elements) {
        Slider* s = new Slider();
        addAndMakeVisible(s);
        sliders.add(s);
        s->setRange(item.range[0], item.range[1]);
        s->setValue((processor.*(item.get_func))());
        s->setTextValueSuffix(item.suffix);
        s->setSliderStyle(Slider::SliderStyle::LinearHorizontal);
        //s->setTextBoxStyle(Slider::TextBoxRight, false, 100, 20); // cambiare numeri
        s->onValueChange = [this, s, &item] { (processor.*(item.set_func))((float)s->getValue()); };

        Label* l = new Label();
        addAndMakeVisible(l);
        l->setText(item.label, dontSendNotification);
        l->attachToComponent(s, true);
    }

    setSize(400, 300);
}

FlangerEditor::~FlangerEditor()
{
}

//==============================================================================
void FlangerEditor::paint(Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.setColour(Colours::white);
    g.setFont(16.0f);
}

void FlangerEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    FlexBox flow;
    flow.flexDirection = FlexBox::Direction::column;
    flow.flexWrap = FlexBox::Wrap::wrap;
    flow.justifyContent = FlexBox::JustifyContent::center;
    flow.alignContent = FlexBox::AlignContent::center;

    for (Slider* s : sliders) {
        flow.items.add(FlexItem(*s).withFlex(1, 0, getHeight() / sliders.size()));
    }

    flow.performLayout(getLocalBounds().toFloat());
}
