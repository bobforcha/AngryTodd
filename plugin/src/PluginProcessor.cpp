#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace ParamID
{
    static const juce::String inputGain   = "inputGain";
    static const juce::String lowContour  = "lowContour";
    static const juce::String boosterFat  = "boosterFat";
    static const juce::String boost       = "boost";
    static const juce::String level       = "level";
    static const juce::String highContour = "highContour";
    static const juce::String treble      = "treble";
    static const juce::String bass        = "bass";
    static const juce::String mid         = "mid";
    static const juce::String limit       = "limit";
    static const juce::String highCut     = "highCut";
    static const juce::String master      = "master";
    static const juce::String boostMaster = "boostMaster";
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto addFloat = [&](const juce::String& id, const juce::String& name,
                        float min, float max, float defaultVal)
    {
        params.push_back (std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { id, 1 }, name,
            juce::NormalisableRange<float> (min, max, 0.01f), defaultVal));
    };

    addFloat (ParamID::inputGain,   "Input Gain",    0.0f, 10.0f, 1.0f);
    addFloat (ParamID::lowContour,  "Low Contour",   0.0f, 1.0f,  0.5f);
    addFloat (ParamID::boosterFat,  "Booster Fat",   0.0f, 1.0f,  0.5f);

    params.push_back (std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { ParamID::boost, 1 }, "Boost", false));

    addFloat (ParamID::level,       "Level",         0.0f, 1.0f,  0.3f);
    addFloat (ParamID::highContour, "High Contour",  0.0f, 1.0f,  0.5f);
    addFloat (ParamID::treble,      "Treble",        0.0f, 1.0f,  0.5f);
    addFloat (ParamID::bass,        "Bass",          0.0f, 1.0f,  0.5f);
    addFloat (ParamID::mid,         "Mid",           0.0f, 1.0f,  0.5f);
    addFloat (ParamID::limit,       "Limit",         0.0f, 1.0f,  1.0f);
    addFloat (ParamID::highCut,     "High Cut",      0.0f, 1.0f,  0.5f);
    addFloat (ParamID::master,      "Master",        0.0f, 1.0f,  0.5f);
    addFloat (ParamID::boostMaster, "Boost Master",  0.0f, 1.0f,  0.5f);

    return { params.begin(), params.end() };
}

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    for (const auto& id : { ParamID::inputGain, ParamID::lowContour, ParamID::boosterFat,
                            ParamID::boost, ParamID::level, ParamID::highContour,
                            ParamID::treble, ParamID::bass, ParamID::mid, ParamID::limit,
                            ParamID::highCut, ParamID::master, ParamID::boostMaster })
    {
        apvts.addParameterListener (id, this);
    }
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
    v2aInterstage.prepare (spec);
    v3aStage.prepare (spec);
    toneStack.prepare (spec);
    masterSection.prepare (spec);

    syncFromParameters();
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

    auto block = juce::dsp::AudioBlock<float> (buffer);
    inputCoupling.process (block);
    v1bStage.process (block);
    v1bInterstage.process (block);
    v1aStage.process (block);
    v1aInterstage.process (block);
    v2bStage.process (block);
    levelContour.process (block);
    v2aStage.process (block);
    v2aInterstage.process (block);
    v3aStage.process (block);
    // V3B cathode follower: unity gain buffer, no processing
    toneStack.process (block);
    masterSection.process (block);
    // V4A + V4B cathode followers: unity gain buffers, no processing
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
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
void AudioPluginAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID == ParamID::inputGain)
    {
        inputGain = newValue;
    }
    else if (parameterID == ParamID::lowContour)
    {
        float potR = newValue * static_cast<float> (AngryToddStages::V1B().RpotMax);
        v1bStage.setCathodePotResistance (potR);
    }
    else if (parameterID == ParamID::boosterFat)
    {
        boosterFatNormalised = newValue;
        updateV2bCathodePot();
    }
    else if (parameterID == ParamID::boost)
    {
        boostEngaged = newValue > 0.5f;
        updateV2bCathodePot();
        toneStack.setBoostHighCutEnabled (boostEngaged);
        masterSection.setBoostEngaged (boostEngaged);
    }
    else if (parameterID == ParamID::level)        levelContour.setLevel (newValue);
    else if (parameterID == ParamID::highContour)  levelContour.setHighContour (newValue);
    else if (parameterID == ParamID::treble)       toneStack.setTreble (newValue);
    else if (parameterID == ParamID::bass)         toneStack.setBass (newValue);
    else if (parameterID == ParamID::mid)          toneStack.setMid (newValue);
    else if (parameterID == ParamID::limit)        toneStack.setLimit (newValue);
    else if (parameterID == ParamID::highCut)      toneStack.setBoosterHighCut (newValue);
    else if (parameterID == ParamID::master)       masterSection.setMaster (newValue);
    else if (parameterID == ParamID::boostMaster)  masterSection.setBoostMaster (newValue);
}

void AudioPluginAudioProcessor::syncFromParameters()
{
    auto get = [this](const juce::String& id) { return apvts.getRawParameterValue (id)->load(); };

    inputGain = get (ParamID::inputGain);
    boostEngaged = get (ParamID::boost) > 0.5f;
    boosterFatNormalised = get (ParamID::boosterFat);

    v1bStage.setCathodePotResistance (get (ParamID::lowContour)
        * static_cast<float> (AngryToddStages::V1B().RpotMax));
    updateV2bCathodePot();
    levelContour.setLevel (get (ParamID::level));
    levelContour.setHighContour (get (ParamID::highContour));
    toneStack.setTreble (get (ParamID::treble));
    toneStack.setBass (get (ParamID::bass));
    toneStack.setMid (get (ParamID::mid));
    toneStack.setLimit (get (ParamID::limit));
    toneStack.setBoosterHighCut (get (ParamID::highCut));
    toneStack.setBoostHighCutEnabled (boostEngaged);
    masterSection.setMaster (get (ParamID::master));
    masterSection.setBoostMaster (get (ParamID::boostMaster));
    masterSection.setBoostEngaged (boostEngaged);
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
