#pragma once

using namespace juce;

class CircularBuffer
{
public:
    
    void prepare(dsp::ProcessSpec& spec)
    {
        double bufferSize = spec.sampleRate * 2.0;
        int numChannels = spec.numChannels;
        
        circularBuffer.setSize(numChannels, (int)bufferSize);
    }
    
    int getSize()
    {
        int size = circularBuffer.getNumSamples();
        return size;
    }
    
    void clearBuffer()
    {
        circularBuffer.clear();
    }
    
    float read(int channel, int index)
    {
        return circularBuffer.getSample(channel, index);
    }
    
    void fillBuffer(AudioBuffer<float>& buffer)
    {
        int bufferSize = buffer.getNumSamples();
        int circularBufferSize = circularBuffer.getNumSamples();
        
        int numChannels = circularBuffer.getNumChannels();
        
        for (int channel = 0; channel < numChannels; channel++)
        {
            auto* input = buffer.getReadPointer(channel);
            
            if (circularBufferSize > bufferSize + writePos)
            {
                // Enough space in circular buffer for incoming buffer, copy and proceed
                circularBuffer.copyFromWithRamp(channel, writePos, input, bufferSize, 0.1f, 0.1f);
            }
            else
            {
                // Not enough space in circular buffer for incoming buffer
                // Copy # of samples until circular buffer is full, wrap around to index 0, and copy rest
                int preWrapSamples = circularBufferSize - writePos;
                int postWrapSamples = bufferSize - preWrapSamples;
                
                circularBuffer.copyFromWithRamp(channel, writePos, input, preWrapSamples, 0.1f, 0.1f);
                circularBuffer.copyFromWithRamp(channel, 0, input, postWrapSamples, 0.1f, 0.1f);
                
                if (!firstWrap)
                    firstWrap = true;
            }
        }
        
        writePos += bufferSize;
        writePos = writePos % circularBufferSize;
    }
    
    int writePos = { 0 };
    
    bool firstWrap = false;
    
private:
    AudioBuffer<float> circularBuffer;
};
