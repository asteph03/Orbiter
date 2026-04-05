#include "GrainProcessor.h"

void GrainProcessor::prepare(dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    numChannels = spec.numChannels;
    circularBuffer.prepare(spec);
    grainPool.reserve(10);
    outputGain.prepare(spec);
    baseGain.prepare(spec);
    baseGain.setGainDecibels(20.f);
    reverb.prepare(spec);
    hiPass[0].prepare(spec);
    hiPass[1].prepare(spec);
    loPass[0].prepare(spec);
    loPass[1].prepare(spec);
    
    const int numPoints = 1024;
    parabolicEnvelope.initialise ([numPoints](float index) {
            // Map the incoming index (0 to 1023) back to 0.0-1.0 for the cos math
            float x = index / (float)(numPoints - 1);
            return 1.f * (1.0f - std::cos(2.0f * pi * x));
        }, numPoints);
    
    trapezoidEnvelope.initialise ([numPoints](float index) {
        float x = index / (float)(numPoints - 1);
        if (x <= 0.25) // ATTACK
            return 8.4f * x;
        else if (x >= 0.75) // RELEASE
            return (-8.4f * x) + 8.4f;
        else // SUSTAIN
            return 2.1f;
        }, numPoints);
    
    bellEnvelope.initialise ([numPoints](float index) {
        float x = index / (float)(numPoints - 1);
        if (x <= 0.25 || x >= 0.75) // x <= 0.25 ATTACK, x >= 0.75 RELEASE
            return 1.f + std::cos(pi + (pi * (x / 0.25f)));
        else  // SUSTAIN
            return 2.f;
        }, numPoints);
    
    slowBellEnvelope.initialise ([numPoints](float index) {
        float x = index / (float)(numPoints - 1);
        if (x <= 0.8) // x <= 0.8 ATTACK, x > 0.8 RELEASE
            return 1.f + std::cos(pi + (pi * (x / 0.8f)));
        else
            return 1.f + std::cos(pi + (pi * ((x - 0.2f) / 0.2f)));
        }, numPoints);
}

void GrainProcessor::addParams(AudioProcessorParameterGroup& params)
{
    params.addChild(std::make_unique<AudioParameterFloat>(ParameterID(PARAMS::GrainMix, 1), "Mix", NormalisableRange<float>(0.f, 100.f, 1.f), 50.f));
    params.addChild(std::make_unique<AudioParameterFloat>(ParameterID(PARAMS::GrainSize, 1), "Size", NormalisableRange<float>(30.f, 120.f, 0.1f, 1.1f), 60.f));
    params.addChild(std::make_unique<AudioParameterFloat>(ParameterID(PARAMS::GrainDensity, 1), "Rate", NormalisableRange<float>(2.f, 16.f, 0.01f), 10.f));
    params.addChild(std::make_unique<AudioParameterFloat>(ParameterID(PARAMS::GrainOnset, 1), "Spray", NormalisableRange<float>(0.f, 100.f, 1.f), 0.f));
    
    params.addChild(std::make_unique<AudioParameterFloat>(ParameterID(PARAMS::OutputGain, 1), "Output Gain", NormalisableRange<float>(-24.f, 24.f, 0.1f), 0.f));
    params.addChild(std::make_unique<AudioParameterFloat>(ParameterID(PARAMS::GrainStereo, 1), "Stereo", NormalisableRange<float>(0.f, 100.f, 1.f), 0.f));
    params.addChild(std::make_unique<AudioParameterFloat>(ParameterID(PARAMS::GrainPitch, 1), "Pitch", NormalisableRange<float>(-12.f, 12.f, 1.f), 0.f));
    params.addChild(std::make_unique<AudioParameterFloat>(ParameterID(PARAMS::GrainReverb, 1), "Reverb", NormalisableRange<float>(0, 100.f, 1.f), 0.f));
    
    params.addChild(std::make_unique<AudioParameterBool>(ParameterID(PARAMS::GrainFreeze, 1), "Orbit Mode", false));
    params.addChild(std::make_unique<AudioParameterChoice>(ParameterID(PARAMS::GrainEnvelope, 1), "Envelope", envelopeTypes, Normal));
    params.addChild(std::make_unique<AudioParameterFloat>(ParameterID(PARAMS::GrainLP, 1), "Lo Pass", NormalisableRange<float>(20.f, 22000.f, 1.f, 0.5f), 22000.f));
    params.addChild(std::make_unique<AudioParameterFloat>(ParameterID(PARAMS::GrainHP, 1), "Hi Pass", NormalisableRange<float>(20.f, 22000.f, 1.f, 0.5f), 20.f));
    
    params.addChild(std::make_unique<AudioParameterBool>(ParameterID(PARAMS::GrainBypass, 1), "Bypass", false));
    params.addChild(std::make_unique<AudioParameterBool>(ParameterID(PARAMS::TempoSync, 1), "Tempo Sync", false));
    params.addChild(std::make_unique<AudioParameterChoice>(ParameterID(PARAMS::TempoTime, 1), "Tempo Time", beatSync, Eighth));
    params.addChild(std::make_unique<AudioParameterFloat>(ParameterID(PARAMS::TempoSwing, 1), "Tempo Swing", NormalisableRange<float>(0.f, 100.f, 1.f), 0.f));
}

