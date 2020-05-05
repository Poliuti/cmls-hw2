#include "PluginProcessor.h"
#include "PluginEditor.h"

// === OUR CODE ================================================================

void FlangerProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    dbuf.setSize(getTotalNumOutputChannels(), 100000);
    dbuf.clear();
    dw = 0;
    ds = 50000;
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
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());


    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...

    int numSamples = buffer.getNumSamples();

    float wet_now = wet;
    float dry_now = dry;
    float fb_now = fb;
    int ds_now = ds;

    float* channelOutDataL = buffer.getWritePointer(0);
    float* channelOutDataR = buffer.getWritePointer(1);
    float* delay = dbuf.getWritePointer(0);

    const float* channelInData = buffer.getReadPointer(0);
    //              ↓ [4, 5, 6, 7]
    // [0, 1, 2, 3, 4, 5, 6, 7]

    for (int i = 0; i < numSamples; ++i) {
        int dr = (dw + 1) % ds_now; // read one sample ahead
        const float in = channelInData[i];

        float out = dry_now * in + wet_now * delay[dr];
        delay[dw] = in + (delay[dr] * fb_now);

        channelOutDataL[i] = out;
        channelOutDataR[i] = out;

        dw = dr; // move writing head one sample ahead
    }
}

void FlangerProcessor::set_wet(float val)
{
    wet = val;
}
void FlangerProcessor::set_dry(float val)
{
    dry = val;
}
void FlangerProcessor::set_ds(int val)
{
    ds = val;
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
