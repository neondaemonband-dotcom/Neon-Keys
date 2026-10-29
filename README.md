# Neon Keys VST3 (prototype)
Original C++/JUCE instrument inspired by the functional block diagram of the Korg Volca Keys. Not affiliated with Korg. This is not a circuit-accurate emulation.

Three oscillators, poly/unison/octave/fifth/ring modes, ring modulation, resonant 12 dB/oct lowpass approximation, ADSR, pitch EG, LFO, stereo cross-feedback delay, MIDI input, GUI and preset state.

## Build
Upload repository contents including `.github/workflows/build.yml` to a new GitHub repository. Open Actions and run workflow or push to main. Download `Neon-Keys-Windows-VST3` artifact after a successful run. Install the whole `.vst3` bundle to the Windows VST3 folder. Compilation and FL Studio 11 compatibility have NOT been verified.

## Limitations
Prototype architecture: simplified digital oscillators/filter, no hardware sequencer, voice mode is a numeric knob, pitch EG approximated from amplitude EG, delay is not a tape emulation. Verify audio and stability before production use.
