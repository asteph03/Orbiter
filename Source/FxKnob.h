#pragma once
#include <JuceHeader.h>
#include "GrainProcessor.h"

using namespace juce;

class FxKnob : public juce::Slider, public juce::Slider::Listener
{
public:
    FxKnob()
    {
        addListener(this);
        setSliderStyle(juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle(juce::Slider::NoTextBox, true, 80, 12);
    }
    
    FxKnob(const String& componentID)
    {
        addListener(this);
        setSliderStyle(juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle(juce::Slider::NoTextBox, true, 80, 12);
        setComponentID(componentID);
        
        if (componentID == PARAMS::GrainPitch)
            isCentered = true;
        
        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.setInterceptsMouseClicks(false, false); // don't block mouse events
        addAndMakeVisible(valueLabel);
        valueLabel.setVisible(false);
    }
    
    void resized() override
    {
        valueLabel.setBounds(getLocalBounds());
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto center = bounds.getCentre();
        auto radius = bounds.getWidth() * 0.5f;
        
        g.setColour(Colours::black.withAlpha(0.5f));
        g.fillEllipse(bounds);
        
        auto middleBounds = bounds.reduced(bounds.getWidth() * 0.08f);
        ColourGradient background(Colour(0xff535661), Point<float>(bounds.getCentreX(), bounds.getBottom()), Colour(0xffa9acba), Point<float>(bounds.getCentreX(), bounds.getY()), false);
        g.setGradientFill(background);
        g.fillEllipse(middleBounds);
        
        const auto strokeType = juce::PathStrokeType(2.f,
                juce::PathStrokeType::JointStyle::curved,
                juce::PathStrokeType::EndCapStyle::rounded);
            
        // upper rand
        auto midRadius = middleBounds.getWidth() * 0.5f;
        ColourGradient grad = juce::ColourGradient(Colour(0x3affffff), middleBounds.getTopLeft().translated(midRadius, 0), Colour(0xffffffff).withAlpha(0.f), center, true);
        g.setGradientFill(grad);
        Path p;
        p.addCentredArc (center.getX(), center.getY(), midRadius-1, midRadius-1, 0.0f, -1.2f, 1.2f, true);
        g.strokePath(p, strokeType);

        auto innerBounds = middleBounds.reduced(middleBounds.getWidth() * 0.145f);
        ColourGradient innerGradient(Colour(0xff535661), center.getX(), center.getY()-radius*0.5f, Colour(0xffa9acba), center.getX(), center.getY() + radius, true);
        g.setGradientFill(innerGradient);
        g.fillEllipse(innerBounds);
        
        const auto sliderValueNormalised = (float)getNormalisableRange().convertTo0to1(getValue());
        drawValue(g, bounds, sliderValueNormalised);
        drawDot(g, middleBounds, sliderValueNormalised);
    }
    
    void drawDot(juce::Graphics& g, juce::Rectangle<float> bounds, float sliderPosProportional)
    {
        // dot
        const auto radius = bounds.getWidth() * 0.42f;
        const auto valueInRadians = valueRangeRadians.getStart() + valueRangeRadians.getLength() * sliderPosProportional - pi / 2.f;
        const auto x = std::cos(valueInRadians) * radius;
        const auto y = std::sin(valueInRadians) * radius;

        g.setColour(juce::Colour(0xffffffff));
        g.fillEllipse(juce::Rectangle(4.f, 4.f).withCentre(juce::Point(x, y) + bounds.getCentre()));
    }
    
    void drawValue(juce::Graphics& g, juce::Rectangle<float> bounds, float sliderPosProportional) const
    {
        const auto radius = (std::min(bounds.getWidth(), bounds.getHeight()) / 2.f) - valueArcThickness / 2 - inset;
        const auto strokeType = juce::PathStrokeType( valueArcThickness-1.f,
                                                     juce::PathStrokeType::JointStyle::curved,
                                                     juce::PathStrokeType::EndCapStyle::rounded);
        {
            auto startAngle = valueRangeRadians.getStart();
            auto endAngle = valueRangeRadians.getEnd();
            auto centerAngle = (startAngle + endAngle) / 2;
            auto centreX = bounds.getCentreX();
            auto centreY = bounds.getCentreY();
            auto angle = startAngle + valueRangeRadians.getLength() * sliderPosProportional;
            
            Path filledArc, filledArc1;
            if (isCentered) {
                juce::ColourGradient gradient (ringColorDark, centreX, 0, ringColorBright, centreX, centreY + radius - 10, false);
                g.setGradientFill(gradient);
                
                filledArc1.addCentredArc(centreX, centreY, radius, radius, 0.0f, centerAngle, angle, true);
                g.strokePath(filledArc1, strokeType);
                
                juce::ColourGradient gradient2 (ringColorBright.withAlpha(0.f), centreX, 0, ringColorBright.brighter(), centreX, centreY + radius - 10, false);
                g.setGradientFill(gradient2);
                
                filledArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, centerAngle, angle, true);
                g.strokePath(filledArc, strokeType);
                            
            }
            else {
                juce::ColourGradient gradient (ringColorDark, centreX-0.7*radius, centreY, ringColorBright, centreX + radius, centreY-0.5*radius, false);
                g.setGradientFill(gradient);
                                
                filledArc1.addCentredArc (centreX, centreY, radius, radius, 0.0f, startAngle, angle, true);
                g.strokePath (filledArc1, strokeType);

                juce::ColourGradient gradient2 (ringColorBright.withAlpha(0.f), centreX-0.7*radius, centreY, ringColorBright.brighter(), centreX + radius, centreY-0.5*radius, false);
                                
                g.setGradientFill (gradient2);
                filledArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, startAngle, angle, true);
                g.strokePath (filledArc, strokeType);
            }
            
        }
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        valueLabel.setVisible(true);
        updateLabel();
        Slider::mouseDown(e);
    }
    
