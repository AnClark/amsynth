#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx9.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <d3d9.h>
#include <tchar.h>
#include <windows.h>

// Data
static LPDIRECT3D9 g_pD3D = NULL;
static LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
