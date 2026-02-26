/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CapstonePluginAudioProcessorEditor::CapstonePluginAudioProcessorEditor (CapstonePluginAudioProcessor& p, juce::AudioProcessorValueTreeState& vts) : AudioProcessorEditor (&p), audioProcessor (p), apvts(vts), visualizer(p.visualizer)
{
    addAndMakeVisible(mixKnob = new FxKnob(PARAMS::GrainMix));
    mixAttach = std::make_unique<SliderAttachment>(apvts, PARAMS::GrainMix, *mixKnob);
    
    addAndMakeVisible(sizeKnob = new FxKnob(PARAMS::GrainSize));
    sizeAttach = std::make_unique<SliderAttachment>(apvts, PARAMS::GrainSize, *sizeKnob);
    
    addAndMakeVisible(densityKnob = new FxKnob(PARAMS::GrainDensity));
    densityAttach = std::make_unique<SliderAttachment>(apvts, PARAMS::GrainDensity, *densityKnob);
    
    addAndMakeVisible(stereoKnob = new FxKnob(PARAMS::GrainStereo));
    stereoAttach = std::make_unique<SliderAttachment>(apvts, PARAMS::GrainStereo, *stereoKnob);
    
    addAndMakeVisible(sprayKnob = new FxKnob(PARAMS::GrainOnset));
    sprayAttach = std::make_unique<SliderAttachment>(apvts, PARAMS::GrainOnset, *sprayKnob);
    
    addAndMakeVisible(pitchKnob = new FxKnob(PARAMS::GrainPitch));
    pitchAttach = std::make_unique<SliderAttachment>(apvts, PARAMS::GrainPitch, *pitchKnob);
    
    addAndMakeVisible(reverbKnob = new FxKnob(PARAMS::GrainReverb));
    reverbAttach = std::make_unique<SliderAttachment>(apvts, PARAMS::GrainReverb, *reverbKnob);
    
    addAndMakeVisible(orbitButton = new OrbitButton(PARAMS::GrainFreeze));
    orbitAttach = std::make_unique<ButtonAttachment>(apvts, PARAMS::GrainFreeze, *orbitButton);
    orbit = orbitButton->getToggleState();
    
    addAndMakeVisible(envButton = new EnvelopeButton(PARAMS::GrainEnvelope, apvts));
    //envAttach = std::make_unique<ButtonAttachment>(apvts, PARAMS::GrainEnvelope, *envButton);
    
    addAndMakeVisible(bypassButton = new BypassButton(PARAMS::GrainBypass));
    bypassAttach = std::make_unique<ButtonAttachment>(apvts, PARAMS::GrainBypass, *bypassButton);
    bypass = bypassButton->getToggleState();
    
    addAndMakeVisible(gainSlider = new GainSlider(PARAMS::OutputGain));
    gainAttach = std::make_unique<SliderAttachment>(apvts, PARAMS::OutputGain, *gainSlider);
    
    orbitButton->onClick = [this] {
        orbit = orbitButton->getToggleState();
        setTheme(orbit, bypass);
    };
    
    bypassButton->onClick = [this] {
        bypass = bypassButton->getToggleState();
        setTheme(orbit, bypass);
    };
    
    addAndMakeVisible(visualizer);
    
    setSize (700, 500);
    resized();
}

CapstonePluginAudioProcessorEditor::~CapstonePluginAudioProcessorEditor()
{
    mixAttach.reset();
    sizeAttach.reset();
    densityAttach.reset();
    stereoAttach.reset();
    sprayAttach.reset();
    pitchAttach.reset();
    reverbAttach.reset();
    orbitAttach.reset();
    //envAttach.reset();
    bypassAttach.reset();
    gainAttach.reset();
    
    if (mixKnob) { delete mixKnob; }
    if (sizeKnob) { delete sizeKnob; }
    if (densityKnob) { delete densityKnob; }
    if (stereoKnob) { delete stereoKnob; }
    if (sprayKnob) { delete sprayKnob; }
    if (pitchKnob) { delete pitchKnob; }
    if (reverbKnob) { delete reverbKnob; }
    if (orbitButton) { delete orbitButton; }
    if (envButton) { delete envButton; }
    if (bypassButton) { delete bypassButton; }
    if (gainSlider) { delete gainSlider; }
}

