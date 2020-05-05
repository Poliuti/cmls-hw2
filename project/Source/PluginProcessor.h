#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class DelayLineAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    DelayLineAudioProcessor();
    ~DelayLineAudioProcessor();

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override; // per inizializzare il processor
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock(AudioBuffer<float>&, MidiBuffer&) override; // AudioBuffer contiene sia input che output, processa l'audio

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;


    // LA ROBA NOSTRA //
    void set_wet(float val);
    void set_dry(float val);
    void set_ds(int val);

private:
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayLineAudioProcessor)

    //********************************************************************************************//
    // LA ROBA NOSTRA
    AudioSampleBuffer dbuf, fb_buf;
    int dw;

    float wet;
    float dry;
    float fb;
    int ds;
    //********************************************************************************************//


};
