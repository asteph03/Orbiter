#pragma once
#include <JuceHeader.h>

using namespace juce;

class AudioVisualizer : public juce::Component, private juce::Timer
{
public:
    
    AudioVisualizer();
    ~AudioVisualizer();
    
    void pushBuffer(const float* data, int numSamples);
    void paint(Graphics& g) override;
    void timerCallback() override;
    void drawWaveform(Graphics& g);
    void setTheme(Colour themeColor);
    
private:
    AbstractFifo abstractFifo { fifoSize };
    static constexpr int fifoSize = 320;
    float fifoBufferMax[fifoSize] = {};
    float fifoBufferMin[fifoSize] = {};
    AudioBuffer<float> displayBuffer { 2, fifoSize };
    int displayBufferWritePos = 0;
    
    int samplesPerBlock = 320;  // higher = slower scroll, lower = faster
    int sampleAccumulator = 0;
    float currentPeakMax = 0.f;
    float currentPeakMin = 0.f;
    
    Colour visualizerColor;
};