bool GrainProcessor::update(AudioProcessorValueTreeState& params)
{
    bool bypass = (bool)params.getRawParameterValue(PARAMS::GrainBypass)->load();
    bool sync = (bool)params.getRawParameterValue(PARAMS::TempoSync)->load();
    float mix = params.getRawParameterValue(PARAMS::GrainMix)->load() * 0.01f;
    float size = params.getRawParameterValue(PARAMS::GrainSize)->load();
    float density = params.getRawParameterValue(PARAMS::GrainDensity)->load();
    float pitch = params.getRawParameterValue(PARAMS::GrainPitch)->load();
    float spray = params.getRawParameterValue(PARAMS::GrainOnset)->load();
    float rev = params.getRawParameterValue(PARAMS::GrainReverb)->load() * 0.01f;
    float gain = params.getRawParameterValue(PARAMS::OutputGain)->load();
    float hp = params.getRawParameterValue(PARAMS::GrainHP)->load();
    float lp = params.getRawParameterValue(PARAMS::GrainLP)->load();
    float swing = params.getRawParameterValue(PARAMS::TempoSwing)->load() * 0.01f;
    int time = (int)params.getRawParameterValue(PARAMS::TempoTime)->load();
    envelopeType = (int)params.getRawParameterValue(PARAMS::GrainEnvelope)->load();
    stereoRange = (int)params.getRawParameterValue(PARAMS::GrainStereo)->load();
    grainFreeze = (bool)params.getRawParameterValue(PARAMS::GrainFreeze)->load();
    
    for (auto& mixer : mixerBlend)
        mixer.setTargetValue(mix);
    
    for (auto& power : powerBlend)
        power.setTargetValue(bypass ? 0.f : 1.f);
    
    setScheduler(size, density, spray);
    setFilters(hp, lp);
    grainPitch = pitch;
    reverb.setMix(rev);
    outputGain.setGainDecibels(gain);
    
    tempoSync = sync;
    calcInterval(time, swing);
    
    return true;
}

void GrainProcessor::setScheduler(float size, int density, float spray)
{
    // size parameter is in miliseconds, convert from ms to samples
    paramGrainSize = (size * sampleRate) / 1000.f;
    grainDensity = density;
    samplesPerGrain = sampleRate / grainDensity;
    sprayFactor = spray;
}

void GrainProcessor::setFilters(float hpFreq, float lpFreq)
{
    hiPass[0].coefficients = dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, hpFreq);
    hiPass[1].coefficients = dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, hpFreq);
    
    loPass[0].coefficients = dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lpFreq);
    loPass[1].coefficients = dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lpFreq);
    
    //DBG("hpFreq: " << hpFreq << " . lpFreq: " << lpFreq);
}

