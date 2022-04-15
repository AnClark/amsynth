/*
 *  editor_pane.cpp
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

#include "editor_pane.h"
#include "font.h"

static int glfw_initialized_cnt;

/**
 * Calculate string hash.
 * Reference: https://www.cnblogs.com/moyujiang/p/11213535.html
 */
int mini_hash(char *str)
{
    // Mod factors. Both P and MOD should be prime number.
    const int P = 13;
    const int MOD = 101;

    const int len = strlen(str);
    int hash[len] = {0};

    for (int i = 0; i < len; i++)
        hash[i] = (hash[i - 1]) * P + (int)(str[i]) % MOD; // Hash formula

    return hash[len - 1];
}

#ifdef _WIN32

/**
 * Set / reset window's parent on Windows.
 * I handle these two functions with my modded GLFW (https://github.com/anclark/GLFW)
 */
void reparent_window([[maybe_unused]] GLFWwindow *window, void *host_window) {}

void reparent_window_to_root([[maybe_unused]] GLFWwindow *window) {}
#else
/**
 * Set / reset window's parent on Linux.
 * I handle these two functions with my modded GLFW (https://github.com/anclark/GLFW)
 */
void reparent_window([[maybe_unused]] GLFWwindow *window, void *host_window) {}

void reparent_window_to_root([[maybe_unused]] GLFWwindow *window) {}
#endif

/**
 * Get a parameter's properties.
 * @param parameter_index Parameter's index.
 * @param minimum A pointed var to store parameter's minimum value.
 * @param maximum A pointed var to store parameter's maximum value.
 * @param default_value A pointed var to store parameter's default value. (Currently unneeded)
 * @param step_size A pointed var to store parameter's step increment size. (Currently unneeded)
 */
void ImguiEditor::_getParamProperties(int parameter_index, double *minimum, double *maximum, double *default_value, double *step_size)
{
    Preset preset;
    Parameter &parameter = preset.getParameter(parameter_index);

    if (minimum)
        *minimum = parameter.getMin();

    if (maximum)
        *maximum = parameter.getMax();

    if (default_value)
        *default_value = parameter.getValue();

    if (step_size)
        *step_size = parameter.getStep();
}

/**
 * Get all parameters' values and properties before creating any controllers.
 */
void ImguiEditor::_getAllParameters()
{
    for (int i = 0; i < kAmsynthParameterCount; i++)
    {
        // Get parameter values
        paramList[i] = (float)synthInstance->getParameterValue((Param)i);

        // Get parameter names
        paramNameList[i] = (char *)malloc(sizeof(char *) * 32);
        synthInstance->getParameterName((Param)i, paramNameList[i], 32);

        // Get parameter properties
        _getParamProperties(i, &paramMinValues[i], &paramMaxValues[i], &paramDefaultValues[i], &paramStepSizes[i]);
    }
}

void ImguiEditor::panic()
{
    // Build MIDI data of All Sound Off
    unsigned char buffer[3] = {MIDI_STATUS_CONTROLLER, MIDI_CC_ALL_SOUND_OFF, 0}; // Buffer format: {status, data1, data2}

    // Directly apply MIDI data
    synthInstance->getMidiController()->HandleMidiData(buffer, 3);
}

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
#if _WIN32
    MessageBoxA(NULL, (LPCSTR)description, "Error", MB_OK);
#endif
}

ImguiEditor::ImguiEditor(void *parentId, int width, int height, Synthesizer *synthInstance)
{
    this->parentId = parentId;
    this->width = width;
    this->height = height;
    this->synthInstance = synthInstance;
}

ImguiEditor::~ImguiEditor()
{
    // Destroy editor instance
    closeEditor();
}

void ImguiEditor::setParamChangeCallback(ParamChangeCallback func, AEffect *effInstance)
{
    this->_onParamChange = func;
    this->effInstance = effInstance;
}

void ImguiEditor::setCurrentSample(int numSamples, float *samples)
{
    this->numCurrentSample = numSamples;
    this->currentSample = samples;
}

int ImguiEditor::_setupGLFW()
{
    /**
     * Setup window as well as initialized count.
     * Here I manage a reference count of UI instances and keep glfw initialized
     * when you have multiple instances open
     *
     * This is Justin Frankel's implementation. Noizebox also has this feature
     * included in his modded GLFW (from which I forked)
     */
    if (!glfw_initialized_cnt++)
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return GLFW_FALSE;
    }

    // Omit explicit version specification to let GLFW guess GL version,
    // or GLFW will fail to load on old environments with GL 2.x
#if 0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Do not allow resizing
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Disable decoration. Or you will see a weird titlebar :-)

    // Enable embedded window
    glfwWindowHint(GLFW_EMBEDDED_WINDOW, GLFW_TRUE);
    glfwWindowHintVoid(GLFW_PARENT_WINDOW_ID, this->parentId);

    window = glfwCreateWindow(this->width, this->height, "Dear ImGui GLFW+OpenGL2 example", NULL, NULL); // This size is only the standalone window's size, NOT editor's size
    if (window == NULL)
        return GLFW_FALSE;

    // Embed editor to host
    // On both Windows and Linux, there's an implementation within my modded GLFW.
    // So they are empty functions now.
    reparent_window(window, this->parentId);

    // Explicitly set window position to avoid occasional misplace (offset)
    glfwSetWindowPos(window, 0, 0);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    return GLFW_TRUE;
}

int ImguiEditor::_setupImGui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    myImGuiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(myImGuiContext);

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Set actual editor UI size here
    io.DisplaySize.x = (float)this->width;
    io.DisplaySize.y = (float)this->height;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    /* The font is loaded from generated/font.h. The font file is in generated by the
     * binary_to_source utility included in Dear ImGui, this util is built and run by
     * CMake when generating the make files. Default font is Roboto
     * To change font, set the CMake varible INCLUDED_FONT */
    ImFontConfig config;
    io.Fonts->AddFontFromMemoryCompressedTTF(font_compressed_data, font_compressed_size, 16, &config);

    return GLFW_TRUE;
}

void ImguiEditor::drawFrame()
{
    // Get current parameter names, values and properties
    _getAllParameters();

    // Called once per idle slice
    // Remember to check myImGuiContext before drawing frames, or ImGui_ImplOpenGL2_NewFrame() may execute
    // on an empty context after closeEditor()!
    if (myImGuiContext)
    {
        ImGui::SetCurrentContext(myImGuiContext);

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Draw main editor window
        _AmsynthWindow_Main();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        //ImGuiIO &io = ImGui::GetIO();
        //glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
        // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
        //GLint last_program;
        //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        //glUseProgram(0);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        //glUseProgram(last_program);

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }
}

int ImguiEditor::openEditor()
{
    if (!_setupGLFW())
        return ERR_GLFW_FAILURE;

    if (!_setupImGui())
        return ERR_IMGUI_FAILURE;

    return 0;
}

void ImguiEditor::closeEditor()
{
    if (myImGuiContext)
    {
        // Set current context to make sure that the following two shutdown functions
        // can be in right context
        ImGui::SetCurrentContext(myImGuiContext);

        // Cleanup
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(myImGuiContext);
        myImGuiContext = nullptr;

        glfwDestroyWindow(window);
        if (!--glfw_initialized_cnt)
            glfwTerminate();
    }
}
