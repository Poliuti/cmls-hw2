#include "PluginProcessor.h"
#include "PluginEditor.h"

struct UIelements {
    String label;
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
    UIelements ui_elements[] = {
        {"LFO frequency", {0.0, 10.0},  &FlangerProcessor::get_freqOsc,    &FlangerProcessor::set_freqOsc},
        {"Sweep width",   {0.0, 25e-3}, &FlangerProcessor::get_sweepWidth, &FlangerProcessor::set_sweepWidth},
        {"Depth",         {0.0, 1.0},   &FlangerProcessor::get_depth,      &FlangerProcessor::set_depth},
        {"Feedback",      {0.0, 1.0},   &FlangerProcessor::get_fb,         &FlangerProcessor::set_fb},
    };

    for (UIelements item : ui_elements) {
        Slider* s = new Slider();
        Label* l = new Label();
        s->setRange(item.range[0], item.range[1]);
        s->setValue((processor.*(item.get_func))());
        s->setTextBoxStyle(Slider::TextBoxRight, false, 100, 20); // cambiare numeri
        s->onValueChange = [this, s, &item] {
            auto v = s->getValue();
            (processor.*(item.set_func))(0.0f);
        };
        l->setText(item.label, dontSendNotification);
        sliders.add(s);
        labels.add(l);
        addAndMakeVisible(s);
        addAndMakeVisible(l);
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
    flow.flexWrap = FlexBox::Wrap::wrap;
    flow.justifyContent = FlexBox::JustifyContent::center;
    flow.alignContent = FlexBox::AlignContent::center;

    for (Slider* s : sliders) {
        flow.items.add(FlexItem(*s).withMinWidth(50.0f).withMinHeight(50.0f));
    }

    flow.performLayout(getLocalBounds().toFloat());
}