void GrainProcessor::spawnGrain(int index)
{
    if (tempoSync)
    {
        // Only attempt sync when the DAW is actually playing
        if (!playheadInfo.getIsPlaying())
        {
            isSynced = false;
            return;
        }

        if (!isSynced)
        {
            auto ppqOpt = playheadInfo.getPpqPosition();
            if (!ppqOpt.hasValue()) return;

            double ppq = *ppqOpt;

            double intervalInBeats = (beatInterval / sampleRate) * (bpm / 60.0);
            double phaseInInterval = std::fmod(ppq, intervalInBeats * 2.0); // *2 to track pairs
            
            // Determine parity from the playhead so swing phase is always DAW-aligned
            isEvenBeat = phaseInInterval < intervalInBeats;
            
            double beatsUntilNext;
            if (isEvenBeat)
            {
                double beatsIntoInterval = std::fmod(ppq, intervalInBeats);
                double longIntervalBeats = (swingIntervalLong / sampleRate) * (bpm / 60.0);
                beatsUntilNext = longIntervalBeats - beatsIntoInterval;
            }
            else
            {
                double beatsIntoInterval = std::fmod(ppq, intervalInBeats);
                double shortIntervalBeats = (swingIntervalShort / sampleRate) * (bpm / 60.0);
                beatsUntilNext = shortIntervalBeats - beatsIntoInterval;
            }

            samplesUntilNextBeat = beatsUntilNext * (sampleRate * 60.0 / bpm);
            samplesSinceSpawn = 0;
            isSynced = true;
        }

        samplesSinceSpawn++;

        if (samplesSinceSpawn >= samplesUntilNextBeat && grainPool.size() <= 10)
        {
            // Spawn the grain
            Grain newGrain;
            newGrain.currentPos = (index - 4401.0);
            if (newGrain.currentPos < 0)
                newGrain.currentPos += circularBuffer.getSize();
            newGrain.grainSize = paramGrainSize;
            newGrain.envPos = 0;
            newGrain.playbackSpeed = std::pow(2.f, grainPitch / 12.f);

            int randomPos = randomSpawn.nextInt(juce::Range<int>(-1 * stereoRange, stereoRange + 1));
            float stereo = (float)randomPos / 100.f;
            if (stereo == 0.f)       { newGrain.spreadL = 1.f; newGrain.spreadR = 1.f; }
            else if (stereo > 0.f)   { newGrain.spreadL = 1.f - stereo; newGrain.spreadR = 1.f; }
            else                     { newGrain.spreadL = 1.f; newGrain.spreadR = 1.f + stereo; }

            grainPool.push_back(newGrain);

            samplesSinceSpawn = 0;
            isEvenBeat = !isEvenBeat; // flip parity
            samplesUntilNextBeat = isEvenBeat ? swingIntervalLong : swingIntervalShort;
        }
    }
    else
    {
        // Original free-running logic unchanged
        samplesSinceSpawn++;

        if (samplesSinceSpawn >= nextSpawn && grainPool.size() <= 10)
        {
            Grain newGrain;
            newGrain.currentPos = (index - 4401.0);
            if (newGrain.currentPos < 0)
                newGrain.currentPos += circularBuffer.getSize();
            newGrain.grainSize = paramGrainSize;
            newGrain.envPos = 0;
            newGrain.playbackSpeed = std::pow(2.f, grainPitch / 12.f);

            int randomPos = randomSpawn.nextInt(juce::Range<int>(-1 * stereoRange, stereoRange + 1));
            float stereo = (float)randomPos / 100.f;
            if (stereo == 0.f)       { newGrain.spreadL = 1.f; newGrain.spreadR = 1.f; }
            else if (stereo > 0.f)   { newGrain.spreadL = 1.f - stereo; newGrain.spreadR = 1.f; }
            else                     { newGrain.spreadL = 1.f; newGrain.spreadR = 1.f + stereo; }

            int randomSpray = randomSpawn.nextInt(juce::Range<int>(-1 * sprayFactor, sprayFactor + 1));
            float spray = randomSpray / 100.f;

            grainPool.push_back(newGrain);
            samplesSinceSpawn = 0;
            nextSpawn = samplesPerGrain + ((samplesPerGrain / 1.5f) * spray);
        }
    }
}

void GrainProcessor::cleanGrainPool()
{
    grainPool.erase(std::remove_if(grainPool.begin(), grainPool.end(),
            [](const Grain& g) { return g.isFinished; }),
            grainPool.end());
}

void GrainProcessor::reset()
{
    circularBuffer.clearBuffer();
    reverb.reset();
    isSynced = false;       // force re-sync on next play
    samplesSinceSpawn = 0;
}

float GrainProcessor::calculateEnvelope(int tableIndex)
{
    float envVal;
    
    switch(envelopeType)
    {
        case Normal:
            envVal = parabolicEnvelope[tableIndex];
            //DBG("Normal");
            break;
        case Smooth:
            envVal = bellEnvelope[tableIndex];
            //DBG("Smooth");
            break;
        case Harsh:
            envVal = trapezoidEnvelope[tableIndex];
            //DBG("Harsh");
            break;
        case Swell:
            envVal = slowBellEnvelope[tableIndex];
            //DBG("Swell");
            break;
    }
    
    return envVal;
}

