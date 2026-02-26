#pragma once
#include <JuceHeader.h>

using namespace juce;

class OrbitButton : public juce::Button
{
public:
    OrbitButton(const String& componentID) : juce::Button("OrbitButton")
    {
        setComponentID(componentID);
        setClickingTogglesState(true);
        setWantsKeyboardFocus(false);
    }
    
    void paintButton(Graphics& g, bool, bool) override
    {
        auto bounds = getLocalBounds();
        auto w = bounds.getWidth();
        auto h = bounds.getHeight();
        
        if (getToggleState()) {
            ColourGradient gradient(buttonColorDark, w/2, h, buttonColorBright, w/2, 0, false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(2, 2, w-4, h-4, 5.f);
        }
        
        g.setColour(Colour(0xffe3e3e3));
        g.drawRoundedRectangle(2, 2, w-4, h-4, 5.f, 1.5f);
        
        Font f(FontOptions("Futura Medium", 24.f, Font::plain));
        f.setHorizontalScale(1.1f);
        g.setFont(f);
        g.drawText("ORBIT", 2, 4, w-4, h-4, Justification::centred);
    }
    
    void setTheme(Colour bright, Colour dark)
    {
        buttonColorBright = bright;
        buttonColorDark = dark;
    }
    
    
private:
    juce::Colour buttonColorDark;
    juce::Colour buttonColorBright;
    
};

class BypassButton : public juce::Button
{
public:
    BypassButton(const String& componentID) : juce::Button("BypassButton")
    {
        setComponentID(componentID);
        setClickingTogglesState(true);
        setWantsKeyboardFocus(false);
    }
    
    void paintButton(Graphics& g, bool, bool) override
    {
        auto bounds = getLocalBounds();
        auto w = bounds.getWidth();
        auto h = bounds.getHeight();
        
        if (getToggleState()) {
            ColourGradient gradient(Colour(0xff808080), w/2, h, Colour(0xff969696), w/2, 0, false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(2, 2, w-4, h-4, 5.f);
        }
        
        g.setColour(Colour(0xffe3e3e3));
        g.drawRoundedRectangle(2, 2, w-4, h-4, 5.f, 1.5f);
        
        g.drawText("Bypass", 2, 2, w-4, h-4, Justification::centred);
    }
    
    private:
};

class EnvelopeButton : public juce::Button
{
public:
    EnvelopeButton(const String& componentID, AudioProcessorValueTreeState& vts) : juce::Button("EnvelopeButton"), apvts(vts)
    {
        setComponentID(componentID);
        setClickingTogglesState(true);
        setWantsKeyboardFocus(false);

        addMouseListener(this, true);
        
        instantiatePaths();
    }
    
    void instantiatePaths()
    {
        normal.addRectangle(2.f, 2.f, 81.f, 42.f);
        normal.closeSubPath();
        
        smooth.addRectangle(83.f, 2.f, 81.f, 42.f);
        smooth.closeSubPath();
        
        harsh.addRectangle(2.f, 44.f, 81.f, 42.f);
        harsh.closeSubPath();
        
        swell.addRectangle(83.f, 44.f, 81.f, 42.f);
        swell.closeSubPath();
    }
    
    void paintButton(Graphics& g, bool, bool) override
    {
        auto bounds = getLocalBounds();
        auto w = bounds.getWidth();
        auto h = bounds.getHeight();
        
        int envelopeSelected = apvts.getRawParameterValue(PARAMS::GrainEnvelope)->load();
        
        if (envelopeSelected == 0) { // NORMAL
            ColourGradient gradient(buttonColorDark, w/4.f, h/2.f, buttonColorBright, w/4.f, 0, false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(2.f, 2.f, (w/2.f)-2.f, (h/2.f)-2.f, 5.f);
            g.fillRect((w/2.f)-8.f, 2.f, 8.f, 8.f);
            g.fillRect((w/2.f)-8.f, (h/2.f)-8.f, 8.f, 8.f);
            g.fillRect(2.f, (h/2.f)-8.f, 8.f, 8.f);
        }
        else if (envelopeSelected == 1) { // SMOOTH
            ColourGradient gradient(buttonColorDark, (w*3.f)/4.f, h/2.f, buttonColorBright, (w*3.f)/4.f, 0, false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(w/2.f, 2.f, (w/2.f)-2.f, (h/2.f)-2.f, 5.f);
            g.fillRect((w/2.f), 2.f, 8.f, 8.f);
            g.fillRect((w/2.f), (h/2.f)-8.f, 8.f, 8.f);
            g.fillRect(w-10.f, (h/2.f)-8.f, 8.f, 8.f);
        }
        else if (envelopeSelected == 2) { // HARSH
            ColourGradient gradient(buttonColorDark, w/4.f, h, buttonColorBright, w/4.f, h/2.f, false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(2.f, h/2.f, (w/2.f)-2.f, (h/2.f)-2.f, 5.f);
            g.fillRect((w/2.f)-8.f, h/2.f, 8.f, 8.f);
            g.fillRect((w/2.f)-8.f, h-10.f, 8.f, 8.f);
            g.fillRect(2.f, h/2.f, 8.f, 8.f);
        }
        else { // SWELL
            ColourGradient gradient(buttonColorDark, (w*3.f)/4.f, h, buttonColorBright, (w*3.f)/4.f, h/2.f, false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(w/2.f, h/2.f, (w/2.f)-2.f, (h/2.f)-2.f, 5.f);
            g.fillRect((w/2.f), h/2.f, 8.f, 8.f);
            g.fillRect((w/2.f), h-10.f, 8.f, 8.f);
            g.fillRect(w-10.f, h/2.f, 8.f, 8.f);
        }
        
        g.setColour(Colour(0xffe3e3e3));
        g.drawRoundedRectangle(2, 2, w-4, h-4, 5.f, 1.5f);
        g.drawLine(w/2, 2, w/2, h-2, 1.5f);
        g.drawLine(2, h/2, w-2, h/2, 1.5f);
        
        g.drawText("Normal", 2, 2, (w/2)-2, (h/2)-2, Justification::centred);
        g.drawText("Smooth", w/2, 2, (w/2)-2, (h/2)-2, Justification::centred);
        g.drawText("Harsh", 2, h/2, (w/2)-2, (h/2)-2, Justification::centred);
        g.drawText("Swell", w/2, h/2, (w/2)-2, (h/2)-2, Justification::centred);
        
        g.setColour(Colours::white); 
        
    }
    
    void setValue(int val)
    {
        apvts.getParameterAsValue(PARAMS::GrainEnvelope).setValue(val);
        //DBG(String(val));
    }
    
    void mouseDown(const MouseEvent& event) override
    {
        int x = event.getMouseDownX();
        int y = event.getMouseDownY();
        
        int value;
        
        if (normal.contains(x, y))
        {
            value = 0;
        }
        else if (smooth.contains(x, y))
        {
            value = 1;
        }
        else if (harsh.contains(x, y))
        {
            value = 2;
        }
        else if (swell.contains(x, y))
        {
            value = 3;
        }
        
        setValue(value);
        repaint();
    }
    
    void setTheme(Colour bright, Colour dark)
    {
        buttonColorBright = bright;
        buttonColorDark = dark;
    }
    
private:
    AudioProcessorValueTreeState& apvts;
    Path normal, smooth, harsh, swell;
    
    juce::Colour buttonColorDark;
    juce::Colour buttonColorBright;
};
