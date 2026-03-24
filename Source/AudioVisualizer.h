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

class VerticalMeter : public juce::Component
{
public:
    
    VerticalMeter() {}
    
    void paint(Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto h = bounds.getHeight();
        g.setColour(Colour(0xff969696).withAlpha(0.3f));
        g.fillAll();
        
        g.setColour(levelColorBright);
        auto value = jmap(level, -60.f, 6.f, 0.f, h);
        auto lightBounds = jmin(value, h*0.5f);
        g.fillRect(bounds.removeFromBottom(lightBounds));
        
        if (value > h*0.5f) {
            g.setColour(levelColorMedium);
            
            auto mediumBounds = jmin(value, h*0.909f);
            g.fillRect(bounds.removeFromBottom(mediumBounds - h*0.5f));
            
            if (value > h*0.909f) {
                g.setColour(levelColorDark);
                
                auto darkBounds = jmin(value, h);
                g.fillRect(bounds.removeFromBottom(darkBounds - h*0.909f));
            }
        }
    }
    
    void setLevel(const float val)
    {
        level = val;
    }
    
    void setTheme(Colour themeColorBright, Colour themeColorMedium, Colour themeColorDark)
    {
        levelColorBright = themeColorBright;
        levelColorMedium = themeColorMedium;
        levelColorDark = themeColorDark;
    }
    
private:
    float level = -60.f;
    
    Colour levelColorBright, levelColorMedium, levelColorDark;
    
};
