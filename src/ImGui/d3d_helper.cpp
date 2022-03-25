#include "d3d_helper.h"
#include <windows.h>
#include <stdio.h>

#include "editor_pane.h"

// Data
LPDIRECT3D9 ImguiEditor::g_pD3D;
LPDIRECT3DDEVICE9 ImguiEditor::g_pd3dDevice;
D3DPRESENT_PARAMETERS ImguiEditor::g_d3dpp;

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((ImguiEditor::g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&ImguiEditor::g_d3dpp, sizeof(ImguiEditor::g_d3dpp));
    ImguiEditor::g_d3dpp.Windowed = TRUE;
    ImguiEditor::g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    ImguiEditor::g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    ImguiEditor::g_d3dpp.EnableAutoDepthStencil = TRUE;
    ImguiEditor::g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    ImguiEditor::g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
    //ImguiEditor::g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (ImguiEditor::g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &ImguiEditor::g_d3dpp, &ImguiEditor::g_pd3dDevice) < 0)
        return false;

    if (ImguiEditor::g_pd3dDevice == NULL)
        printf("******************* ERROR CreateDeviceD3d: ImguiEditor::g_pd3dDevice == NULL *******************\n");
    else
        printf("******************* OK CreateDeviceD3d: ImguiEditor::g_pd3dDevice won't be NULL *******************\n");

    return true;
}

void CleanupDeviceD3D()
{
    if (ImguiEditor::g_pd3dDevice)
    {
        ImguiEditor::g_pd3dDevice->Release();
        ImguiEditor::g_pd3dDevice = NULL;
    }
    if (ImguiEditor::g_pD3D)
    {
        ImguiEditor::g_pD3D->Release();
        ImguiEditor::g_pD3D = NULL;
    }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = ImguiEditor::g_pd3dDevice->Reset(&ImguiEditor::g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (ImguiEditor::g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            ImguiEditor::g_d3dpp.BackBufferWidth = LOWORD(lParam);
            ImguiEditor::g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
