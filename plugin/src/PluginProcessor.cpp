#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec {
        sampleRate,
        static_cast<juce::uint32> (samplesPerBlock),
        static_cast<juce::uint32> (getTotalNumOutputChannels())
    };

    inputCoupling.prepare (spec);
    v1bStage.prepare (spec);
    v1bInterstage.prepare (spec);
    v1aStage.prepare (spec);
    v1aInterstage.prepare (spec);
    v2bStage.prepare (spec);
    levelContour.prepare (spec);
    v2aStage.prepare (spec);
}

void AudioPluginAudioProcessor::releaseResources()
{
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Apply input gain and noise gate
    // High-gain amp stages amplify even -96dB noise to 0dB,
    // so gate the input to prevent amplifying the DAW noise floor.
    static constexpr float gateThreshold = 1e-6f; // ~ -120 dBFS
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float s = channelData[sample] * inputGain;
            channelData[sample] = (std::abs (s) < gateThreshold) ? 0.0f : s;
        }
    }

    // Preamp signal chain: V1B → V1A → V2B → Level/Contour → V2A
    auto block = juce::dsp::AudioBlock<float> (buffer);
    inputCoupling.process (block);
    v1bStage.process (block);
    v1bInterstage.process (block);
    v1aStage.process (block);
    v1aInterstage.process (block);
    v2bStage.process (block);
    levelContour.process (block);
    v2aStage.process (block);

    // Output trim — compensate for cumulative gain through the preamp chain
    static constexpr float outputTrimDb = -27.0f;
    static const float outputTrimGain = std::pow (10.0f, outputTrimDb / 20.0f);
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            channelData[sample] *= outputTrimGain;
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}

void AudioPluginAudioProcessor::setLowContour (float normalised)
{
    // normalised: 0.0 = full bass boost (pot at 0Ω), 1.0 = no extra bypass (pot at 50kΩ)
    float potR = normalised * static_cast<float> (AngryToddStages::V1B().RpotMax);
    v1bStage.setCathodePotResistance (potR);
}

void AudioPluginAudioProcessor::setBoosterFat (float normalised)
{
    boosterFatNormalised = normalised;
    updateV2bCathodePot();
}

void AudioPluginAudioProcessor::setBoostSwitch (bool engaged)
{
    boostEngaged = engaged;
    updateV2bCathodePot();
}

void AudioPluginAudioProcessor::setLevel (float normalised)
{
    levelContour.setLevel (normalised);
}

void AudioPluginAudioProcessor::setHighContour (float normalised)
{
    levelContour.setHighContour (normalised);
}

void AudioPluginAudioProcessor::updateV2bCathodePot()
{
    if (boostEngaged)
    {
        // BOOST ON: P2 controls the bass boost amount
        float potR = boosterFatNormalised * static_cast<float> (AngryToddStages::V2B().RpotMax);
        v2bStage.setCathodePotResistance (potR);
    }
    else
    {
        // BOOST OFF: C12 shorted to ground, bypassing P2 = full bass boost
        v2bStage.setCathodePotResistance (0.0f);
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
