# ZMBFRM 1.5.0 — Tune Engine Refinement

## Changes in this development build
- Refined the monophonic pitch estimator with normalized autocorrelation, a low-level/silence guard, and parabolic interpolation around the best lag.
- Tightened the voicing confidence threshold to reduce unstable pitch decisions on noisy/unvoiced material.
- Unvoiced frames now release the correction ratio gently toward unity instead of resetting the input/output peak meters.
- Retains the 8-band EQ work from v1.4.0 and the existing vocal processing modules.

## Honest limitations
The pitch-shifting stage is still a lightweight dual-tap delay shifter. Improving detection does not make it equivalent to commercial Antares/Waves/MetaTune algorithms: a high-quality tuner needs a purpose-built low-artifact pitch-shifting engine, careful latency management, vibrato/formant handling, and extensive listening tests. This source has not been compiled or auditioned in FL Studio in this environment. Treat this as an experimental development build.

## Build
Use the existing CMake workflow and build target `ZMBFRM_VST3`. Please share the complete CI/build log if it fails.
