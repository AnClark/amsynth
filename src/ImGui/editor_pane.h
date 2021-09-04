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
#include "../PresetController.h"
#include "../Synthesizer.h"

#include "../gettext.h"
#define _(string) gettext (string)

typedef void (*ParamChangeCallback) (float[], AEffect *);

class ImguiEditor
{
public:
    ImguiEditor(void *parentId, int width, int height, Synthesizer *synthInstance);
    ~ImguiEditor();

    void openEditor();
    void closeEditor();
    void drawFrame();

    void setParamChangeCallback(ParamChangeCallback func, AEffect *effInstance);

private:
    GLFWwindow *window;
    ImGuiContext *myImGuiContext = nullptr;
    void *parentId;

    void _setupGLFW();
    void _setupImGui();

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

    void _getParamProperties(int parameter_index, double *minimum, double *maximum, double *default_value, double *step_size);
    void _getParamValues();
};

/** Shortcuts */

// Get parameter range before creating widget
// You must define float variables "lower", "upper", "step_increment" before this macro
#define fetchParamRange(index)  _getParamProperties(index, &lower, &upper, nullptr, &step_increment)
