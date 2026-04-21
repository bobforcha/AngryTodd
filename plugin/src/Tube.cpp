// Implementation of a Tube class in C++

#include "Tube.h"

Tube::Tube()
{}

Tube::~Tube()
{}

// =================================================
void Tube::prepare(const juce::dsp::ProcessSpec& spec)
{
    processSpec = spec;
    tempBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
}

void Tube::process(juce::dsp::AudioBlock<float>& block, const TubeParameters& params)
{
    const size_t numChannels = block.getNumChannels();
    const size_t numSamples = block.getNumSamples();

    // Ensure tempBuffer is the right size
    tempBuffer.setSize((int)numChannels, (int)numSamples);
    
    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* input = block.getChannelPointer(ch);
        auto* output = tempBuffer.getWritePointer((int)ch);

        for (size_t n = 0; n < numSamples; ++n)
        {
            // Apply tube distortion formula
            float drivenSample = input[n] * params.drive;
            float distortedSample = (drivenSample >= 0.0f) ? 
                (1.0f - std::exp(-drivenSample)) : 
                (-1.0f + std::exp(drivenSample));

            // Mix dry and wet signals
            output[n] = juce::jlimit(-1.0f, 1.0f, 
                (1.0f - params.mix) * input[n] + params.mix * distortedSample);
        }
    }

    // Copy processed data back to the original block
    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* output = block.getChannelPointer(ch);
        auto* tempOutput = tempBuffer.getReadPointer((int)ch);
        std::memcpy(output, tempOutput, numSamples * sizeof(float));
    }
}
