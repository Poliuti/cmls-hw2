#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class FlangerProcessor  : public AudioProcessor
{
public:
    // === RELEVANT METHODS ====================================================
    FlangerProcessor();
    ~FlangerProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override; // per inizializzare il processor
    void releaseResources() override;

    void processBlock(AudioBuffer<float>&, MidiBuffer&) override; // AudioBuffer contiene sia input che output, processa l'audio

    // === OTHER MEMBERS =======================================================
    void set_freqOsc(float val);
    void set_sweepWidth(float val);
    void set_depth(float val);
    void set_fb(float val);


    /// === JUCE GENERATED CODE ================================================
   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
   #endif
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName(int index, const String& newName) override;
    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlangerProcessor)

    // === OUR PRIVATE MEMBERS =================================================
    AudioSampleBuffer dbuf; // delay buffer
    int dw; // writing head
    float ph; // phase LFO

    float freqOsc; // Frequency LFO
    float sweepWidth; // Width LFO in samples (campioni di ritardo)
    float depth; // Depth Flanger (0 - 1)
    float fb; // feedback

};
