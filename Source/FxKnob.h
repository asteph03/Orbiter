#pragma once
#include <JuceHeader.h>

using namespace juce;

class FxKnob : public juce::Slider
{
public:
    FxKnob()
    {
        setSliderStyle(juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle(juce::Slider::NoTextBox, true, 80, 12);
    }
    
    FxKnob(const String& componentID)
    {
        setSliderStyle(juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle(juce::Slider::NoTextBox, true, 80, 12);
        setComponentID(componentID);
        
        if (componentID == PARAMS::GrainPitch)
            isCentered = true;
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto center = bounds.getCentre();
        auto radius = bounds.getWidth() * 0.5f;
        
        g.setColour(Colours::black.withAlpha(0.5f));
        g.fillEllipse(bounds);
        
        auto middleBounds = bounds.reduced(bounds.getWidth() * 0.08f);
        ColourGradient background(Colour(0xff535661), Point<float>(bounds.getCentreX(), bounds.getBottom()), Colour(0xff878b9c), Point<float>(bounds.getCentreX(), bounds.getY()), false);
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
        ColourGradient innerGradient(Colour(0xff535661), center.getX(), center.getY()-radius*0.5f, Colour(0xff878b9c), center.getX(), center.getY() + radius, true);
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
    
    void setTheme(Colour bright, Colour dark)
    {
        ringColorBright = bright;
        ringColorDark = dark;
    }
    
    
private:
    
    bool isCentered { false };
    
    juce::Colour ringColorDark;
    juce::Colour ringColorBright;
    
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
        setSliderStyle(Slider::LinearVertical);
        setTextBoxStyle(juce::Slider::NoTextBox, true, 80, 12);
        setComponentID(componentID);
    }
    
    void paint(Graphics& g) override
    {
        auto bounds = getLocalBounds();
        
        g.setColour(Colours::white);
        g.fillAll();
    }
};