    void mouseUp(const juce::MouseEvent& e) override
    {
        valueLabel.setVisible(false);
        Slider::mouseUp(e);
    }

    void sliderValueChanged(juce::Slider*) override
    {
        if (valueLabel.isVisible())
            updateLabel();
    }
    
    void setTheme(Colour bright, Colour dark)
    {
        ringColorBright = bright;
        ringColorDark = dark;
    }
    
    
private:
    
    bool isChoiceParam() const
    {
        return dynamic_cast<juce::AudioParameterChoice*>(param) != nullptr;
    }
    
    void updateLabel()
    {
        if (getComponentID() == PARAMS::TempoTime)
            valueLabel.setText(beatSync.getReference((int)getValue()), juce::dontSendNotification);
        else
            valueLabel.setText(juce::String((int)getValue()), juce::dontSendNotification);
    }
    
    StringArray beatSync =
    {
        "1", "1/2.d", "1/2", "1/2.t", "1/4.d", "1/4", "1/4.t", "1/8.d",
        "1/8", "1/8.t", "1/16.d", "1/16", "1/16.t", "1/32.d", "1/32",
    };
    
    bool isCentered { false };
    juce::RangedAudioParameter* param = nullptr;
    
    juce::Colour ringColorDark;
    juce::Colour ringColorBright;
    
    Label valueLabel;
    static constexpr auto inset = 2.f;
    static constexpr auto valueArcThickness = 3.f;
    static constexpr auto valueArcGapPercentage = 0.2f;
    static constexpr auto pi = juce::MathConstants<float>::pi;
    static constexpr auto valueArcGapRadians = pi * valueArcGapPercentage;
    inline static const auto valueRangeRadians = juce::Range(pi + valueArcGapRadians, pi + 2 * pi - valueArcGapRadians);
};

class GainSlider : public juce::Slider
{
public:
    GainSlider(const String& componentID)
    {
        setSliderStyle(juce::Slider::LinearVertical);
        setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        setComponentID(componentID);
        setSliderSnapsToMousePosition(false);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto w = bounds.getWidth();

        const auto sliderValueNormalised = (float)getNormalisableRange().convertTo0to1(getValue());
        const auto bgStrokeType = juce::PathStrokeType(2.f,
            juce::PathStrokeType::JointStyle::curved,
            juce::PathStrokeType::EndCapStyle::rounded);

        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        auto start = bounds.getBottom();
        auto end = bounds.getY();
        
        setMouseDragSensitivity((start-8) - (end+8));

        auto val = (start-8) - (((start-8) - (end+8)) * sliderValueNormalised);

        Path bg;
        bg.startNewSubPath(centerX, start-4);
        bg.lineTo(centerX, end+4);
        g.setColour(Colours::black.withAlpha(0.2f));
        g.strokePath(bg, bgStrokeType);
        
        g.setColour(Colour(0xff8f8f8f));
        g.fillRoundedRectangle(0, val-8.f, w, 16.f, 8.f);
        g.setColour(Colour(0xffc2c2c2));
        g.fillRoundedRectangle(2, val-6.f, w-4.f, 12.f, 8.f);
        g.setColour(Colours::black.withAlpha(0.3f));
        g.drawRoundedRectangle(2, val-6.f, w-4.f, 12.f, 8.f, 1.f);
    }
    
};

