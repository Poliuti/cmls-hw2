#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <math.h>
#include <cstdlib>
#include <ctime>
#include <string>

// === OUR CODE ================================================================

FlangerProcessor::FlangerProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    freqOsc = 0.0f;
    sweepWidth = 0.0f;
    depth = 0.0f;
    fb = 0.0f;
}

FlangerProcessor::~FlangerProcessor()
{
}

void FlangerProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    dbuf.setSize(getTotalNumOutputChannels(), 100000);
    dbuf.clear();
    dwL = 0;
    dwR = 0;
    ph = 0;
    phR = 0;
}

void FlangerProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

float FlangerProcessor::waveForm(float ph, OscFunction chosenWave)
{
    switch(chosenWave)
 {
    case OscFunction::sineWave:
    return 0.5f + 0.5f * sinf(2.0 * M_PI * ph);

    case OscFunction::squareWave:
       float sqr;
         if(ph!=0)
             sqr = 0.5f + 0.5f * abs(sinf(2.0 * M_PI * ph))/sinf(2.0 * M_PI * ph);
         else
             sqr = 0.5f;
    return sqr;

    case OscFunction::sawtoothWave:
    return 1 - (ph - floor(ph));

    case OscFunction::triangleWave:
         float tri;
         if(ph - floor(ph) < 0.5) tri = 2*(ph - floor(ph));
         else tri = 2*(1-ph - floor(ph));
    return  tri;

    case OscFunction::inv_sawWave:
    return ph - floor(ph);

    case OscFunction::randWave:
       //srand ((unsigned int) (time(NULL)));
       if(ph - phtmp < 0) rnd = rand()%100;
    return rnd/100;
 }
}

void FlangerProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; i++)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...

    int numSamples = buffer.getNumSamples();
    int delayBufLength = dbuf.getNumSamples();
    chosenWave = OscFunction::squareWave;
    OscFunction chosenWave_now = chosenWave;
    float freqOsc_now = freqOsc;
    float sweepWidth_now = sweepWidth;
    float fb_now = fb;
    float depth_now = depth;
    float deltaPh_now = deltaPh;
    deltaPh_now = 0.5;

    float* channelOutDataL = buffer.getWritePointer(0);
    float* channelOutDataR = buffer.getWritePointer(1);
    float* delayL = dbuf.getWritePointer(0);
    float* delayR = dbuf.getWritePointer(1);

    const float* channelInData = buffer.getReadPointer(0);

    for (int i = 0; i < numSamples; i++) {
        const float in = channelInData[i];

        // Recalculate the read pointer position with respect to
        // the write pointer.
        float currentDelayL = sweepWidth_now * waveForm(ph, chosenWave_now);
        phR = ph + deltaPh_now;
        float currentDelayR = sweepWidth_now * waveForm(phR, chosenWave_now);

        // Subtract 3 samples to the delay pointer to make sure
        // we have enough previous samples to interpolate with
        float drL = fmodf((float)dwL - (float)(currentDelayL * getSampleRate()) + (float)delayBufLength - 3.0, (float)delayBufLength);
         float drR = fmodf((float)dwR - (float)(currentDelayR * getSampleRate()) + (float)delayBufLength - 3.0, (float)delayBufLength);
        // (N + K) % K == N % K
        // -3 % 10 = -3 (risultato di c++) // 7 (risultato matematico)
        //                     ↓
        // [-9 -8 -7 -6 -5 -4 -3 -2 -1 0]
        // [1   2  3  4  5  6  7  8  9 10] ← classi di equivalenza
        // [11 12 13 14 15 16 17 18 19 20]

        // Use linear interpolation to read a fractional index
        // into the buffer.
        float fractionL = drL - floorf(drL);
        int previousSampleL = (int)floorf(drL);
        int nextSampleL = (previousSampleL + 1) % delayBufLength;
        float interpolatedSampleL = fractionL*delayL[nextSampleL] + (1.0f-fractionL)*delayL[previousSampleL];

        float fractionR = drR - floorf(drR);
        int previousSampleR = (int)floorf(drR);
        int nextSampleR = (previousSampleR + 1) % delayBufLength;
        float interpolatedSampleR = fractionR*delayR[nextSampleR] + (1.0f-fractionR)*delayR[previousSampleR];
        // Store the current information in the delay buffer.
        // With feedback, what we read is included in what gets
        // stored in the buffer, otherwise it’s just a simple
        // delay line of the input signal.
        delayL[dwL] = in + (interpolatedSampleL * fb_now);
        delayR[dwR] = in + (interpolatedSampleR * fb_now);

        dwL = (dwL + 1) % delayBufLength;
        dwR = (dwR + 1) % delayBufLength;

        channelOutDataL[i] = in + depth_now * interpolatedSampleL;
        channelOutDataR[i] = in + depth_now * interpolatedSampleR;
        // Update the LFO phase, keeping it in the range 0-1
        phtmp = ph; //per l'onda random
        ph += freqOsc_now / getSampleRate();
        if (ph >= 1.0) ph -= 1.0;

        //phtmp = phR; //per l'onda random
        //phR += freqOsc_now / getSampleRate();
        //if (phR >= 1.0 + deltaPh_now) phR -= 1.0;

    }
}


void FlangerProcessor::set_chosenWave(OscFunction val)
{
    chosenWave = val;
}

OscFunction FlangerProcessor::get_chosenWave(void) {
    return chosenWave;
}

void FlangerProcessor::set_freqOsc(float val)
{
    freqOsc = val;
}

float FlangerProcessor::get_freqOsc(void) {
    return freqOsc;
}

void FlangerProcessor::set_sweepWidth(float val)
{
    sweepWidth = val;
}

float FlangerProcessor::get_sweepWidth(void) {
    return sweepWidth;
}

void FlangerProcessor::set_depth(float val)
{
    depth = val;
}

float FlangerProcessor::get_depth(void) {
    return depth;
}

void FlangerProcessor::set_fb(float val)
{
    fb = val;
}

float FlangerProcessor::get_fb(void) {
    return fb;
}








// === JUCE GENERATED CODE =====================================================

//==============================================================================
const String FlangerProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FlangerProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FlangerProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FlangerProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FlangerProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FlangerProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FlangerProcessor::getCurrentProgram()
{
    return 0;
}

void FlangerProcessor::setCurrentProgram(int index)
{
}

const String FlangerProcessor::getProgramName(int index)
{
    return {};
}

void FlangerProcessor::changeProgramName(int index, const String& newName)
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FlangerProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================

bool FlangerProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* FlangerProcessor::createEditor()
{
    return new FlangerEditor(*this);
}

void FlangerProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FlangerProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FlangerProcessor();
}
