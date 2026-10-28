# ZMBFRM v0.5 — experimental pitch engine + redesigned UI

JUCE 8.0.6 / CMake / VST3. Intended for Windows x64 and FL Studio.

## What's changed
- New dark teal/blue module-tab interface inspired by the ZMBTune concept mockup.
- ZMBTune screen with Key, 13 scales, Retune Speed, Humanize, Detune (-100..+100 cents), pitch tracking display, and clickable piano note reference.
- Added a first experimental monophonic vocal pitch correction path: autocorrelation-style pitch estimate, scale target, smoothing, and a lightweight dual-tap delay pitch shifter.
- Separate ring buffers for pitch processing and delay effect; JUCE DSP reverb is prepared and processed through the DSP API.
- Existing EQ/compressor/saturation/reverb/delay/chorus controls remain in the project.

## Important limitations
This is an **experimental MVP**, not a production-quality replacement for Auto-Tune/Waves Tune Real-Time. The pitch shifter may sound grainy, particularly on transients or polyphonic audio, and should be tested with a dry monophonic vocal. Pitch detection is approximate and latency/artifacts have not been measured in a DAW. The De-Esser tab is a UI placeholder; a de-esser DSP is not implemented yet. The EQ graph is a visual panel; the current EQ remains the existing three-band low/mid/high implementation, not a full parametric EQ.

## Build
Use the same GitHub Actions workflow as before (Windows x64, JUCE 8.0.6). CMake downloads JUCE from GitHub if it is not cached. This environment could not complete a local configure because network DNS access to github.com is unavailable, so the build is **not verified here**. After pushing the source, check the Actions build log before installing the VST3.