//==============================================================================
void CapstonePluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    auto w = bounds.getWidth();
    
    g.setColour(Colour(0xff1c253b));
    g.fillAll();
    
    ColourGradient gradient(Colour(0xff373c47), w*0.25, 0, Colour(0xff2d313b), w*0.75, h, false);
    g.setGradientFill(gradient);
    
    g.fillRoundedRectangle(8, 8, w-16, h-16, 5.f);
    g.setColour(Colour(0xff697182));
    g.drawRoundedRectangle(8, 8, w-16, h-16, 5.f, 1.f);
    
    Font f(FontOptions("Founders Grotesk Text", 12.f, Font::plain));
    g.setColour(Colour(0xffe3e3e3));
    g.setFont(f);
    
    g.drawText("Mix", 538, 468, 80, 12, Justification::centred);
    g.drawText("Grain Density", 30, 468, 80, 12, Justification::centred);
    g.drawText("Grain Size", 30, 372, 80, 12, Justification::centred);
    g.drawText("Stereo", 140, 372, 80, 12, Justification::centred);
    g.drawText("Onset Spray", 140, 468, 80, 12, Justification::centred);
    g.drawText("Pitch", 250, 372, 80, 12, Justification::centred);
    g.drawText("Reverb", 250, 468, 80, 12, Justification::centred);
    g.drawText("Mode", 391, 398, 80, 12, Justification::centred);
}

void CapstonePluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    float w = bounds.getWidth();
    float h = bounds.getHeight();
    
    setTheme(orbit, bypass);
    
    mixKnob->setBounds(w-160, h-120, 80, 80);
    sizeKnob->setBounds(38, h-200, 64, 64);
    densityKnob->setBounds(38, h-104, 64, 64);
    stereoKnob->setBounds(148, h-200, 64, 64);
    sprayKnob->setBounds(148, h-104, 64, 64);
    pitchKnob->setBounds(258, h-200, 64, 64);
    reverbKnob->setBounds(258, h-104, 64, 64);
    orbitButton->setBounds(348, h-78, 166, 56);
    envButton->setBounds(348, h-198, 166, 88);
    bypassButton->setBounds(w-160, h-198, 80, 48);
    visualizer.setBounds(36, 24, 588, 262);
    gainSlider->setBounds(638, 24, 40, 452);
}

void CapstonePluginAudioProcessorEditor::setTheme(bool o, bool b)
{
    int column;
    if (b) { // BYPASS OVERRIDES ORBIT/STANDARD
        column = 2;
    }
    else if (o) { // ORBIT
        column = 1;
    }
    else { // STANDARD
        column = 0;
    }
    
    mixKnob->setTheme(themeLightColor[column], themeDarkColor[column]);
    sizeKnob->setTheme(themeLightColor[column], themeDarkColor[column]);
    densityKnob->setTheme(themeLightColor[column], themeDarkColor[column]);
    stereoKnob->setTheme(themeLightColor[column], themeDarkColor[column]);
    sprayKnob->setTheme(themeLightColor[column], themeDarkColor[column]);
    pitchKnob->setTheme(themeLightColor[column], themeDarkColor[column]);
    reverbKnob->setTheme(themeLightColor[column], themeDarkColor[column]);
    orbitButton->setTheme(themeLightColor[column], themeDarkColor[column]);
    envButton->setTheme(themeLightColor[column], themeDarkColor[column]);
    visualizer.setTheme(themeLightColor[column]);
    
    repaint();
}
