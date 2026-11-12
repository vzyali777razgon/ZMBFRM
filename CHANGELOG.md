# Changelog

## 2.0.1 — Reliability and UI Fixes
- Fixed the tuner history writer so voiced pitch frames are recorded and the live pitch trace actually renders.
- Reset delay history when the delay is bypassed to prevent stale repeats from returning when re-enabled.
- Report reverb tail only when the reverb wet mix is audible.
- Recalculate editor layout whenever tabs change and clarify ON/BYPASSED module states.
- Removed project-owned compiler warnings in the editor and processor setup.


## 2.0.0 — Vocal Suite Completion Pass
- Exposed Gain, Frequency, and Q controls for all five parametric EQ bands.
- Added complete Chorus mix/feedback/centre-delay controls and independent Reverb mix/width controls.
- Removed per-change EQ coefficient object allocation from the audio callback by updating preallocated coefficient arrays in place.
- Added sample-by-sample smoothing for delay time, feedback, and wet mix automation.
- Added a real pitch-history trace to the tuner display, live input/output level bars, and a fixed-buffer logarithmic FFT spectrum overlay on the EQ graph.
- Added mono/stereo bus-layout support and updated reported tail duration for long feedback-delay settings.
- Added six editable factory vocal starting points and file-based save/load for user presets.
- Updated project documentation to reflect the current pitch engine, processing modules, state handling, and build limitations.


## 2.0.0 — Visual Identity Refresh
- Reworked the plugin's visual palette to a high-contrast black and signal-red production interface.
- Expanded the default editor canvas and minimum size for a more spacious workstation layout.
- Updated the interface branding and pitch-engine header for the 2.0 development line.


## 1.5.0 — Tune Engine Refinement
- Improved pitch detection precision using parabolic lag interpolation.
- Added a silence/low-level guard and raised confidence threshold to reduce false pitch estimates.
- Removed an unintended coupling where unvoiced frames reset input/output peak meters.
- Added gentle return-to-unity behavior during unvoiced frames.
- Retained 8-band EQ and all existing modules.

## 1.4.0
- Added five parametric bell bands alongside the three original EQ bands.
