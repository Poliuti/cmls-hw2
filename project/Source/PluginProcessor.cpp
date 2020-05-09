#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <math.h>

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

    float freqOsc_now = freqOsc;
    float sweepWidth_now = sweepWidth;
    float fb_now = fb;
    int depth_now = depth;

    float* channelOutDataL = buffer.getWritePointer(0);
    float* channelOutDataR = buffer.getWritePointer(1);
    float* delay = dbuf.getWritePointer(0);

    const float* channelInData = buffer.getReadPointer(0);

    for (int i = 0; i < numSamples; i++) {
        const float in = channelInData[i];

        // Recalculate the read pointer position with respect to
        // the write pointer.
        float currentDelay = sweepWidth_now * (0.5f + 0.5f * sinf(2.0 * M_PI * ph));

        // Subtract 3 samples to the delay pointer to make sure
        // we have enough previous samples to interpolate with
        float dr = fmodf((float)dw - (float)(currentDelay * getSampleRate()) + (float)delayBufLength - 3.0, (float)delayBufLength);
        // (N + K) % K == N % K
        // -3 % 10 = -3 (risultato di c++) // 7 (risultato matematico)
        //                     ↓
        // [-9 -8 -7 -6 -5 -4 -3 -2 -1 0]
        // [1   2  3  4  5  6  7  8  9 10] ← classi di equivalenza
        // [11 12 13 14 15 16 17 18 19 20]

        // Use linear interpolation to read a fractional index // into the buffer.
        float fraction = dr - floorf(dr);
        int previousSample = (int)floorf(dr);
        int nextSample = (previousSample + 1) % delayBufLength;
        float interpolatedSample = fraction*delay[nextSample] + (1.0f-fraction)*delay[previousSample];

        // Store the current information in the delay buffer.
        // With feedback, what we read is included in what gets
        // stored in the buffer, otherwise it’s just a simple
        // delay line of the input signal.
        delay[dw] = in + (interpolatedSample * fb_now);

        dw = (dw + 1) % delayBufLength;

        channelOutDataL[i] = in + depth_now * interpolatedSample;
        channelOutDataR[i] = channelOutDataL[i];
        // Update the LFO phase, keeping it in the range 0-1
        ph += freqOsc_now / getSampleRate();
        if (ph >= 1.0) ph -= 1.0;
    }
}

void FlangerProcessor::set_freqOsc(float val)
{
    freqOsc = val;
}

void FlangerProcessor::set_sweepWidth(float val)
{
    sweepWidth = val;
}

void FlangerProcessor::set_depth(float val)
{
    depth = val;
}

void FlangerProcessor::set_fb(float val)
{
    fb = val;
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
