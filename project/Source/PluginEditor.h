#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class FlangerEditor : public AudioProcessorEditor
{
public:
    FlangerEditor(FlangerProcessor&);
    ~FlangerEditor();

    void paint(Graphics&) override;
    void resized() override;

private:
    FlangerProcessor& processor;

    Slider freqOscSlider;
    Label freqOscLabel;
    Slider sweepWidthSlider;
    Label sweepWidthLabel;
    Slider depthSlider;
    Label depthLabel;
    Slider fbackSlider;
    Label fbackLabel;

    // === JUCE GENERATED CODE =================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlangerEditor)
};
