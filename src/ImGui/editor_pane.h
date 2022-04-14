/*
 *  editor_pane.h
 *
 *  Copyright (c) 2021 AnClark Liu
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl2.h>
#include <stdio.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

/** ImGui addons */
#include <imgui_addons/zynlab/imgui_knob.h>
#include <imgui_addons/anclark/imgui_extra_button.h>
#include <imgui_addons/anclark/imgui_oscilloscope.h>

/** ImGui workarounds */
#include <imgui_workarounds/backends/imgui_opengl2_newframe.h>

/** Prefer WINAPI on Windows to reduce dependencies */
#include <atomic>
#include <thread>
#include <mutex>
#if _WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
#endif

/** Communicate with DSP part */
#include <vestige/aeffectx.h>
#include "../Preset.h"
#include "../midi.h"
#include "../MidiController.h"
#include "../PresetController.h"
#include "../Synthesizer.h"

#include "../gettext.h"
#define _(string) gettext (string)

typedef void (*ParamChangeCallback) (float[], AEffect *);

// Error code
#define ERR_GLFW_FAILURE 1
#define ERR_IMGUI_FAILURE 2

class ImguiEditor
{
public:
    ImguiEditor(void *parentId, int width, int height, Synthesizer *synthInstance);
    ~ImguiEditor();

    int openEditor();
    void closeEditor();
    void drawFrame();

    void setParamChangeCallback(ParamChangeCallback func, AEffect *effInstance);
    void panic();

    void setCurrentSample(int numSamples, float *samples);

private:
    GLFWwindow *window;
    ImGuiContext *myImGuiContext = nullptr;
    void *parentId;

    int _setupGLFW();
    int _setupImGui();

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool showPresetWindow = false;

    int width = 800;
    int height = 480;

    // Communicate with host
    AEffect *effInstance;
    Synthesizer *synthInstance;
    float paramList[kAmsynthParameterCount];
    char *paramNameList[kAmsynthParameterCount];
    ParamChangeCallback _onParamChange;

    // Data for oscilloscope
    int numCurrentSample;
    float *currentSample;

    // Parameter properties
    void _getParamProperties(int parameter_index, double *minimum, double *maximum, double *default_value, double *step_size);
    void _getAllParameters();
    double paramMinValues[kAmsynthParameterCount] = {0.0f};
    double paramMaxValues[kAmsynthParameterCount] = {0.0f};
    double paramDefaultValues[kAmsynthParameterCount] = {0.0f}; // NOTICE: Currently unnecessary, because DSP side will automatically
                                                                //         apply default values when needed
    double paramStepSizes[kAmsynthParameterCount] = {0.0f};     // NOTICE: step_increment is not supported by ImGui

    // Controllers

    void _AmsynthControl_AmpEnvAttack();
    void _AmsynthControl_AmpEnvDecay();
    void _AmsynthControl_AmpEnvSustain();
    void _AmsynthControl_AmpEnvRelease();

    void _AmsynthControl_Oscillator1Waveform();

    void _AmsynthControl_FilterEnvAttack();
    void _AmsynthControl_FilterEnvDecay();
    void _AmsynthControl_FilterEnvSustain();
    void _AmsynthControl_FilterEnvRelease();
    void _AmsynthControl_FilterResonance();
    void _AmsynthControl_FilterEnvAmount();
    void _AmsynthControl_FilterCutoff();

    void _AmsynthControl_Oscillator2Detune();   // OSC2 detune
    void _AmsynthControl_Oscillator2Waveform(); // OSC2 waveform

    void _AmsynthControl_MasterVolume(); // Main volume

    void _AmsynthControl_LFOFreq();     // LFO frequency (speed)
    void _AmsynthControl_LFOWaveform(); // LFO waveform

    void _AmsynthControl_Oscillator2Octave();
    void _AmsynthControl_OscillatorMix();

    void _AmsynthControl_LFOToOscillators(); // Freq Mod Amount
    void _AmsynthControl_LFOToFilterCutoff();
    void _AmsynthControl_LFOToAmp();

    void _AmsynthControl_OscillatorMixRingMod();

    void _AmsynthControl_Oscillator1Pulsewidth();
    void _AmsynthControl_Oscillator2Pulsewidth();

    void _AmsynthControl_ReverbRoomsize();
    void _AmsynthControl_ReverbDamp();
    void _AmsynthControl_ReverbWet();
    void _AmsynthControl_ReverbWidth();

    void _AmsynthControl_AmpDistortion();

    void _AmsynthControl_Oscillator2Sync();

    void _AmsynthControl_PortamentoTime();

    void _AmsynthControl_KeyboardMode();

    void _AmsynthControl_Oscillator2Pitch();
    void _AmsynthControl_FilterType();
    void _AmsynthControl_FilterSlope();

    void _AmsynthControl_LFOOscillatorSelect();

    void _AmsynthControl_FilterKeyTrackAmount();
    void _AmsynthControl_FilterKeyVelocityAmount();

    void _AmsynthControl_AmpVelocityAmount();

    void _AmsynthControl_PortamentoMode();

    void _AmsynthControl_Oscilloscope();

    // Sub window / panel

    void _AmsynthWindow_Preset();
};

// Toolings

int mini_hash(char *str); // Calculate string hash
