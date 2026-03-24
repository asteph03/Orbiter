/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "FxKnob.h"
#include "FxButton.h"
#include "AudioVisualizer.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

static std::array<juce::Colour, 3> themeLightColor { Colour(0xff4a8df7), // STANDARD
                                                    Colour(0xffcc4141), // ORBIT
                                                    Colour(0xff969696) }; // BYPASS

static std::array<juce::Colour, 3> themeDarkColor { Colour(0xff366cc2), // STANDARD
                                                    Colour(0xffa32727), // ORBIT
                                                    Colour(0xff737373) }; // BYPASS

static std::array<juce::Colour, 3> levelLightColor { Colour(0xff7aa9f5), // STANDARD
                                                    Colour(0xffc97979), // ORBIT
                                                    Colour(0xffc4c4c4) }; // BYPASS

//==============================================================================
/**
*/
class CapstonePluginAudioProcessorEditor  : public juce::AudioProcessorEditor, public Timer
{
public:
    CapstonePluginAudioProcessorEditor (CapstonePluginAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~CapstonePluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void setTheme(bool o, bool b);
    void timerCallback() override;

private:
    CapstonePluginAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& apvts;
    AudioVisualizer& visualizer;
    VerticalMeter& leftMeter;
    VerticalMeter& rightMeter;
    
    bool orbit, bypass, sync;
    
    FxKnob* mixKnob;
    FxKnob* sizeKnob;
    FxKnob* densityKnob;
    FxKnob* pitchKnob;
    FxKnob* stereoKnob;
    FxKnob* sprayKnob;
    FxKnob* reverbKnob;
    FxKnob* swingKnob;
    FxKnob* beatKnob;
    OrbitButton* orbitButton;
    EnvelopeButton* envButton;
    BypassButton* bypassButton;
    GainSlider* gainSlider;
    EQComponent* eqComponent;
    SyncButton* syncButton;
    
    std::unique_ptr<SliderAttachment> mixAttach;
    std::unique_ptr<SliderAttachment> sizeAttach;
    std::unique_ptr<SliderAttachment> densityAttach;
    std::unique_ptr<SliderAttachment> pitchAttach;
    std::unique_ptr<SliderAttachment> stereoAttach;
    std::unique_ptr<SliderAttachment> sprayAttach;
    std::unique_ptr<SliderAttachment> reverbAttach;
    std::unique_ptr<SliderAttachment> swingAttach;
    std::unique_ptr<SliderAttachment> beatAttach;
    std::unique_ptr<ButtonAttachment> orbitAttach;
    std::unique_ptr<ButtonAttachment> bypassAttach;
    std::unique_ptr<SliderAttachment> gainAttach;
    std::unique_ptr<ButtonAttachment> syncAttach;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CapstonePluginAudioProcessorEditor)
};
