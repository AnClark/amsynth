/*
 *  imgui_opengl2_newframe.cpp
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

#include "imgui_opengl2_newframe.h"

// Include OpenGL header (without an OpenGL loader) requires a bit of fiddling
#if defined(_WIN32) && !defined(APIENTRY)
#define APIENTRY __stdcall // It is customary to use APIENTRY for OpenGL function pointer declarations on all platforms.  Additionally, the Windows OpenGL header needs APIENTRY.
#endif
#if defined(_WIN32) && !defined(WINGDIAPI)
#define WINGDIAPI __declspec(dllimport) // Some Windows OpenGL headers need this
#endif
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

struct ImGui_ImplOpenGL2_Data
{
    GLuint FontTexture;

    ImGui_ImplOpenGL2_Data() { memset(this, 0, sizeof(*this)); }
};

static ImGui_ImplOpenGL2_Data *ImGui_ImplOpenGL2_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplOpenGL2_Data *)ImGui::GetIO().BackendRendererUserData : NULL;
}

/**
 * @brief Workaround of ImGui_ImplOpenGL2_NewFrame() which won't assert.
 *
 * This workaround aims at VST editors. In VST SDK, event effEditClose and effEditIdle
 * has conflicts, which means effEditClose can be invoked WITHIN the process of effEditIdle.
 * So if developer destroys ImGui context in effEditClose, ImGui_ImplOpenGL2_NewFrame() may
 * end up with an empty context, then assertion error raises.
 *
 * In my opinion, such an assertion may work for standalone applications, but not suitable
 * for embedded ones. In VST editors, since there's no context, there's no need to continue
 * executing. Just remove assertion, return bool value instead so that you can terminate
 * drawing process if fails.
 *
 * @warning You should check return value. Once returns "false", the drawing function
 *          should exit right now. For example:
 *          if (!ImGui_ImplOpenGL2_NewFrame_NoAssert()) return;
 *
 *          Otherwise, unexpected things may happen.
 *
 * @return true New frame created. Means your program can continue.
 * @return false Failure. Means your program should not continue.
 */
bool ImGui_ImplOpenGL2_NewFrame_NoAssert()
{
    ImGui_ImplOpenGL2_Data *bd = ImGui_ImplOpenGL2_GetBackendData();

    if (bd == NULL)
        return false;

    if (!bd->FontTexture)
        ImGui_ImplOpenGL2_CreateDeviceObjects();

    return true;
}
