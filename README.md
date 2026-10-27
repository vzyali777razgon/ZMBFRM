# ZMBFRM source prototype v0.4

This archive contains C++ source code, **not a compiled or verified VST3 binary**.

## Changes in v0.4
- Fixed the ZMBSaturation enable parameter to be a Boolean, matching its UI toggle.
- Added a visible ZMBTune enable toggle.
- Changed the reverb wet/dry configuration to avoid adding an unnecessary dry signal inside the reverb stage.
- Updated project version and clarified the ZMBTune limitation in the interface.

## Included source DSP
- Three-band EQ, compressor, waveshaper saturation, chorus, reverb, delay, wet/dry blend and output gain.
- Saved parameter state and automatable parameters.

## Important limitation
**ZMBTune still does not detect or correct pitch.** Its key, scale, speed, humanize and enable controls are scaffolding only. This is not equivalent to Slate Digital MetaTune and must not be used as if it were an implemented autotune.

## Build on Windows 10 x64
Requirements: Visual Studio 2022 with Desktop development with C++, CMake 3.22+, Git and internet access. From Developer PowerShell in this folder:

```powershell
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

CMake downloads JUCE 8.0.6. Expected VST3 bundle location (if compilation succeeds):
`build\ZMBFRM_artefacts\Release\VST3\ZMBFRM.vst3`

Install the `.vst3` bundle under `C:\Program Files\Common Files\VST3\` and rescan plugins in FL Studio. Do not copy the source ZIP itself into the VST3 folder.

## Verification status
This project has not been compiled or tested in FL Studio in this environment. JUCE/CMake API compatibility and audio behavior still require a real Windows build and testing pass.
