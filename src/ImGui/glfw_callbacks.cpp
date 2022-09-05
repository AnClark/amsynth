/*
 *  glfw_callbacks.cpp
 *
 *  Copyright (c) 2022 AnClark Liu
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

/**
 * Workaround for empty ImGui context issue on Windows
 *
 * On Windows, when I've used dedicated thread for ImGui rendering, and
 * used TLS for GImGui, there's still a bug:
 *
 * When I move mouse into editor window, plugin crashes. GDB shows that it
 * crashes when ImGui's GLFW callback function is accessing backend data
 * via ImGui_ImplGlfw_GetBackendData() (returns current ImGui context).
 *
 * This is because GLFW queries window events in main thread, not in
 * drawing thread. Since the two threads have their own GImGui instance,
 * GImGui will be null in main thread.
 *
 * @bear24rw (Max Thurn) provided a workaround: register our own glfw
 * callbacks to ensure the imgui context is set correctly before fowarding
 * the call to the imgui backend.
 *
 * This workaround doesn't aim at multithreading, but works well for me!
 *
 * Reference: https://github.com/ocornut/imgui/pull/3934#issuecomment-873213161
 *
 * WARNING: On Linux (with X11), This workaround will cause "Forgot to call
            Render() or EndFrame() at the end of the previous frame?" error.
 */

#include "editor_pane.h"

void ImguiEditor::_setMyGLFWCallbacks()
{
    // Define intermediate callback functions.
    // Those intermediates will execute callbacks defined in ImguiEditor's
    // instance.
    auto char_callback_func = [](GLFWwindow *w, unsigned int c) {
        static_cast<ImguiEditor *>(glfwGetWindowUserPointer(w))->_charCallback(c);
    };
    auto cursor_enter_callback_func = [](GLFWwindow *w, int entered) {
        static_cast<ImguiEditor *>(glfwGetWindowUserPointer(w))->_cursorEnterCallback(entered);
    };
    auto mouse_button_callback_func = [](GLFWwindow *w, int button, int action, int mods) {
        static_cast<ImguiEditor *>(glfwGetWindowUserPointer(w))->_mouseButtonCallback(button, action, mods);
    };
    auto scroll_callback_func = [](GLFWwindow *w, double xoffset, double yoffset) {
        static_cast<ImguiEditor *>(glfwGetWindowUserPointer(w))->_scrollCallback(xoffset, yoffset);
    };
    auto key_callback_func = [](GLFWwindow *w, int key, int scancode, int action, int mods) {
        static_cast<ImguiEditor *>(glfwGetWindowUserPointer(w))->_keyCallback(key, scancode, action, mods);
    };

    // Register my own callbacks
    glfwSetCharCallback(window, char_callback_func);
    glfwSetCursorEnterCallback(window, cursor_enter_callback_func);
    glfwSetMouseButtonCallback(window, mouse_button_callback_func);
    glfwSetScrollCallback(window, scroll_callback_func);
    glfwSetKeyCallback(window, key_callback_func);

    // Set window user pointer to ImguiEditor's current instance
    glfwSetWindowUserPointer(window, this);
}

// ---------- CALLBACKS ----------
// These callbacks will first set correct ImGui context, then invoke ImGui's own
// callbacks. This can prevent GLFW from accessing wrong ImGui instance.

void ImguiEditor::_charCallback(unsigned int c)
{
    ImGui::SetCurrentContext(this->myImGuiContext);
    ImGui_ImplGlfw_CharCallback(this->window, c);
}

void ImguiEditor::_cursorEnterCallback(int entered)
{
    ImGui::SetCurrentContext(this->myImGuiContext);
    ImGui_ImplGlfw_CursorEnterCallback(this->window, entered);
}

void ImguiEditor::_mouseButtonCallback(int button, int action, int mods)
{
    ImGui::SetCurrentContext(this->myImGuiContext);
    ImGui_ImplGlfw_MouseButtonCallback(this->window, button, action, mods);
}

void ImguiEditor::_scrollCallback(double xoffset, double yoffset)
{
    ImGui::SetCurrentContext(this->myImGuiContext);
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

void ImguiEditor::_keyCallback(int key, int scancode, int action, int mods)
{
    ImGui::SetCurrentContext(this->myImGuiContext);
    ImGui_ImplGlfw_KeyCallback(this->window, key, scancode, action, mods);
}
