#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl2.h>
#include <stdio.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

struct ImGuiContext;                              // Forward decls.
extern thread_local ImGuiContext *myImGuiContext; // ImGui editor context. Should be global

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

class ImguiEditor
{
public:
    ImguiEditor(void *parentId, int width, int height);
    ~ImguiEditor();

    void openEditor();
    void closeEditor();

private:
    GLFWwindow *window;
    void *parentId;

    void _setupGLFW();
    void _setupImGui();
    void _drawLoop();

    static std::atomic<int> instance_counter; // To avoid crash when more than one instance
    std::atomic_bool _running{false};         // Mark if drawing thread is running
    std::thread _update_thread;               // Thread for drawing UI

    static std::mutex _init_lock; // To avoid conflict or dead lock when initializing ImGui

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    int width = 800;
    int height = 480;
};
