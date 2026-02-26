#include "AudioVisualizer.h"

AudioVisualizer::AudioVisualizer()
{
    displayBuffer.clear();
    startTimerHz(60);
}

AudioVisualizer::~AudioVisualizer()
{
    stopTimer();
}

void AudioVisualizer::pushBuffer(const float* data, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        // Track the peak over this block of samples
        currentPeakMax = juce::jmax(currentPeakMax, data[i]);
        currentPeakMin = juce::jmin(currentPeakMin, data[i]);
        ++sampleAccumulator;

        if (sampleAccumulator >= samplesPerBlock)
        {
            // Write one peak value into the fifo
            auto writer = abstractFifo.write(1);
            if (writer.blockSize1 > 0)
            {
                fifoBufferMax[writer.startIndex1] = currentPeakMax;
                fifoBufferMin[writer.startIndex1] = currentPeakMin;
            }
                

            currentPeakMax = 0.f;
            currentPeakMin = 0.f;
            sampleAccumulator = 0;
        }
    }
}

void AudioVisualizer::timerCallback()
{
    auto reader = abstractFifo.read(abstractFifo.getNumReady());

    auto copyToDisplay = [&](int srcStart, int numSamples)
    {
        auto displaySize = displayBuffer.getNumSamples();

        int space = displaySize - displayBufferWritePos;

        if (numSamples <= space)
        {
            displayBuffer.copyFrom(0, displayBufferWritePos, fifoBufferMax + srcStart, numSamples);
            displayBuffer.copyFrom(1, displayBufferWritePos, fifoBufferMin + srcStart, numSamples);
            displayBufferWritePos += numSamples;
        }
        else
        {
            displayBuffer.copyFrom(0, displayBufferWritePos, fifoBufferMax + srcStart, space);
            displayBuffer.copyFrom(1, displayBufferWritePos, fifoBufferMin + srcStart, space);
            
            displayBuffer.copyFrom(0, 0, fifoBufferMax + srcStart + space, numSamples - space);
            displayBuffer.copyFrom(1, 0, fifoBufferMin + srcStart + space, numSamples - space);
            displayBufferWritePos = numSamples - space;
        }

        displayBufferWritePos = displayBufferWritePos % displaySize;
    };

    if (reader.blockSize1 > 0)
        copyToDisplay(reader.startIndex1, reader.blockSize1);

    if (reader.blockSize2 > 0)
        copyToDisplay(reader.startIndex2, reader.blockSize2);

    repaint();
}

void AudioVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();
    
    // Background
    ColourGradient gradient1(Colour(0xff475563), w*0.5f, 0, Colour(0xff3e4a57), w*0.5f, h, false);
    g.setGradientFill(gradient1);
    g.fillRoundedRectangle(0, 0, w, h, 8.f);

    drawWaveform(g);
    
    ColourGradient gradient2(Colour(0xff8f8f8f), w*0.25f, 0, Colour(0xff999999), w*0.66f, h, false);
    g.setGradientFill(gradient2);
    g.drawRoundedRectangle(2, 2, w-4, h-4, 8.f, 4.f);
}

void AudioVisualizer::drawWaveform(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto width  = bounds.getWidth();
    auto height = bounds.getHeight();
    auto numSamples = displayBuffer.getNumSamples();

    juce::Path waveformPath;

    // Forward pass — top of waveform (positive peaks)
    for (int x = 0; x < (int)width; ++x)
    {
        int sampleIndex = (displayBufferWritePos + (int)juce::jmap((float)x,
                              0.0f, width, 0.0f, (float)numSamples)) % numSamples;

        float sample = displayBuffer.getSample(0, sampleIndex); // max channel
        float y = juce::jmap(juce::jlimit(-1.0f, 1.0f, sample),
                                 -1.0f, 1.0f, height - 2.0f, 2.0f);

        if (x == 0) waveformPath.startNewSubPath(0.0f, y);
        else        waveformPath.lineTo((float)x, y);
    }

    // Reverse pass — bottom of waveform (negative peaks)
    for (int x = (int)width - 1; x >= 0; --x)
    {
        int sampleIndex = (displayBufferWritePos + (int)juce::jmap((float)x,
                              0.0f, width, 0.0f, (float)numSamples)) % numSamples;

        float sample = displayBuffer.getSample(1, sampleIndex); // min channel
        float y = juce::jmap(juce::jlimit(-1.0f, 1.0f, sample),
                                 -1.0f, 1.0f, height - 2.0f, 2.0f);

        waveformPath.lineTo((float)x, y);
    }

    waveformPath.closeSubPath();

    // Fill
    g.setColour(visualizerColor);
    g.fillPath(waveformPath);
}

void AudioVisualizer::setTheme(Colour themeColor)
{
    visualizerColor = themeColor;
}
