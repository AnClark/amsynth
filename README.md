# amsynth (DPF implementation)

amsynth is an analog modelling (a.k.a virtual analog) software synthesizer.

This branch of amsynth is a reimplementation on DPF, aka. DISTRHO Plugin Framework. With help of DPF, it can generate several formats of plugins in one codebase, and it would be easier to maintain.

## Features of amsynth

* Dual oscillators (sine / saw / square / noise) with hard sync
* 12/24 dB/oct resonant filter (low-pass / high-pass / band-pass / notch)
* Mono / poly / legato keyboard modes
* Dual ADSR envelope generators (filter & amplitude)
* LFO which can modulate the oscillators, filter, and amplitude
* Distortion and reverb
* Hundreds of presets

## Improvements of this branch

* Full DSP part (without UI) implementation. Already for use with generic UI
* Embed factory banks into plugin binary
* Full factory program support (VST2/VST3/LV2)

## Versions

There are currently several different ways to run amsynth, with :

* VST2 Plug-in
* VST3 Plug-in
* LV2 Plug-in
* CLAP Plug-in
* Standalone application with JACK support (very limited features)

## Preset bank management

The factory banks are embedded into the plug-in binaries.

By default, all banks reside in `<PROJECT_DIR>/data/banks` will be converted into C++ code, then compiled as a part of plug-in.

Loading banks from external directories (upstream's behavior) is not yet implemented.

## How to build

### Clone this project first

```bash
# Clone this project
git clone https://github.com/anclark/amsynth.git -b dpf-implementation --recursive amsynth-dpf

# If you have already cloned, remember to update
# I frequently rebase my commits, so don't forget the --rebase argument!
cd amsynth-dpf
git pull origin dpf-implementation --rebase
```

### Common build

```bash
# Configure
cd amsynth-dpf
cmake -S . -B build -GNinja

# Build
cmake --build build
```

### Cross-build on Linux for Windows

```bash
# Configure
cd amsynth-dpf
cmake -S . -B build-win32 -GNinja -DCMAKE_TOOLCHAIN_FILE=windows-cross-build.cmake

# Build
cmake --build build
```

> **NOTICE: ** Win32 version of SDL2 is not yet provided, so the standalone version will not build, but it does not affect the plug-in versions.

### Use Clang instead of GCC

```bash
# Configure
cd amsynth-dpf
cmake -S . -B build-clang -GNinja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

# Build
cmake --build build-clang
```

### Where are built files?

Built files reside in `<CMAKE_BUILD_DIRECTORY>/bin`, for example, `build/bin`.

### What about those Autotools files?

Autotools files are not parts of DPF implementation. But you can still build the classic GTK2 standalone application and plug-in once you need.

## Notice

This project uses AnClark's own fork of DPF, which has features that upstream does not present (e.g. `DISTRHO::Plugin::getParameterDisplay()`).

## TO-DO lists

- [✅] Basic implemention
- [✅] Embedded factory program support
- [✅] VST2 effGetParamDisplay support
- [  ] Dear ImGui UI implementation
- [  ] Switch factory preset within plug-in UI
- [  ] External bank support
- [  ] MSVC-compatible