class EQComponent : public juce::Component, public juce::AudioProcessorValueTreeState::Listener
{
public:
    EQComponent(AudioProcessorValueTreeState& vts) : apvts(vts)
    {
        hpFreq = apvts.getRawParameterValue(PARAMS::GrainHP)->load();
        lpFreq = apvts.getRawParameterValue(PARAMS::GrainLP)->load();

        apvts.addParameterListener(PARAMS::GrainHP, this);
        apvts.addParameterListener(PARAMS::GrainLP, this);
    }
    
    ~EQComponent()
    {
        apvts.removeParameterListener(PARAMS::GrainHP, this);
        apvts.removeParameterListener(PARAMS::GrainLP, this);
    }
    
    void parameterChanged(const juce::String& paramID, float newValue) override
    {
        if (paramID == PARAMS::GrainHP)
            hpFreq = newValue;
        else if (paramID == PARAMS::GrainLP)
            lpFreq = newValue;
        
        repaint();
    }
    
    void paint(Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto w = bounds.getWidth();
        auto h = bounds.getHeight();

        // Background
        g.setColour(juce::Colour(0xff969696).withAlpha(0.3f));
        g.fillRoundedRectangle(bounds, 5.f);

        // Grid lines (optional but helpful)
        drawGrid(g, w, h);

        // Build and fill the response curve path
        auto responsePath = buildResponsePath(w, h);

        g.setColour(regionColor);
        auto filledPath = responsePath;
        filledPath.lineTo(w, h);
        filledPath.lineTo(0.f, h);
        filledPath.closeSubPath();
        g.fillPath(filledPath);

        g.setColour(pathColor);
        g.strokePath(responsePath, juce::PathStrokeType(2.f));

        drawNode(g, hpFreq, w, h, dragging == DragTarget::HP);
        drawNode(g, lpFreq, w, h, dragging == DragTarget::LP);
        
        g.setColour(Colour(0xff2d313b));
        Path touchUp1, touchUp2;
        touchUp1.startNewSubPath(0.f, h-4.2f);
        touchUp1.lineTo(4.2f, h);
        touchUp1.lineTo(0.f, h);
        touchUp1.closeSubPath();
        g.fillPath(touchUp1);
        
        touchUp2.startNewSubPath(w, h-4.2f);
        touchUp2.lineTo(w-4.2f, h);
        touchUp2.lineTo(w, h);
        touchUp2.closeSubPath();
        g.fillPath(touchUp2);
        
        ColourGradient borderGradient(Colour(0xff999999), w*0.25f, 0, Colour(0xffababab), w*0.66f, h, false);
        g.setGradientFill(borderGradient);
        g.drawRoundedRectangle(0.7f, 0.7f, w-1.4f, h-1.4f, 5.f, 0.7f);
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        auto w = (float)getWidth();
        auto h = (float)getHeight();

        auto hpPos = juce::Point<float>(freqToX(hpFreq, w), getYForFreq(hpFreq, h));
        auto lpPos = juce::Point<float>(freqToX(lpFreq, w), getYForFreq(lpFreq, h));

        if (e.position.getDistanceFrom(hpPos) <= nodeRadius * 2.f)
            dragging = DragTarget::HP;
        else if (e.position.getDistanceFrom(lpPos) <= nodeRadius * 2.f)
            dragging = DragTarget::LP;
        else
            dragging = DragTarget::None;
    }
    
    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (dragging == DragTarget::None) return;

        auto w = (float)getWidth();
        float newFreq = xToFreq(e.position.x, w);

        // Retrieve the parameter range to clamp correctly
        auto& param = *apvts.getParameter(
            dragging == DragTarget::HP ? PARAMS::GrainHP : PARAMS::GrainLP);
        auto range = apvts.getParameterRange(
            dragging == DragTarget::HP ? PARAMS::GrainHP : PARAMS::GrainLP);

        newFreq = range.snapToLegalValue(juce::jlimit(range.start, range.end, newFreq));

        // Prevent HP and LP from crossing over each other
        if (dragging == DragTarget::HP)
            newFreq = std::min(newFreq, lpFreq);
        else
            newFreq = std::max(newFreq, hpFreq);

        // Convert to 0..1 normalized value and set on the parameter
        float normalized = range.convertTo0to1(newFreq);
        param.setValueNotifyingHost(normalized);

        // Update local copy immediately for smooth repainting
        if (dragging == DragTarget::HP) hpFreq = newFreq;
        else lpFreq = newFreq;

