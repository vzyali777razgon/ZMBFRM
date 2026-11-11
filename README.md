# ZMBFRM 2.0 — Vocal Production Suite

Development branch: `zmbfrm-2.0-dev`

Target platform: Windows x64 / VST3 / FL Studio.

## 2.0 goals

- Preserve the existing vocal-processing modules and parameter IDs wherever practical.
- Replace the current pitch shifter with a proper, tested monophonic vocal pitch-correction design.
- Redesign the editor around a wide, futuristic black-and-red visual system.
- Make the EQ display, pitch display and meters reflect real signal data.
- Keep the audio callback real-time safe: no heap allocation, locks, file access or UI work in `processBlock`.
- Validate the plugin with automated builds and then in FL Studio.

## Existing modules

- 8-band EQ (three main bands plus five parametric bands)
- Compressor
- Saturation
- De-esser
- Reverb
- Delay
- Chorus
- Pitch correction with key and scale selection

## Current technical findings

The current pitch estimator uses normalized autocorrelation with parabolic lag refinement. Its pitch shifter is a dual-tap delay design and is explicitly an MVP, not a transparent vocal tuner. The `delta` local in `pitchShiftSample` is calculated but unused; pitch movement is instead controlled by the shared delay-tap phase in `processBlock`. This architecture is a major candidate for replacement.

The current editor uses nine tabs and a cyan accent palette. The EQ graph draws the calculated response curve, not a live FFT spectrum. The tuner display reports a detected note but does not yet provide a true time-history pitch trace.

## Development safety

The original `main` branch is kept unchanged. All 2.0 work is isolated on `zmbfrm-2.0-dev`. Existing parameter IDs should remain stable unless a migration is implemented.

## Build

GitHub Actions builds the Windows x64 VST3 and uploads a ZIP artifact. The workflow can be started manually or runs on pushes to the development branch.

A successful CI build confirms compilation and packaging only; final sound quality and host compatibility still require listening and testing in FL Studio.