void GrainProcessor::process(juce::AudioBuffer<float>& buffer)
{
    auto numSamples = buffer.getNumSamples();
    numChannels = buffer.getNumChannels();
    
    circularBuffer.fillBuffer(buffer);
    
    if (circularBuffer.firstWrap)
    {
        // Prevents any reading from circular buffer before first wrap is complete
        // Fixes bug where gain peaks at 10+ dB when first instantiated in DAW
        
        writePosition = circularBuffer.writePos;
        
        auto* channelDataL = buffer.getWritePointer(0);
        auto* channelDataR = buffer.getWritePointer(1);
        
        for (int i = 0; i < numSamples; i++)
        {
            auto inputL = channelDataL[i];
            auto inputR = channelDataR[i];
            
            spawnGrain((writePosition + i) % circularBuffer.getSize());
            
            float outputL = 0.f;
            float outputR = 0.f;
            for (auto& grain : grainPool) {
                if (grain.isFinished) continue;
                
                if (grain.envPos < 0) {
                    grain.envPos++;
                    continue;
                }

                // Calculate the floating-point read position
                double preciseIndex = grain.currentPos + (grain.envPos * grain.playbackSpeed);
                    
                // Wrap the index around the circular buffer
                double wrappedIndex = std::fmod(preciseIndex, (double)circularBuffer.getSize());

                // Get the two integer neighbors (i and i+1)
                int indexA = static_cast<int>(wrappedIndex);
                int indexB = (indexA + 1) % circularBuffer.getSize();
                    
                // Calculate the fraction (how far we are between A and B)
                float fraction = static_cast<float>(wrappedIndex - indexA);
                
                // Calculate the envelope value given the grain position
                float p = (float)grain.envPos / (float)grain.grainSize;
                int tableIndex = p * (parabolicEnvelope.getNumPoints() - 1);
                float envelope = 1.f * calculateEnvelope(tableIndex);

                // Fetch samples and interpolate: Output = A + (fraction * (B - A))
                float sampleLA = circularBuffer.read(0, indexA);
                float sampleLB = circularBuffer.read(0, indexB);
                float intrpL = sampleLA + fraction * (sampleLB - sampleLA);
                outputL += (intrpL * envelope * grain.spreadL);

                float sampleRA = circularBuffer.read(1, indexA);
                float sampleRB = circularBuffer.read(1, indexB);
                float intrpR = sampleRA + fraction * (sampleRB - sampleRA);
                outputR += (intrpR * envelope * grain.spreadR);
                
                // Age the grain
                grain.envPos++;
                
                if (grain.envPos >= grain.grainSize) {
                    if (!grainFreeze) {
                        grain.isFinished = true;
                    }
                    else {
                        grain.envPos = (-1.0 * samplesPerGrain) + grain.grainSize;
                        grain.replayCount++;
                        
                        if (grain.replayCount >= 20)
                            grain.isFinished = true;
                    }
                }
                        
            }
            
            
            {
                // SIGNAL CHAIN
                outputL = baseGain.processSample(outputL);
                outputR = baseGain.processSample(outputR);
                
                outputL = hiPass[0].processSample(outputL);
                outputL = loPass[0].processSample(outputL);
                
                outputR = hiPass[1].processSample(outputR);
                outputR = loPass[1].processSample(outputR);
                
                if (envelopeType == 2) { // SOFTCLIPPING ONLY WITH HARSH ENVELOPE
                    outputL = processSoftClipper(outputL);
                    outputR = processSoftClipper(outputR);
                }
                
                outputL = limit(outputL);
                outputR = limit(outputR);
                
                outputL = reverb.processSample(outputL, 0);
                outputR = reverb.processSample(outputR, 1);
            }
            
            float wet = mixerBlend[0].getTargetValue();
            float dry = 1.f - wet;
            
            float mixedL, mixedR;
            if (powerBlend[0].getCurrentValue() > 0.0001 || powerBlend[0].getTargetValue() > 0.5)
            {
                mixedL = (dry * inputL) + (wet * (outputL));
                mixedR = (dry * inputR) + (wet * (outputR));
                channelDataL[i] = outputGain.processSample(mixedL);
                channelDataR[i] = outputGain.processSample(mixedR);
            }
            else
            {
                channelDataL[i] = outputGain.processSample(inputL);
                channelDataR[i] = outputGain.processSample(inputR);
            }
        }
    }
    
    cleanGrainPool();
}
