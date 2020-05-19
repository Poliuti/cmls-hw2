#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <math.h>
#include <cstdlib>
#include <ctime>
#include <string>

// === OUR CODE ================================================================

void FlangerProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    dbuf.setSize(getTotalNumOutputChannels(), 100000);
    dbuf.clear();
    dw = 0;
    ph = 0;
}

void FlangerProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

float FlangerProcessor::waveForm(float ph, oscFunction chosenWave)
{
    switch(chosenWave)
 {
    case sineWave:
    return 0.5f + 0.5f * sinf(2.0 * M_PI * ph);
     
    case squareWave:
       float sqr;
         if(ph!=0)
             sqr = 0.5f + 0.5f * abs(sinf(2.0 * M_PI * ph))/sinf(2.0 * M_PI * ph);
         else
             sqr = 0.5f;
    return sqr;
    
    case sawtoothWave:
    return 1 - (ph - floorf(ph));
         
    case triangleWave:
         float tri;
         if(ph - floorf(ph) < 0.5) tri = 2*(ph - floorf(ph));
         else tri = 2*(1-ph - floorf(ph));
    return  tri;
    
    case inv_sawWave:
    return ph - floorf(ph);

    case randWave:
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
    chosenWave = squareWave;
    oscFunction chosenWave_now = chosenWave;
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
        float currentDelayR = sweepWidth_now * waveForm(ph + deltaPh_now, chosenWave_now);

        // Subtract 3 samples to the delay pointer to make sure
        // we have enough previous samples to interpolate with
        float drL = fmodf((float)dw - (float)(currentDelayL * getSampleRate()) + (float)delayBufLength - 3.0, (float)delayBufLength);
         float drR = fmodf((float)dw - (float)(currentDelayR * getSampleRate()) + (float)delayBufLength - 3.0, (float)delayBufLength);
        // (N + K) % K == N % K
        // -3 % 10 = -3 (risultato di c++) // 7 (risultato matematico)
        //                     ↓
        // [-9 -8 -7 -6 -5 -4 -3 -2 -1 0]
        // [1   2  3  4  5  6  7  8  9 10] ← classi di equivalenza
        // [11 12 13 14 15 16 17 18 19 20]

        // Use linear interpolation to read a fractional index
        // into the buffer.
        //float fractionL = drL - floorf(drL);
        //int previousSampleL = (int)floorf(drL);
        //int nextSampleL = (previousSampleL + 1) % delayBufLength;
        //float interpolatedSampleL = fractionL*delayL[nextSampleL] + (1.0f-fractionL)*delayL[previousSampleL];

        //float fractionR = drR - floorf(drR);
        //int previousSampleR = (int)floorf(drR);
        //int nextSampleR = (previousSampleR + 1) % delayBufLength;
        //float interpolatedSampleR = fractionR*delayR[nextSampleR] + (1.0f-fractionR)*delayR[previousSampleR];
        
        
        // POLINOMIAL 2nd order INTERPOLATION
        // into the buffer.
        int nextSampleL = (int)floorf(drL);                                             // y[0]
        int next_nextSampleL = (nextSampleL + 1) % delayBufLength;                      // y[1]
        int previousSampleL = (nextSampleL - 1 + delayBufLength) % delayBufLength;      // y[-1]
        float fractionL = drL - floorf(drL);
        float c0L = delayL[nextSampleL];
        float c1L = (delayL[next_nextSampleL] - delayL[previousSampleL]) / 2;
        float c2L = (delayL[next_nextSampleL] - (2 * delayL[nextSampleL]) + delayL[previousSampleL]) / 2;
        float frac2L = fractionL * fractionL;
        float interpolatedSampleL = (c2L * frac2L) + (c1L * fractionL) + c0L;
        
        int nextSampleR = (int)floorf(drR);                                             // y[0]
        int next_nextSampleR = (nextSampleR + 1) % delayBufLength;                      // y[1]
        int previousSampleR = (nextSampleR - 1 + delayBufLength) % delayBufLength;      // y[-1]
        float fractionR = drR - floorf(drR);
        float c0R = delayR[nextSampleR];
        float c1R = (delayR[next_nextSampleR] - delayR[previousSampleR]) / 2;
        float c2R = (delayR[next_nextSampleR] - (2 * delayR[nextSampleR]) + delayR[previousSampleR]) / 2;
        float frac2R = fractionR * fractionR;
        float interpolatedSampleR = (c2R * frac2R) + (c1R * fractionR) + c0R;
        
        
        // POLINOMIAL 3rd order INTERPOLATION
        // into the buffer.
        //int prev_previousSampleL = (int)floorf(drL)-1 % delayBufLength;     // x[n-1]
        //int previousSampleL = (int)floorf(drL);                             // x[n]
        //int nextSampleL = (previousSampleL + 1) % delayBufLength;           // x[n+1]
        //int next_nextSampleL = (previousSampleL + 2) % delayBufLength;      // x[n+2]
        //float fractionL = drL - floorf(drL);
        //float c0L = delayL[previousSampleL];
        //float c1L = delayL[nextSampleL] - delayL[prev_previousSampleL];
        //float c2L = delayL[prev_previousSampleL] - delayL[previousSampleL] - 1;  //a0=???
        //float c3L = -delayL[prev_previousSampleL] + delayL[previousSampleL] - delayL[nextSampleL] + delayL[next_nextSampleL];
        //float frac2L = fractionL * fractionL;
        //float frac3L = frac2L * fractionL;
        //float interpolatedSampleL = (c3L*frac3L) + (c2L*frac2L) + (c1L*fractionL) + c0L;
        
        //int prev_previousSampleR = (int)floorf(drR)-1% delayBufLength;      // x[n-1]
        //int previousSampleR = (int)floorf(drR);                             // x[n]
        //int nextSampleR = (previousSampleR + 1) % delayBufLength;           // x[n+1]
        //int next_nextSampleR = (previousSampleR + 2) % delayBufLength;      // x[n+2]
        //float fractionR = drR - floorf(drR);
        //float c0R = delayR[previousSampleR];
        //float c1R = delayR[nextSampleR] - delayR[prev_previousSampleR];
        //float c2R = delayR[prev_previousSampleR] - delayR[previousSampleR] - 1;
        //float c3R = -delayR[prev_previousSampleR] + delayR[previousSampleR] - delayR[nextSampleR] + delayR[next_nextSampleR];
        //float frac2R = fractionR * fractionR;
        //float frac3R = frac2R * fractionR;
        //float interpolatedSampleR = (c3R*frac3R) + (c2R*frac2R) + (c1R*fractionR) + c0R;

        
        
        // Store the current information in the delay buffer.
        // With feedback, what we read is included in what gets
        // stored in the buffer, otherwise it’s just a simple
        // delay line of the input signal.
        delayL[dw] = in + (interpolatedSampleL * fb_now);
        delayR[dw] = in + (interpolatedSampleR * fb_now);

        dw = (dw + 1) % delayBufLength;

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


void FlangerProcessor::set_chosenWave(oscFunction val)
{
    chosenWave = val;
}

FlangerProcessor::oscFunction FlangerProcessor::get_chosenWave(void) {
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
}

FlangerProcessor::~FlangerProcessor()
{
}

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
