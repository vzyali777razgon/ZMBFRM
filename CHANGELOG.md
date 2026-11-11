# Changelog

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
