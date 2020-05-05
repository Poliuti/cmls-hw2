#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class FlangerEditor : public AudioProcessorEditor,  private Slider::Listener
{
public:
    FlangerEditor(FlangerProcessor&);
    ~FlangerEditor();

    void paint(Graphics&) override;
    void resized() override;

private:
    FlangerProcessor& processor;

    Slider wetSlider;
    Label wetLabel;
    Slider drySlider;
    Label dryLabel;
    Slider timeSlider;
    Label timeLabel;

    void sliderValueChanged(Slider* slider) override;

    // === JUCE GENERATED CODE =================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlangerEditor)
};
