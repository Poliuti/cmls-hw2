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

    OwnedArray<Component> uiElements;

    // === JUCE GENERATED CODE =================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlangerEditor)
};
