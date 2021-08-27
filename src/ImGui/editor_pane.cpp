#include "editor_pane.h"

static int glfw_initialized_cnt;

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

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
#if _WIN32
    MessageBoxA(NULL, (LPCSTR)description, "Error", MB_OK);
#endif
}

ImguiEditor::ImguiEditor(void *parentId, int width, int height)
{
    this->parentId = parentId;
    this->width = width;
    this->height = height;
}

ImguiEditor::~ImguiEditor()
{
    // Re-call closeEditor() in case user forget to call it
    closeEditor();
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
    io.DisplaySize.x = (float) this->width;
    io.DisplaySize.y = (float) this->height;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
}

void ImguiEditor::drawFrame()
{
    // Called once per idle slice
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

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
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
