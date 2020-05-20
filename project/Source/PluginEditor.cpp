#include "PluginProcessor.h"
#include "PluginEditor.h"

struct UISliders {
    String label, suffix;
    float range[2];
    float (FlangerProcessor::*get_func)(void);
    void (FlangerProcessor::*set_func)(float);
};

String wave_labels[10] = {
    [0] = "-- Select --",
    [OscFunction::sineWave]     = "Sinusoid",
    [OscFunction::squareWave]   = "Square",
    [OscFunction::sawtoothWave] = "Sawtooth",
    [OscFunction::triangleWave] = "Triangle",
    [OscFunction::inv_sawWave]  = "Inverted sawtooth",
    [OscFunction::randWave]     = "Random",
};

//==============================================================================
FlangerEditor::FlangerEditor(FlangerProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    // --- Sliders ---
    UISliders sliders[] = {
        {"LFO frequency", " Hz", {0.0, 10.0},  &FlangerProcessor::get_freqOsc,    &FlangerProcessor::set_freqOsc},
        {"Sweep width",   " s",  {0.0, 25e-3}, &FlangerProcessor::get_sweepWidth, &FlangerProcessor::set_sweepWidth},
        {"Depth",         " %",  {0.0, 1.0},   &FlangerProcessor::get_depth,      &FlangerProcessor::set_depth},
        {"Feedback",      " %",  {0.0, 1.0},   &FlangerProcessor::get_fb,         &FlangerProcessor::set_fb},
    };

    for (UISliders item : sliders) {
        Slider* s = new Slider();
        addAndMakeVisible(s);
        s->setRange(item.range[0], item.range[1]);
        s->setValue((processor.*(item.get_func))());
        s->setTextValueSuffix(item.suffix);
        s->setSliderStyle(Slider::SliderStyle::LinearHorizontal);
        s->setTextBoxStyle(Slider::TextBoxRight, false, 100, 20); // cambiare numeri
        s->onValueChange = [this, s, item] { (processor.*(item.set_func))(s->getValue()); };

        Label* l = new Label();
        addAndMakeVisible(l);
        l->setText(item.label, dontSendNotification);
        l->attachToComponent(s, true);

        uiElements.add(s);
    }

    // --- Waveform selection ---
    OscFunction shapes[] = {
        OscFunction::sineWave,
        OscFunction::squareWave,
        OscFunction::sawtoothWave,
        OscFunction::triangleWave,
        OscFunction::inv_sawWave,
        OscFunction::randWave,
    };

    ComboBox* wave_selector = new ComboBox();
    addAndMakeVisible(wave_selector);
    for (OscFunction shape : shapes) {
        wave_selector->addItem(wave_labels[shape], shape);
    }
    wave_selector->setSelectedId(processor.get_chosenWave());
    wave_selector->onChange = [this, wave_selector] { processor.set_chosenWave((OscFunction)wave_selector->getSelectedId()); };

    uiElements.add(wave_selector);

    setSize(400, 300);
}

FlangerEditor::~FlangerEditor()
{
    for (Component* c : uiElements) {
        delete c; // TODO: are labels deleted?
    }
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

    for (Component* s : uiElements) {
        // TODO: fix slider width not taking into account the label
        flow.items.add(FlexItem(*s).withMinWidth(getWidth() / 2).withMinHeight((float)getHeight() / (uiElements.size() + 1)));
    }

    flow.performLayout(getLocalBounds().toFloat());
}
