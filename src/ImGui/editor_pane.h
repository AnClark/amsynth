#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl2.h>
#include <stdio.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>


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

    void setupGLFW();
    void setupImGui();

    void setParamChangeCallback(ParamChangeCallback func, AEffect *effInstance);

    GLFWwindow *getWindow() { return this->window; }
    ImGuiContext *getImguiContext() { return this->myImGuiContext; }

private:
    GLFWwindow *window;
    ImGuiContext *myImGuiContext = nullptr;
    void *parentId;

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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

static void imgui_drawing_thread(ImguiEditor *editorInstance);
