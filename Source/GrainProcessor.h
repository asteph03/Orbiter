#pragma once

#include <vector>
#include <JuceHeader.h>
#include "CircularBuffer.h"
#include "ReverbProcessor.h"
using namespace juce;

namespace PARAMS
{
    #define PARAMETER_ID(str) constexpr const char* str { #str };

    PARAMETER_ID(GrainMix)
    PARAMETER_ID(GrainBypass)
    PARAMETER_ID(GrainSize)
    PARAMETER_ID(GrainDensity)
    PARAMETER_ID(GrainPitch)
    PARAMETER_ID(GrainEnvelope)
    PARAMETER_ID(GrainFreeze)
    PARAMETER_ID(GrainStereo)
    PARAMETER_ID(GrainOnset)
    PARAMETER_ID(GrainReverb)
    PARAMETER_ID(OutputGain)
}

struct Grain
{
    double currentPos;
    int grainSize, envPos;
    bool isFinished = false;
    float playbackSpeed = 1.f;
    float spreadL, spreadR;
    int replayCount = 0;
};

enum envelopeIndex
{
    Parabolic = 0,
    CosineBell = 1,
    Trapezoidal = 2,
    SlowAttackBell = 3,
};

class GrainProcessor
{
public:
    GrainProcessor() {}
    
    void prepare(dsp::ProcessSpec& spec);
    void addParams(AudioProcessorParameterGroup& params);
    bool update(AudioProcessorValueTreeState& params);
    void setScheduler(float size, int density, float spray);
    void spawnGrain(int index);
    void cleanGrainPool();
    void reset();
    float calculateEnvelope(int tableIndex);
    void process(juce::AudioBuffer<float>& buffer);
    
    StringArray envelopeTypes =
    {
        "Parabolic",
        "Cosine Bell",
        "Trapezoidal",
        "Slow Attack Bell",
    };
    
    static float limit (float x) noexcept
    {
        auto out = 3.5f * juce::dsp::FastMathApproximations::tanh(0.3f * x);
        return jlimit(-3.5f, 3.5f, out);
    }
    
private:
    static constexpr auto bufferMaxSamples = 480000; // 5s @ 96k sample rate
    static constexpr float pi = juce::MathConstants<float>::pi;
    
    CircularBuffer circularBuffer;
    ReverbProcessor reverb;
    std::array<juce::LinearSmoothedValue<float>, 2> powerBlend, mixerBlend;
    
    std::vector<Grain> grainPool;
    juce::dsp::LookupTable<float> parabolicEnvelope, trapezoidEnvelope, bellEnvelope, slowBellEnvelope;
    dsp::Gain<float> outputGain, baseGain;
    juce::Random randomSpawn;
    
    float sampleRate, paramGrainSize, grainDensity, samplesPerGrain, grainPitch, sprayFactor, nextSpawn;
    int numChannels, envelopeType, stereoRange, activeGrainCount;
    int counter = 0;
    int samplesSinceSpawn = 0;
    int writePosition = { 0 };
    bool grainFreeze = false;
};
