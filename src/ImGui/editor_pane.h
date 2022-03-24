#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx9.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <stdio.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

struct ImGuiContext;                              // Forward decls.
extern thread_local ImGuiContext *myImGuiContext; // ImGui editor context. Should be global

#include "d3d_helper.h"

/** Prefer WINAPI on Windows to reduce dependencies */
#include <atomic>
#include <thread>
#include <mutex>

class ImguiEditor
{
public:
    ImguiEditor(void *parentId, int width, int height);
    ~ImguiEditor();

    void openEditor();
    void closeEditor();

private:
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL};;
    HWND hwnd;
    void *parentId;

    int _setupDX9();
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
