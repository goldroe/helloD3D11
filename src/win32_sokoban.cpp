
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#include <stdint.h>
#include <assert.h>

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

ID3D11Device *d3d_device = NULL;
ID3D11DeviceContext *d3d_context = NULL;
IDXGISwapChain *swap_chain = NULL;
ID3D11RenderTargetView *render_target_view = NULL;

bool global_running;

LRESULT CALLBACK
main_window_callback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_CLOSE:
            global_running = false;
            break;
        default:
            result = DefWindowProcA(window, message, wparam, lparam);
    }
    return result;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show)
{
    DXGI_SWAP_CHAIN_DESC swap_chain_descr = {0};
    swap_chain_descr.BufferDesc.RefreshRate.Numerator = 0;
    swap_chain_descr.BufferDesc.RefreshRate.Denominator = 0;
    swap_chain_descr.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    WNDCLASSA window_class = {};
    window_class.hInstance = instance;
    window_class.lpszClassName = "main_window_class";
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = main_window_callback;

    RegisterClassA(&window_class);
    
    HWND window = CreateWindowExA(0,
        window_class.lpszClassName, "Sokoban",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, instance, 0);

    global_running = true;
    while (global_running)
    {
        MSG message;
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        Sleep(10);
    }

    return 0;
}
