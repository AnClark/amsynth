/*
 *  imconfig.h - User ImGui configuration
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

//-----------------------------------------------------------------------------
// COMPILE-TIME OPTIONS FOR DEAR IMGUI
//-----------------------------------------------------------------------------
// This is ImGui user config file, applied by '#define IMGUI_USER_CONFIG "my_imgui_config.h"' in CMakeLists.txt.
//-----------------------------------------------------------------------------

#pragma once

// ---- Workaround for ImGui thread-safety
// See imgui.cpp:957 for more details.
struct ImGuiContext;
extern thread_local ImGuiContext* MyImGuiTLS;
#define GImGui MyImGuiTLS
