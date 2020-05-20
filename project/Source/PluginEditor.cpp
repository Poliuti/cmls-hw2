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
        {"LFO frequency", String::fromUTF8(" Hz"), {0.0, 10.0},  &FlangerProcessor::get_freqOsc,    &FlangerProcessor::set_freqOsc},
        {"Phase R-L",     String::fromUTF8(" °"),  {0.0, 360.0}, &FlangerProcessor::get_deltaPh,    &FlangerProcessor::set_deltaPh},
        {"Sweep width",   String::fromUTF8(" ms"), {0.0, 25.0},  &FlangerProcessor::get_sweepWidth, &FlangerProcessor::set_sweepWidth},
        {"Depth",         String::fromUTF8(" %"),  {0.0, 100.0}, &FlangerProcessor::get_depth,      &FlangerProcessor::set_depth},
        {"Feedback",      String::fromUTF8(" %"),  {0.0, 99.9},  &FlangerProcessor::get_fb,         &FlangerProcessor::set_fb},
    };

    for (UISliders item : sliders) {
        Slider* s = new CustomSlider();
        addAndMakeVisible(s);
        s->setRange(item.range[0], item.range[1]);
        s->setValue((processor.*(item.get_func))());
        s->setTextValueSuffix(item.suffix);
        s->setSliderStyle(Slider::SliderStyle::LinearHorizontal);
        s->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
        s->onValueChange = [this, s, item] { (processor.*(item.set_func))(s->getValue()); };

        Label* l = new Label("", item.label);
        addAndMakeVisible(l);
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

    Label* l = new Label("", "LFO shape");
    addAndMakeVisible(l);
    l->attachToComponent(wave_selector, true);

    uiElements.add(wave_selector);

    setSize(400, 300);
}

FlangerEditor::~FlangerEditor()
{
    /* for (Component* c : uiElements) {
        delete c; // TODO: are labels deleted?
    } */
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
    flow.justifyContent = FlexBox::JustifyContent::flexStart;
    flow.alignContent = FlexBox::AlignContent::flexEnd;

    for (Component* c : uiElements) {
        // TODO: fix slider width not taking into account the label
        flow.items.add(FlexItem(*c).withMinWidth(getWidth() - 120.0f).withFlex(1).withMargin(FlexItem::Margin(0, 15, 10, 0)));
    }

    flow.performLayout(getLocalBounds().toFloat());
}
