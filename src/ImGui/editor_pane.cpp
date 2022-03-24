#include "editor_pane.h"

/**
 * @brief Initialize threading-related objects
 */
thread_local ImGuiContext *myImGuiContext;
std::mutex ImguiEditor::_init_lock;
std::atomic<int> ImguiEditor::instance_counter = 0;

ImguiEditor::ImguiEditor(void *parentId, int width, int height)
{
    this->parentId = parentId;
    this->width = width;
    this->height = height;
}

ImguiEditor::~ImguiEditor()
{
}

// TODO: Set a return value!
int ImguiEditor::_setupDX9()
{
    /* Belt and braces! This refcount is mostly for the imgui-dx9 backend. */
    auto inst_no = instance_counter.fetch_add(1);
    //if (inst_no == 0)
    {
        // Create application window
        //ImGui_ImplWin32_EnableDpiAwareness();
        ::RegisterClassEx(&wc);
        DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILD;
        hwnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui DirectX9 Example"), style, 100, 100, this->width, this->height, (HWND)this->parentId, NULL, wc.hInstance, NULL);

        if (!CreateDeviceD3D(hwnd))
        {
            CleanupDeviceD3D();
            ::UnregisterClass(wc.lpszClassName, wc.hInstance);

            printf("******************* ERROR setupDX9: Cannot create device *******************\n");
            return 1;
        }
    }

    if (g_pd3dDevice == NULL)
        printf("******************* ERROR setupDX9: g_pd3dDevice == NULL *******************\n");

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    return 0;
}

// TODO: Set a return value!
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
    ImGui_ImplWin32_Init(hwnd);

    if (g_pd3dDevice == NULL)
        printf("******************* ERROR setupImgui: g_pd3dDevice == NULL *******************\n");
    ImGui_ImplDX9_Init(g_pd3dDevice);

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
        _setupDX9();
        _setupImGui();
    }

    // Main loop
    while (!_running)
    {
        ImGui::SetCurrentContext(myImGuiContext); // Maybe unnecessary!

        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                _running = false;
        }
        if (!_running)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Draw windows
        {
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
        }

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    auto inst_no = instance_counter.fetch_add(-1);
    std::scoped_lock<std::mutex> lock(_init_lock);

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (inst_no <= 1)
    {
        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
    }
}

// TODO: Set return value!
void ImguiEditor::openEditor()
{
    /* Handle situations when open_view is called on an already
     * open editor or one that wasn't closed properly */
    if (_running == true)
    {
        //return false;
        return;
    }
    if (_update_thread.joinable())
    {
        _update_thread.join();
    }

    _running = true;
    try
    {
        _update_thread = std::thread(&ImguiEditor::_drawLoop, this);
    }
    catch (std::exception &e)
    {
        //std::cerr << "Failed to start draw thread: " << e.what() << std::endl;
        //return false;
        return;
    }

    return;
}

void ImguiEditor::closeEditor()
{
    //std::cout << "Closing window" << std::endl;

    _running = false;

    if (_update_thread.joinable())
    {
        _update_thread.join();
    }
}
