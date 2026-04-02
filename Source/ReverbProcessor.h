#pragma once

using namespace juce;

class ReverbProcessor
{
public:

    ReverbProcessor() {}
    
    void prepare(dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        
        ap1.prepare(spec);
        ap1.setMaximumDelayInSamples((int)2 * sampleRate);
        ap2.prepare(spec);
        ap2.setMaximumDelayInSamples((int)2 * sampleRate);
        ap3.prepare(spec);
        ap3.setMaximumDelayInSamples((int)2 * sampleRate);
        ap4.prepare(spec);
        ap4.setMaximumDelayInSamples((int)2 * sampleRate);
        tankL.prepare(spec);
        tankL.setMaximumDelayInSamples((int)2 * sampleRate);
        tankR.prepare(spec);
        tankR.setMaximumDelayInSamples((int)2 * sampleRate);
        
        float srScale = sampleRate / 44100.f;
        ap1.setDelay(142.f * srScale);
        ap2.setDelay(107.f * srScale);
        ap3.setDelay(379.f * srScale);
        ap4.setDelay(277.f * srScale);
        tankL.setDelay(4453.f * srScale);
        tankR.setDelay(3720.f * srScale);
        
        for (auto& lp : lowPass)
        {
            lp.prepare(spec);
            lp.coefficients = dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(sampleRate, 4000.f);
        }

        lastDelayEffectOutput[0] = 0.f;
        lastDelayEffectOutput[1] = 0.f;
    }
    
    void setMix(float mixVal)
    {
        for (auto& mix : mixerBlend)
            mix.setTargetValue(0.75f * mixVal);
    }
    
    void oscillateTankDelays()
    {
        
    }
    
    void reset()
    {
        ap1.reset();
        ap2.reset();
        ap3.reset();
        ap4.reset();
        tankL.reset();
        tankR.reset();
    }
    
    float processSample(float inputSample, int channel)
    {
        float sample = inputSample;
        
        float wet = mixerBlend[0].getTargetValue();
        float dry = 1.f - wet;
        
        ap1.pushSample(channel, sample);
        float diffused = ap1.popSample(channel);
        ap2.pushSample(channel, diffused);
        diffused = ap2.popSample(channel);
        ap3.pushSample(channel, diffused);
        diffused = ap3.popSample(channel);
        ap4.pushSample(channel, diffused);
        diffused = ap4.popSample(channel);

        float outputSample;
        if (channel == 0) {
            float leftNode = diffused + (0.5f * lastDelayEffectOutput[1]);
            float leftOut = tankL.popSample(0);
            tankL.pushSample(0, leftNode);
            
            leftOut = lowPass[0].processSample(leftOut);
            leftOut = limit(leftOut);
            lastDelayEffectOutput[0] = leftOut;
            outputSample = (dry * inputSample) + (wet * leftOut);
        }
        else { // channel == 1
            float rightNode = diffused + (0.5f * lastDelayEffectOutput[0]);
            float rightOut = tankR.popSample(0);
            tankR.pushSample(0, rightNode);
            
            rightOut = lowPass[1].processSample(rightOut);
            rightOut = limit(rightOut);
            lastDelayEffectOutput[1] = rightOut;
            outputSample = (dry * inputSample) + (wet * rightOut);
        }

        //DBG("channel: " << channel << " sample: " << outputSample);
        return outputSample;
    }
    
    static float limit (float x) noexcept
    {
        if (std::isnan(x) || std::isinf(x)) return 0.f;
        
        auto out = 3.5f * juce::dsp::FastMathApproximations::tanh(0.3f * x);
        return jlimit(-1.f, 1.f, out);
    }
    
private:
    float sampleRate;
    float feedbackL, feedbackR;
    
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> ap1, ap2, ap3, ap4, tankL, tankR;
    std::array<float, 2> lastDelayEffectOutput;
    std::array<juce::LinearSmoothedValue<float>, 2> mixerBlend;
    dsp::IIR::Filter<float> lowPass[2];
    
};
