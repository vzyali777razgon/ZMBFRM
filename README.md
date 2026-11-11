# ZMBFRM 2.0 — Vocal Production Suite

Development branch: `zmbfrm-2.0-dev`  
Target: Windows x64 · VST3 · JUCE 8 · FL Studio-compatible workflow

ZMBFRM is a vocal-processing effect built around a real-time pitch-correction engine and a set of mix-processing modules. The 2.0 development line is focused on completing the DSP, exposing every control clearly, and validating the plug-in build.

## Processing modules

- **ZMBTune** — monophonic vocal pitch correction with key and scale selection, retune speed, Humanize, and ±100-cent Detune.
- **ZMBEQ** — eight bands: low shelf, mid bell, high shelf, and five fully parametric bell bands. The EQ response graph supports direct frequency/gain editing.
- **ZMBComp** — threshold, ratio, attack, and release.
- **ZMBSaturation** — drive and wet mix.
- **ZMBDeEsser** — amount, detection frequency, and threshold.
- **ZMBReverb** — room amount and damping.
- **ZMBDelay** — delay time, feedback, and wet mix.
- **ZMBChorus** — depth and rate.
- **ZMBMaster** — global wet/dry and output level.

The processor supports mono and stereo main-bus layouts. The editor includes six editable factory starting points (Init / Neutral, Natural Vocal, Modern Pop, Rap Vocal, Hard Tune, and Wide Vocal), plus save/load support for user presets, live input/output level bars, a live pitch-history trace, a pre-EQ FFT spectrum overlay, and direct EQ-curve editing.

All exposed AudioProcessorValueTreeState parameters are host-automatable and saved/restored with the plug-in state. Existing parameter IDs are retained to protect sessions and automation.

## Pitch engine

The pitch detector uses a YIN/CMND-style periodicity analysis with DC removal, confidence checks, parabolic refinement, sample-rate-scaled analysis windows, and a fixed-size history buffer. Tracking confirms large pitch transitions over multiple observations, preserves short unvoiced gaps, and applies scale-note hysteresis.

The shifter uses two overlapping delay taps, Hann crossfading, cubic interpolation, a startup warmup, and host-reported latency. Speed controls correction response; Humanize extends the response and release times without reducing the selected scale correction depth.

The detector and shifter have been compiled in the automated VST3 workflow. Synthetic pitch-detector simulations are useful algorithm checks, but they are not a substitute for recording real vocals and listening in a DAW.

## Real-time and state handling

- Pitch history and processing scratch buffers are prepared before audio processing.
- Input and output samples are checked for non-finite values.
- EQ coefficient storage is allocated during preparation; parameter changes update fixed-size coefficient arrays in place.
- Plug-in state is serialized through the JUCE ValueTree state.
- The editor reads signal meters and pitch information through atomic processor telemetry.

## Build

GitHub Actions configures JUCE 8.0.6, builds the Windows x64 VST3 Release target, packages the `.vst3` bundle, and uploads a ZIP artifact.

A successful CI run verifies compilation and packaging only. It does **not** prove sound quality, CPU performance, or compatibility with every host configuration. Final acceptance still requires listening tests in FL Studio across sample rates, buffer sizes, mono/stereo routing, and automation scenarios.

## Development policy

All 2.0 work is isolated on `zmbfrm-2.0-dev`. The original `main` branch remains untouched. Avoid changing existing parameter IDs unless a migration path is implemented.
