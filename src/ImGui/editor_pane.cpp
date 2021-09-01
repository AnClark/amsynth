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
void reparent_window([[maybe_unused]] GLFWwindow *window, void *host_window)
{
}

void reparent_window_to_root([[maybe_unused]] GLFWwindow *window)
{
}
#else
/**
 * Set / reset window's parent on Linux.
 * I handle these two functions with my modded GLFW (https://github.com/anclark/GLFW)
 */
void reparent_window([[maybe_unused]] GLFWwindow *window, void *host_window) {}

void reparent_window_to_root([[maybe_unused]] GLFWwindow *window) {}
#endif

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

void ImguiEditor::_getParamValues()
{
    for (int i = 0; i < kAmsynthParameterCount; i++)
    {
        paramList[i] = (float)synthInstance->getParameterValue((Param)i);

        paramNameList[i] = (char *)malloc(sizeof(char *) * 32);
        synthInstance->getParameterName((Param)i, paramNameList[i], 32);
    }
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

void ImguiEditor::_setupGLFW()
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
            return;
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
        return;

    // Embed editor to host
    // On both Windows and Linux, there's an implementation within my modded GLFW.
    // So they are empty functions now.
    reparent_window(window, this->parentId);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
}

void ImguiEditor::_setupImGui()
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
}

void ImguiEditor::drawFrame()
{
    // Get current parameter names and values
    _getParamValues();

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

        // 1. Show a window listing all those possible parameters
        {
            ImGui::Begin("Hello, Amsynth! - Parameter summary");

            for (int i = 0; i < kAmsynthParameterCount; i++)
            {
                ImGui::Text("[%s]: %f", paramNameList[i], paramList[i]);
            }

            ImGui::End();
        }

        // 2. Show a window with siiders controlling all those parameters
        {
            ImGui::Begin("Hello, Amsynth! - Simple parameter controller");

            for (int i = 0; i < kAmsynthParameterCount; i++)
            {
                double lower = 0, upper = 0, step_increment = 0; // NOTICE: step_increment is not supported by ImGui
                _getParamProperties(i, &lower, &upper, nullptr, &step_increment);

                if (ImGui::SliderFloat(paramNameList[i], &paramList[i], (float)lower, (float)upper))
                {
                    _onParamChange(paramList, effInstance);
                }
            }

            ImGui::End();
        }

        // 3. Show a window listing all the presets
        {
            ImGui::Begin("Hello Amsynth! - Preset list");

            for (auto &bank : PresetController::getPresetBanks())
            {
                char text[64]; // Buffer

                /**
                 * a. Create root nodes for each bank 
                 */
                snprintf(text, sizeof(text), "[%s] %s", bank.read_only ? _("F") : _("U"), bank.name.c_str());
                if (ImGui::TreeNode(text))
                {
                    /**
                     * b. Create child nodes for each preset item 
                     *    Each bank has up to 127 presets, accessed by index.
                     */
                    static int selected = -1; // Current selection's index

                    PresetController presetController;
                    presetController.loadPresets(bank.file_path.c_str());
                    for (int i = 0; i < PresetController::kNumPresets; i++)
                    {
                        snprintf(text, sizeof(text), "%d: %s", i, presetController.getPreset(i).getName().c_str());

                        char *bank_file_path = strdup(bank.file_path.c_str());
                        size_t preset_index = (size_t)i;

                        /**
                         * c. Apply preset when you click a preset item 
                         */
                        int node_index = i + mini_hash(text); // Calculate unique index (almost unique among normal usages)
                        if (ImGui::Selectable(text, selected == node_index))
                        {
                            selected = node_index; // Mark selected item

                            PresetController presetController;
                            presetController.loadPresets(bank_file_path); // Load preset bank

                            Preset &preset = presetController.getPreset((int)preset_index); // Load preset item
                            for (unsigned int i = 0; i < kAmsynthParameterCount; i++)       // Apply preset parameters
                            {
                                float value = preset.getParameter(i).getValue();
                                paramList[i] = value;
                                _onParamChange(paramList, effInstance);
                            }
                        }
                    }

                    ImGui::TreePop(); // Must add this, or ImGui will crash!
                }
            }

            ImGui::End();
        }

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

void ImguiEditor::openEditor()
{
    _setupGLFW();
    _setupImGui();
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
