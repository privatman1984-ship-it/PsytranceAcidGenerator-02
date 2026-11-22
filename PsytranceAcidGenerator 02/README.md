# HYPER ACID — Psytrance & Goatrance Generator (VST3 source)

A JUCE/C++17 software-instrument project aimed at driving Psytrance and Goatrance acid lines. The visual direction is dark, high-contrast, neon teal / amber, with a focused macro-control panel and a 16-step modulation lane.

## Sound engine
- Three selectable oscillator shapes plus a sub component
- Acid-style resonant low-pass filter with envelope and accent modulation
- Saturation / distortion stage
- LFO modulation
- 16-step pattern clock derived from DAW host BPM and PPQ position
- Step lane drawing by mouse click and drag
- Per-step modulation shape drives cutoff, resonance, gate depth and vocoder-style rhythmic modulation
- Tempo-division stereo ping-pong delay with feedback and stereo movement
- Host transport start/stop sync
- 100 deterministic preset slots with distinct parameter and step-pattern variations

## Build the VST3
Requirements:
- CMake 3.22+
- Git
- C++17 compiler (Visual Studio 2022 on Windows, Xcode on macOS, GCC/Clang on Linux)
- Internet connection for the first CMake configure (JUCE 8.0.6 is fetched from GitHub)

```bash
cmake -S . -B build
cmake --build build --config Release
```

The VST3 bundle is generated in:
`build/PsytranceAcidGenerator_artefacts/Release/VST3/`

Copy the generated `.vst3` bundle to your system's VST3 folder, then rescan plug-ins in your DAW.

## DAW sync
The internal sequencer reads the host BPM, PPQ position and transport state. Press Play in the DAW to run it; Stop halts the pattern and clears the delay buffer. The plug-in does not need a MIDI note to start its internal pattern.

## Important scope / quality notes
- This download is a **source-code project ZIP**, not a precompiled binary. A local build and DAW validation are required.
- The draw lane currently controls a linked per-step modulation macro that affects filter, resonance, gate and vocoder-style movement together. Independent editable lanes for each parameter are not implemented yet.
- “Step Vocoder” is a rhythmic/formant-style gate modulation, not a full FFT / multiband vocoder.
- The 100 factory slots are generated deterministically by the preset code. The JSON files in `Presets/` are descriptive preset-slot metadata, not separately sampled audio.
- The current oscillator and DSP chain are a compact starter synth engine. “High quality” depends on further listening tests, oversampling, anti-aliasing work and optimization in the target DAW. No mastering or loudness level is guaranteed.
- Please test at your intended sample rate and buffer size before production use.

## GitHub
Create a new repository and upload the project folder contents. Do not commit the generated `build/` directory.
