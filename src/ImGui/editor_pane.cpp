#include "editor_pane.h"

thread_local ImGuiContext *myImGuiContext;
std::mutex ImguiEditor::_init_lock;

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
void reparent_window(GLFWwindow *window, void *host_window)
{
    HWND hwnd = glfwGetWin32Window(window);
    if (SetParent(hwnd, reinterpret_cast<HWND>(host_window)) == nullptr)
    {
        //MessageBoxA(NULL, GetLastError(), "Info", MB_OK);
    }
}

void reparent_window_to_root(GLFWwindow *window)
{
    HWND hWnd = glfwGetWin32Window(window);
    SetParent(hWnd, nullptr);
}
#else
void reparent_window(GLFWwindow *window, void *host_window)
{
    Window host_x11_win = reinterpret_cast<Window>(host_window);
    auto display = glfwGetX11Display();
    auto glfw_x11_win = glfwGetX11Window(window);

    XReparentWindow(display, glfw_x11_win, host_x11_win, 0, 0); // h_offset);
    glfwFocusWindow(window);
    XRaiseWindow(display, glfw_x11_win);
    XSync(display, true);
}

void reparent_window_to_root([[maybe_unused]] GLFWwindow *window) {}
#endif

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    MessageBoxA(NULL, (LPCSTR)description, "Error", MB_OK);
}

ImguiEditor::ImguiEditor(void *parentId)
{
    this->parentId = parentId;
}

ImguiEditor::~ImguiEditor()
{
    // Destroy editor instance
    closeEditor();
}

void ImguiEditor::_setupGLFW()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);     // Do not allow resizing
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);     // Disable decoration. Or you will see a weird titlebar :-)

    window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL2 example", NULL, NULL);    // This size is only the standalone window's size, NOT editor's size
    if (window == NULL)
        return;

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
    io.DisplaySize.x = 1280.0f;
    io.DisplaySize.y = 720.0f;

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

void ImguiEditor::_drawLoop()
{
    {
        std::scoped_lock<std::mutex> lock(_init_lock);
        _setupGLFW();
        _setupImGui();
    }

    // Main loop
    while (!glfwWindowShouldClose(window) && shouldEditorOn)
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
        //int display_w, display_h;
        //glfwGetFramebufferSize(window, &display_w, &display_h);

        ImGuiIO &io = ImGui::GetIO();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        //glViewport(0, 0, 1280, 720);

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

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(myImGuiContext);

    glfwDestroyWindow(window);
    glfwTerminate();
}

void ImguiEditor::openEditor()
{
    shouldEditorOn = true;
    //_drawLoop();

    /**
     * Create thread for editor.
     * Actually, a non-static member can also be a thread function,
     * but you need to use operator "&".
     * Omitting "&" is OK on Msys2 MinGW-w64, but won't allowed on Linux.
     */
    editorThread = std::thread(&ImguiEditor::_drawLoop, this);
}

void ImguiEditor::closeEditor()
{
    if (shouldEditorOn)
    {
        //reparent_window_to_root(window);
        shouldEditorOn = false;
        if (editorThread.joinable())
            editorThread.join();
    }
}
