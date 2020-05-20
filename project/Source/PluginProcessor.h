#pragma once

#include <JuceHeader.h>

enum OscFunction : int { sineWave = 1, squareWave, sawtoothWave, triangleWave, inv_sawWave, randWave };

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
    float waveForm(float phi, OscFunction waveform);
    float interpolate(float dr, int delayBufLength, float* delay);

    void set_chosenWave(OscFunction val);
    OscFunction get_chosenWave(void);

    void set_freqOsc(float val);
    float get_freqOsc(void);
    void set_sweepWidth(float val);
    float get_sweepWidth(void);
    void set_depth(float val);
    float get_depth(void);
    void set_fb(float val);
    float get_fb(void);


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

    OscFunction chosenWave;
    float phtmp;
    float rnd;
    float deltaPh; // Frequency LFO

    float freqOsc; // Frequency LFO
    float sweepWidth; // Width LFO in samples (campioni di ritardo)
    float depth; // Depth Flanger (0 - 1)
    float fb; // feedback
};
