#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    deltaPh = 0.0f;
    freqOsc = 0.0f;
    sweepWidth = 0.0f;
    depth = 0.0f;
    fb = 0.0f;
    sign = +1;
    alpha = 1;
    chosenWave = OscFunction::sineWave;
}

FlangerProcessor::~FlangerProcessor()
{
}

void FlangerProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    dbuf.setSize(getTotalNumOutputChannels(), (int)ceilf(sampleRate * 25e-3));
    dbuf.clear();
    dw = 0;
    ph = 0;
    xp = 0;
    yp = 0;
    srand ((unsigned int)time(NULL));
}

void FlangerProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

float FlangerProcessor::waveForm(float ph, OscFunction chosenWave, float deltaphi)
{
    static float rndL, rndR, frozen_deltaphi;

    switch(chosenWave)
    {
        case OscFunction::sineWave:
            return 0.5f + 0.5f * sinf(2.0 * M_PI * ph);

        case OscFunction::squareWave:
            return 0.5f + (ph == 0 ? 0 : 0.5f * abs(sinf(2.0 * M_PI * ph)) / sinf(2.0 * M_PI * ph));

        case OscFunction::sawtoothWave:
            return 1 - (ph - floorf(ph));

        case OscFunction::inv_sawWave:
            return ph - floorf(ph);

        case OscFunction::triangleWave:
            float tri;
            if (ph - floorf(ph) < 0.5)
                tri = 2 * (ph - floorf(ph));
            else
                tri = 2 * (1-ph - floorf(ph));
            return tri;

        case OscFunction::randWave:
            //srand ((unsigned int) (time(NULL)));
            if (ph < phtmp) {
                frozen_deltaphi = deltaphi;
                rndL = (float)rand() / RAND_MAX;
                rndR = (float)rand() / RAND_MAX;
            }
            deltaphi = (deltaphi ? frozen_deltaphi : 0); // rewrite deltaphi with frozen version
            return (1 - deltaphi) * rndL + (deltaphi) * rndR;
    }
}

float FlangerProcessor::interpolate(float dr, int delayBufLength, float* delay)
{
    // POLYNOMIAL 2nd order INTERPOLATION
    int nextSample = (int)floorf(dr);                                             // y[0]
    int next_nextSample = (nextSample + 1) % delayBufLength;                      // y[1]
    int previousSample = (nextSample - 1 + delayBufLength) % delayBufLength;      // y[-1]
    float fraction = dr - floorf(dr);
    float c0 = delay[nextSample];
    float c1 = (delay[next_nextSample] - delay[previousSample]) / 2;
    float c2 = (delay[next_nextSample] - (2 * delay[nextSample]) + delay[previousSample]) / 2;
    float frac2 = fraction * fraction;
    float interpolatedSample = (c2 * frac2) + (c1 * fraction) + c0;


    // POLINOMIAL 3rd order INTERPOLATION
    /*
    int prev_previousSample = (int)floorf(dr)-1 % delayBufLength;     // x[n-1]
    int previousSample = (int)floorf(dr);                             // x[n]
    int nextSample = (previousSample + 1) % delayBufLength;           // x[n+1]
    int next_nextSample = (previousSample + 2) % delayBufLength;      // x[n+2]
    float fraction = dr - floorf(dr);
    float c0 = delay[previousSample];
    float c1 = delay[nextSample] - delay[prev_previousSample];
    float c2 = delay[prev_previousSample] - delay[previousSample] - 1;  //a0=???
    float c3 = -delay[prev_previousSample] + delay[previousSample] - delay[nextSample] + delay[next_nextSample];
    float frac2 = fraction * fraction;
    float frac3 = frac2 * fraction;
    float interpolatedSample = (c3*frac3) + (c2*frac2) + (c1*fraction) + c0;
    */

    return interpolatedSample;
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
    OscFunction chosenWave_now = chosenWave;
    float freqOsc_now = freqOsc;
    float sweepWidth_now = sweepWidth;
    float fb_now = sign * fb;
    float depth_now = depth;
    float deltaPh_now = deltaPh;

    float* channelOutDataL = buffer.getWritePointer(0);
    float* channelOutDataR = buffer.getWritePointer(1);
    float* delayL = dbuf.getWritePointer(0);
    float* delayR = dbuf.getWritePointer(1);

    const float* channelInData = buffer.getReadPointer(0);

    for (int i = 0; i < numSamples; i++) {
        const float in = channelInData[i];
        // HP filter
        const float filt_in = alpha * (yp + in - xp);
        xp = in; // for next iteration
        yp = filt_in; // for next iteration

        // Recalculate the read pointer position with respect to
        // the write pointer.
        float currentDelayL = sweepWidth_now * waveForm(ph, chosenWave_now, 0);
        // Subtract 4 samples to the delay pointer to make sure
        // we have enough previous samples to interpolate with
        float drL = fmodf((float)dw - (float)(currentDelayL * getSampleRate()) + (float)delayBufLength - 4.0, (float)delayBufLength);
        // POLINOMIAL 2nd order INTERPOLATION
        // into the buffer.
        float interpolatedSampleL = interpolate(drL, delayBufLength, delayL);
        // Store the current information in the delay buffer.
        // With feedback, what we read is included in what gets
        // stored in the buffer, otherwise it’s just a simple
        // delay line of the input signal.
        delayL[dw] = filt_in + (interpolatedSampleL * fb_now);
        channelOutDataL[i] = in + depth_now * interpolatedSampleL;

        float currentDelayR = sweepWidth_now * waveForm(ph + deltaPh_now, chosenWave_now, deltaPh_now);
        float drR = fmodf((float)dw - (float)(currentDelayR * getSampleRate()) + (float)delayBufLength - 4.0, (float)delayBufLength);
        float interpolatedSampleR = interpolate(drR, delayBufLength, delayR);
        delayR[dw] = filt_in + (interpolatedSampleR * fb_now);
        channelOutDataR[i] = in + depth_now * interpolatedSampleR;

        dw = (dw + 1) % delayBufLength;
        // Update the LFO phase, keeping it in the range 0-1
        phtmp = ph; //per l'onda random
        ph += freqOsc_now / getSampleRate();
        if (ph >= 1.0) ph -= 1.0;
    }
}

void FlangerProcessor::set_fc(float val) {
    alpha = 1 / (2 * M_PI * (val/getSampleRate()) + 1);
}

float FlangerProcessor::get_fc(void) {
    return getSampleRate() * (1 / alpha - 1) / (2 * M_PI);
}

void FlangerProcessor::set_inverted(bool val) {
    sign = val ? -1 : +1;
}

bool FlangerProcessor::get_inverted(void) {
    return sign == -1;
}

void FlangerProcessor::set_deltaPh(float val) {
    deltaPh = val / 360.0f;
}

float FlangerProcessor::get_deltaPh(void) {
    return deltaPh * 360.0f;
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
    sweepWidth = val / 1000.0f;
}

float FlangerProcessor::get_sweepWidth(void) {
    return sweepWidth * 1000.0f;
}

void FlangerProcessor::set_depth(float val)
{
    depth = val / 100.0f;
}

float FlangerProcessor::get_depth(void) {
    return depth * 100.0f;
}

void FlangerProcessor::set_fb(float val)
{
    fb = val / 100.0f;
}

float FlangerProcessor::get_fb(void) {
    return fb * 100.0f;
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
