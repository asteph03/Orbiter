# Orbiter

Orbiter is a real-time granulator audio plugin built with [JUCE](https://juce.com/) and C++.

---

## What is a Granulator?

A granulator is an audio plugin — a software component that runs inside a Digital Audio Workstation **(DAW)** such as Ableton Live, Avid Pro-Tools, or Logic Pro. It applies granular synthesis to any incoming audio signal in real time, slicing it into tiny fragments called grains and reassembling them to create new soundscapes.

---

## Features

- Real-time granular processing of any incoming audio signal
- Independent control over grain size, playback rate, and playback position
- Randomization parameters for organic, evolving textures
- Dry/wet mix control for blending processed and original signal
- Orbit Mode which makes every grain feedback for several repeats
- Low and High-pass filtering for effective frequency control
- 4 different grain enveloping modes with unique sonic qualities
- Additional audio effects such as pitch shifting, plate reverb, and soft clip distortion
- Low-latency performance suitable for live use
- Clean, DAW-friendly UI built with JUCE

---

## Dependencies

- **[JUCE](https://juce.com/)** — Cross-platform C++ framework for audio application and plugin development

---

## Requirements

- Windows 10/11 (64-bit) or macOS 10.13+
- A .vst3 or .au compatible DAW (.aaxplugin format coming soon)
- **Windows:** Visual Studio 2022 with the "Desktop development with C++" workload
- **macOS:** Xcode 13 or later

---

## About

Developed by Aidan Stephenson as a senior design project at the University of Cincinnati.

Feel free to reach out at aidansteph03@gmail.com with any questions.
