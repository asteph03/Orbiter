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
    PARAMETER_ID(GrainHP)
    PARAMETER_ID(GrainLP)
    PARAMETER_ID(OutputGain)
    PARAMETER_ID(TempoSync)
    PARAMETER_ID(TempoTime)
    PARAMETER_ID(TempoSwing)
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
    Normal = 0,
    Smooth = 1,
    Harsh = 2,
    Swell = 3,
};

enum beatIndex
{
    Whole = 0, HalfDot = 1, Half = 2, HalfTrip = 3, QuartDot = 4, Quarter = 5, QuartTrip = 6, EightDot = 7,
    Eighth = 8, EightTrip = 9, SixtDot = 10, Sixteenth = 11, SixtTrip = 12, ThirDot = 13, ThirtySec = 14,
};

class GrainProcessor
{
public:
    GrainProcessor() {}
    
    void prepare(dsp::ProcessSpec& spec);
    void addParams(AudioProcessorParameterGroup& params);
    bool update(AudioProcessorValueTreeState& params);
    void setScheduler(float size, int density, float spray);
    void setFilters(float hpFreq, float lpFreq);
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

    StringArray beatSync =
    {
        "1", "1/2.d", "1/2", "1/2.t", "1/4.d", "1/4", "1/4.t", "1/8.d",
        "1/8", "1/8.t", "1/16.d", "1/16", "1/16.t", "1/32.d", "1/32",
    };
    
    static float limit (float x) noexcept
    {
        auto out = 3.5f * juce::dsp::FastMathApproximations::tanh(0.3f * x);
        return jlimit(-3.5f, 3.5f, out);
    }
    
    void setBpm(float newBpm)
    {
        bpm = newBpm;
    }
    
    float processHardClipper(float inputSample)
    {
        auto wetSignal = inputSample * juce::Decibels::decibelsToGain(16.f);
        wetSignal = (2.0 / juce::MathConstants<float>::pi) * std::atan(wetSignal);
        wetSignal *= 2.0;
                
        if (std::abs(wetSignal) > 1.0)
        {
            wetSignal *= 1.0 / std::abs(wetSignal);
        }
            
        wetSignal = wetSignal * juce::Decibels::decibelsToGain(-16.f);
        return wetSignal;
    }
    
    void calcInterval(int time, float swing)
    {
        float mpi; // miliseconds per interval
        float mpb = 60000.f / bpm; // miliseconds per beat
        
        switch(time) {
            case Whole: mpi = mpb * 4.f; break;
            case HalfDot: mpi = mpb * 2.f * 1.5f; break;
            case Half: mpi = mpb * 2.f; break;
            case HalfTrip: mpi = mpb * 2.f * (2.f / 3.f); break;
            case QuartDot: mpi = mpb * 1.5f; break;
            case Quarter: mpi = mpb; break;
            case QuartTrip: mpi = mpb * (2.f / 3.f); break;
            case EightDot: mpi = mpb * 0.5f * 1.5f; break;
            case Eighth: mpi = mpb * 0.5f; break;
            case EightTrip: mpi = mpb * 0.5f * (2.f / 3.f); break;
            case SixtDot: mpi = mpb * 0.25f * 1.5f; break;
            case Sixteenth: mpi = mpb * 0.25f; break;
            case SixtTrip: mpi = mpb * 0.25f * (2.f / 3.f); break;
            case ThirDot: mpi = mpb * 0.125f * 1.5f; break;
            case ThirtySec: mpi = mpb * 0.125f; break;
        }
        
        beatInterval = (mpi * sampleRate * 0.001f);
        
        float swingRatio = 0.5f + (swing * 0.3f); // maps 0..1 → 0.5..1.0
        swingIntervalLong  = beatInterval * 2.f * swingRatio;
        swingIntervalShort = beatInterval * 2.f * (1.f - swingRatio);
    }
    
    void setPlayheadInfo(const juce::AudioPlayHead::PositionInfo& info)
    {
        playheadInfo = info;
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
    dsp::IIR::Filter<float> hiPass[2], loPass[2];
    
    juce::AudioPlayHead::PositionInfo playheadInfo;
    double samplesUntilNextBeat = 0.0;
    bool isSynced = false;
    
    bool tempoSync = false;
    float sampleRate, paramGrainSize, grainDensity, samplesPerGrain, grainPitch, sprayFactor, nextSpawn, bpm, beatInterval;
    int numChannels, envelopeType, stereoRange, activeGrainCount;
    int counter = 0;
    int samplesSinceSpawn = 0;
    int writePosition = { 0 };
    bool grainFreeze = false;
    
    bool isEvenBeat = false;
    float swingIntervalLong = 0.f;
    float swingIntervalShort = 0.f;
};
