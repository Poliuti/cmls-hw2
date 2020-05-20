#pragma once

#include <sstream>
#include <iomanip>

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

    OwnedArray<Component> uiElements;

    // === JUCE GENERATED CODE =================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlangerEditor)
};

class CustomSlider : public juce::Slider
{
public:
    String getTextFromValue(double val) override {
        std::ostringstream streamObj;
        streamObj << std::fixed << std::setprecision(2) << val;
        return String(streamObj.str()) + getTextValueSuffix();
    }
};
