#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class DelayLineAudioProcessorEditor : public AudioProcessorEditor,  private Slider::Listener
{
public:
    DelayLineAudioProcessorEditor (DelayLineAudioProcessor&);
    ~DelayLineAudioProcessorEditor();

    void paint(Graphics&) override;
    void resized() override;

private:
    DelayLineAudioProcessor& processor;

    Slider wetSlider;
    Label wetLabel;
    Slider drySlider;
    Label dryLabel;
    Slider timeSlider;
    Label timeLabel;

    void sliderValueChanged(Slider* slider) override;

    // === JUCE GENERATED CODE =================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayLineAudioProcessorEditor)
};