        repaint();
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        dragging = DragTarget::None;
        repaint(); // redraw to drop the node highlight
    }

    // Change cursor to a left-right resize arrow when hovering over a node
    void mouseMove(const juce::MouseEvent& e) override
    {
        auto w = (float)getWidth();
        auto h = (float)getHeight();

        auto hpPos = juce::Point<float>(freqToX(hpFreq, w), getYForFreq(hpFreq, h));
        auto lpPos = juce::Point<float>(freqToX(lpFreq, w), getYForFreq(lpFreq, h));

        bool overNode = e.position.getDistanceFrom(hpPos) <= nodeRadius * 2.f
                         || e.position.getDistanceFrom(lpPos) <= nodeRadius * 2.f;

        setMouseCursor(overNode ? juce::MouseCursor::LeftRightResizeCursor
                                    : juce::MouseCursor::NormalCursor);
    }
    
    void setTheme(Colour bright)
    {
        pathColor = bright;
        regionColor = bright.withAlpha(0.3f);
    }
    
private:
    AudioProcessorValueTreeState& apvts;
    
    float hpFreq = 22000.f;
    float lpFreq = 20.f;
    enum class DragTarget { None, HP, LP };
    DragTarget dragging = DragTarget::None;
    
    static constexpr float freqMin = 18.f;
    static constexpr float freqMax = 24400.f;
    static constexpr float dbMin = -36.f;
    static constexpr float dbMax = 0.f;
    static constexpr float nodeRadius = 6.f;
    
    Colour pathColor;
    Colour regionColor;
    
    float freqToX(float freq, float width) const
    {
        return width * std::log(freq / freqMin) / std::log(freqMax / freqMin);
    }

    float xToFreq(float x, float width) const
    {
        return freqMin * std::pow(freqMax / freqMin, x / width);
    }

    // Convert a linear magnitude (0..~1) to a Y pixel coordinate
    float magToY(float magnitude, float height) const
    {
        float db = juce::Decibels::gainToDecibels(magnitude, dbMin - 1.f);
        db = juce::jlimit(dbMin, dbMax, db);
        float normalized = (db - dbMin) / (dbMax - dbMin);
        return (height * 0.9f) * (1.f - normalized) + (height * 0.05f); // 5% padding top and bottom
    }

    // Combined HP+LP magnitude at a given frequency
    float combinedMagnitude(float freq) const
    {
        float ratio_hp = freq / hpFreq;
        float ratio_lp = freq / lpFreq;

        float hp = std::pow(ratio_hp, 4.f) / std::sqrt(1.f + std::pow(ratio_hp, 8.f));
        float lp = 1.f / std::sqrt(1.f + std::pow(ratio_lp, 8.f));

        return hp * lp;
    }

    // Y position for a node sitting on the response curve at its cutoff freq
    float getYForFreq(float freq, float height) const
    {
        // At the cutoff frequency, a 1st-order filter is at -3dB (magnitude = 1/sqrt(2))
        // We compute the combined magnitude so the LP node accounts for the HP shelf too
        return magToY(combinedMagnitude(freq), height);
    }

    // Drawing helpers
    juce::Path buildResponsePath(float w, float h) const
    {
        juce::Path path;
        const int numPoints = static_cast<int>(w);

        for (int i = 0; i <= numPoints; ++i)
        {
            float x = (float)i;
            float freq = xToFreq(x, w);
            float y = magToY(combinedMagnitude(freq), h);

            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }

        return path;
    }

    void drawNode(juce::Graphics& g, float freq, float w, float h, bool isActive) const
    {
        float x = freqToX(freq, w);
        float y = getYForFreq(freq, h);

        // Outer ring
        g.setColour(isActive ? juce::Colours::white : juce::Colours::white.withAlpha(0.6f));
        g.drawEllipse(x - nodeRadius, y - nodeRadius,
                          nodeRadius * 2.f, nodeRadius * 2.f, 1.5f);

        // Inner fill
        g.setColour(isActive ? juce::Colours::white.withAlpha(0.4f)
                                 : juce::Colours::white.withAlpha(0.15f));
        g.fillEllipse(x - nodeRadius, y - nodeRadius,
                          nodeRadius * 2.f, nodeRadius * 2.f);
    }

    void drawGrid(juce::Graphics& g, float w, float h) const
    {
        g.setColour(juce::Colours::white.withAlpha(0.07f));

        // Vertical lines at musically useful frequencies
        for (float freq : { 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f })
        {
            float x = freqToX(freq, w);
            g.drawVerticalLine(static_cast<int>(x), 0.f, h);
        }

        // Horizontal lines at dB intervals
        for (float db = dbMin; db < dbMax; db += 12.f)
        {
            float y = h * (1.f - (db - dbMin) / (dbMax - dbMin));
            g.drawHorizontalLine(static_cast<int>(y), 0.f, w);
        }
    }
};
