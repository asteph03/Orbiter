/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CapstonePluginAudioProcessorEditor::CapstonePluginAudioProcessorEditor (CapstonePluginAudioProcessor& p, juce::AudioProcessorValueTreeState& vts) : AudioProcessorEditor (&p), audioProcessor (p), apvts(vts), visualizer(p.visualizer), leftMeter(p.leftMeter), rightMeter(p.rightMeter)
{
    startTimerHz(30);
    
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
    
    addAndMakeVisible(beatKnob = new FxKnob(PARAMS::TempoTime));
    beatAttach = std::make_unique<SliderAttachment>(apvts, PARAMS::TempoTime, *beatKnob);
    
    addAndMakeVisible(swingKnob = new FxKnob(PARAMS::TempoSwing));
    swingAttach = std::make_unique<SliderAttachment>(apvts, PARAMS::TempoSwing, *swingKnob);
    
    addAndMakeVisible(orbitButton = new OrbitButton(PARAMS::GrainFreeze));
    orbitAttach = std::make_unique<ButtonAttachment>(apvts, PARAMS::GrainFreeze, *orbitButton);
    orbit = orbitButton->getToggleState();
    
    addAndMakeVisible(envButton = new EnvelopeButton(PARAMS::GrainEnvelope, apvts));
    //envAttach = std::make_unique<ButtonAttachment>(apvts, PARAMS::GrainEnvelope, *envButton);
    
    addAndMakeVisible(bypassButton = new BypassButton(PARAMS::GrainBypass));
    bypassAttach = std::make_unique<ButtonAttachment>(apvts, PARAMS::GrainBypass, *bypassButton);
    bypass = bypassButton->getToggleState();
    
    addAndMakeVisible(syncButton = new SyncButton(PARAMS::TempoSync));
    syncAttach = std::make_unique<ButtonAttachment>(apvts, PARAMS::TempoSync, *syncButton);
    sync = syncButton->getToggleState();
    
    addAndMakeVisible(eqComponent = new EQComponent(apvts));
    
    addAndMakeVisible(leftMeter);
    addAndMakeVisible(rightMeter);
    
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
    
    syncButton->onClick = [this] {
        sync = syncButton->getToggleState();
        repaint();
        resized();
    };
    
    addAndMakeVisible(visualizer);
    
    setSize (700, 580);
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
    bypassAttach.reset();
    gainAttach.reset();
    syncAttach.reset();
    swingAttach.reset();
    beatAttach.reset();
    
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
    if (eqComponent) { delete eqComponent; }
    if (syncButton) { delete syncButton; }
    if (swingKnob) { delete swingKnob; }
    if (beatKnob) { delete beatKnob; }
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
    
    g.setColour(Colour(0xff2d313b));
    g.fillRoundedRectangle(8, 8, w-16, h-16, 5.f);
    g.setColour(Colour(0xff697182));
    g.drawRoundedRectangle(8, 8, w-16, h-16, 5.f, 1.f);
    
    Font f(FontOptions("Founders Grotesk Text", 12.f, Font::plain));
    g.setColour(Colour(0xffe3e3e3));
    g.setFont(f);
    
    String textString;
    
    if (sync)
        textString = "Swing";
    else
        textString = "Spray";
    
    g.drawText("Grain Size", 24, h-128, 80, 12, Justification::centred);
    g.drawText("Rate", 130, h-128, 80, 12, Justification::centred);
    g.drawText(textString, 236, h-128, 80, 12, Justification::centred);
    
    g.drawText("Stereo", 24, h-32, 80, 12, Justification::centred);
    g.drawText("Pitch", 130, h-32, 80, 12, Justification::centred);
    g.drawText("Reverb", 236, h-32, 80, 12, Justification::centred);
    
    g.drawText("Mix", 530, h-32, 80, 12, Justification::centred);
    g.drawText("Mode", 385, h-102, 80, 12, Justification::centred);
    g.drawText("Out", 618, h-32, 80, 12, Justification::centred);
}

void CapstonePluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    float w = bounds.getWidth();
    float h = bounds.getHeight();
    
    sprayKnob->setVisible(!sync);
    swingKnob->setVisible(sync);
    densityKnob->setVisible(!sync);
    beatKnob->setVisible(sync);
    
    setTheme(orbit, bypass);
    
    sizeKnob->setBounds(32, h-200, 64, 64);
    densityKnob->setBounds(138, h-200, 64, 64);
    beatKnob->setBounds(138, h-200, 64, 64);
    sprayKnob->setBounds(244, h-200, 64, 64);
    swingKnob->setBounds(244, h-200, 64, 64);
    
    stereoKnob->setBounds(32, h-104, 64, 64);
    pitchKnob->setBounds(138, h-104, 64, 64);
    reverbKnob->setBounds(244, h-104, 64, 64);
    
    mixKnob->setBounds(w-168, h-120, 80, 80);
    orbitButton->setBounds(342, h-78, 166, 56);
    envButton->setBounds(342, h-198, 166, 88);
    bypassButton->setBounds(w-176, h-198, 96, 32);
    syncButton->setBounds(w-176, h-162, 96, 32);
    visualizer.setBounds(24, 24, 596, 262);
    leftMeter.setBounds(636, 24, 20, 512);
    rightMeter.setBounds(660, 24, 20, 512);
    gainSlider->setBounds(634, 24, 48, 512);
    eqComponent->setBounds(32, 298, 580, 72);
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
    swingKnob->setTheme(themeLightColor[column], themeDarkColor[column]);
    beatKnob->setTheme(themeLightColor[column], themeDarkColor[column]);
    orbitButton->setTheme(themeLightColor[column], themeDarkColor[column]);
    envButton->setTheme(themeLightColor[column], themeDarkColor[column]);
    syncButton->setTheme(themeLightColor[column], themeDarkColor[column]);
    visualizer.setTheme(themeLightColor[column]);
    leftMeter.setTheme(levelLightColor[column], themeLightColor[column], themeDarkColor[column]);
    rightMeter.setTheme(levelLightColor[column], themeLightColor[column], themeDarkColor[column]);
    eqComponent->setTheme(themeLightColor[column]);
    
    repaint();
}

void CapstonePluginAudioProcessorEditor::timerCallback()
{
    leftMeter.repaint();
    rightMeter.repaint();
}
