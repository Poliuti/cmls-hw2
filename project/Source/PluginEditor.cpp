#include "PluginProcessor.h"
#include "PluginEditor.h"

struct UISliders {
    String label, suffix, mutex_group;
    float range[2];
    float (FlangerProcessor::*get_func)(void);
    void (FlangerProcessor::*set_func)(float);
};

std::map<int, String> wave_labels = {
    {OscFunction::sineWave     , "Sinusoid"},
    {OscFunction::squareWave   , "Square"},
    {OscFunction::sawtoothWave , "Sawtooth"},
    {OscFunction::triangleWave , "Triangle"},
    {OscFunction::inv_sawWave  , "Inverted sawtooth"},
    {OscFunction::randWave     , "Random"},
};

//==============================================================================
FlangerEditor::FlangerEditor(FlangerProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    // --- Waveform selection ---
    {
        ComboBox* wave_selector = new ComboBox();
        addAndMakeVisible(wave_selector);
        for (auto& shape_and_label : wave_labels) {
            wave_selector->addItem(shape_and_label.second, shape_and_label.first);
        }
        wave_selector->setSelectedId(processor.get_chosenWave());
        wave_selector->onChange = [this, wave_selector] {
            OscFunction shape = (OscFunction)wave_selector->getSelectedId();
            processor.set_chosenWave(shape);
            // toggle slider
            sliderGroups["phase"][0]->setVisible(shape != OscFunction::randWave);
            sliderGroups["phase"][1]->setVisible(shape == OscFunction::randWave);
            // trigger layout
            resized();
        };

        Label* l = new Label("", "LFO shape");
        addAndMakeVisible(l);
        l->attachToComponent(wave_selector, true);

        uiElements.add(wave_selector);
    }

    // --- Sliders ---
    {
        UISliders sliders[] = {
            {"LFO frequency", String::fromUTF8(" Hz"), "",      {0.1,  10.0},    &FlangerProcessor::get_freqOsc,    &FlangerProcessor::set_freqOsc},
            {"LFO phase R/L", String::fromUTF8(" °"),  "phase", {0.0,  360.0},   &FlangerProcessor::get_deltaPh,    &FlangerProcessor::set_deltaPh},
            {"LFO width",     String::fromUTF8(" %"),  "phase", {0.0,  100.0},   &FlangerProcessor::get_width,      &FlangerProcessor::set_width},
            {"Sweep width",   String::fromUTF8(" ms"), "",      {1.0,  10.0},    &FlangerProcessor::get_sweepWidth, &FlangerProcessor::set_sweepWidth},
            {"Depth",         String::fromUTF8(" %"),  "",      {0.0,  100.0},   &FlangerProcessor::get_depth,      &FlangerProcessor::set_depth},
            {"Feedback",      String::fromUTF8(" %"),  "",      {0.0,  99.0},    &FlangerProcessor::get_fb,         &FlangerProcessor::set_fb},
            {"HP cut-off",    String::fromUTF8(" Hz"), "",      {20.0, 5000.0},  &FlangerProcessor::get_fc,         &FlangerProcessor::set_fc},
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
            if (item.mutex_group != "") {
                if (sliderGroups.find(item.mutex_group) == sliderGroups.end())
                    sliderGroups[item.mutex_group] = OwnedArray<Slider> {};
                sliderGroups[item.mutex_group].add(s);
            }
        }

    }

    // -- Invert polarity ---
    {
        Button* b = new ToggleButton();
        addAndMakeVisible(b);
        b->setToggleState(processor.get_inverted(), dontSendNotification);
        b->onStateChange = [this, b] { processor.set_inverted(b->getToggleState()); };

        Label* l = new Label("", "Invert polarity");
        addAndMakeVisible(l);
        l->attachToComponent(b, true);

        uiElements.add(b);
    }

    setSize(400, 400);
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
        if (c->isVisible())
            flow.items.add(FlexItem(*c).withMinWidth(getWidth() - 120.0f).withFlex(1).withMargin(FlexItem::Margin(10, 15, 10, 15)));
    }

    flow.performLayout(getLocalBounds().toFloat());
}
