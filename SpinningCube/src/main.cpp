
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#include <stdint.h>
#include <assert.h>

#include <math.h>
#include <DirectXMath.h>

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

typedef float float32_t;
typedef double float64_t;

using namespace DirectX;

namespace Colors
{
    XMFLOAT4 White = {1.0f, 1.0f, 1.0f, 1.0f};
    XMFLOAT4 Black = {0.0f, 0.0f, 0.0f, 1.0f};
    XMFLOAT4 Red = {1.0f, 0.0f, 0.0f, 1.0f};
    XMFLOAT4 Green = {0.0f, 1.0f, 0.0f, 1.0f};
    XMFLOAT4 Blue = {0.0f, 0.0f, 1.0f, 1.0f};
    XMFLOAT4 Yellow = {1.0f, 1.0f, 0.0f, 1.0f};
    XMFLOAT4 Cyan = {0.0f, 1.0f, 1.0f, 1.0f};
    XMFLOAT4 Magenta = {1.0f, 0.0f, 1.0f, 1.0f};
}

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

ID3D11Device *device = NULL;
ID3D11DeviceContext *device_context = NULL;
IDXGISwapChain *swap_chain = NULL;
ID3D11RenderTargetView *render_target_view = NULL;
D3D11_VIEWPORT viewport;

XMMATRIX world_view_proj;
XMFLOAT4X4 world, view, projection;
POINT last_mouse = {};
float theta, phi, radius;

bool global_running = false;

void
get_window_size(HWND window, int *width, int *height) {
    RECT client_rect;
    GetClientRect(window, &client_rect);
    *width = client_rect.right - client_rect.left;
    *height = client_rect.bottom - client_rect.top;
};

float
fclamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

LRESULT CALLBACK
main_window_callback(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    switch (message) {
        case WM_LBUTTONDOWN:
            last_mouse.x = GET_X_LPARAM(lparam);
            last_mouse.y = GET_Y_LPARAM(lparam);
            SetCapture(window);
            break;
        case WM_LBUTTONUP:
            ReleaseCapture();
            break;

        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            if (wparam & MK_LBUTTON) {
                float dx = XMConvertToRadians(0.25f * static_cast<float>(last_mouse.x - x));
                float dy = XMConvertToRadians(0.25f * static_cast<float>(last_mouse.y - y));
                // theta += dx;
                // phi += dy;
                // fclamp(phi, 0.1f, XM_PIDIV2 - 0.1f);
            }

            last_mouse.x = x;
            last_mouse.y = y;
        } break;

        case WM_SIZE: {
            int width, height;
            get_window_size(window, &width, &height);

            // Update viewport
            viewport = { 0, 0, (float)width, (float)height, 0.0f, 1.0f };

            // Update projection matrix
            float aspect_ratio = (float)width / (float)height;
            XMMATRIX P = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect_ratio, 0.01f, 10.0f);
            XMStoreFloat4x4(&projection, P);
        } break;
        case WM_CLOSE:
            global_running = false;
            break;
        default:
            result = DefWindowProcA(window, message, wparam, lparam);
    }
    return result;
}

int WINAPI
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show) {
    XMMATRIX I = XMMatrixIdentity();
    world_view_proj = I;
    XMStoreFloat4x4(&world, I);
    XMStoreFloat4x4(&view, I);
    XMStoreFloat4x4(&projection, I);

    theta = 0.0f;
    phi = XM_PIDIV4;
    radius = 5.0f;

    LARGE_INTEGER perf_frequency;
    QueryPerformanceFrequency(&perf_frequency);

    WNDCLASSA window_class = {};
    window_class.hInstance = instance;
    window_class.lpszClassName = "main_window_class";
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = main_window_callback;
    window_class.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
    RegisterClassA(&window_class);

    HWND window;
    {
        // RECT client_rect = {0, 0, 720, 720};
        // AdjustWindowRect(&client_rect, WS_OVERLAPPEDWINDOW, false);
        window = CreateWindowExA(0, window_class.lpszClassName, "HelloD3D11", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, instance, 0);
    }    
    assert(window);

    DXGI_SWAP_CHAIN_DESC swap_chain_descr = {0};
    swap_chain_descr.BufferDesc.RefreshRate.Numerator = 0;
    swap_chain_descr.BufferDesc.RefreshRate.Denominator = 0;
    swap_chain_descr.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swap_chain_descr.SampleDesc.Count = 1;
    swap_chain_descr.SampleDesc.Quality = 0;
    swap_chain_descr.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_descr.BufferCount = 1;
    swap_chain_descr.OutputWindow = window;
    swap_chain_descr.Windowed = true;

    D3D_FEATURE_LEVEL feature_level;
    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0, D3D11_SDK_VERSION, &swap_chain_descr, &swap_chain, &device, &feature_level, &device_context);
    assert(hr == S_OK && swap_chain && device && device_context);
    
    ID3D11Texture2D *backbuffer;
    hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backbuffer);
    assert(SUCCEEDED(hr));
    hr = device->CreateRenderTargetView(backbuffer, NULL, &render_target_view);
    assert(SUCCEEDED(hr));
    backbuffer->Release();

    flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ID3DBlob *vs_blob = NULL, *ps_blob = NULL, *error_blob = NULL;

    // Compile Vertex Shader
    hr = D3DCompileFromFile(L"shaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", flags, 0, &vs_blob, &error_blob);
    if (FAILED(hr)) {
        if (error_blob) {
            OutputDebugStringA((char *)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        if (vs_blob) {
            vs_blob->Release();
        }
        assert(false);
    }

    // Compile Pixel Shader
    hr = D3DCompileFromFile(L"shaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", flags, 0, &ps_blob, &error_blob);
    if (FAILED(hr)) {
        if (error_blob) {
            OutputDebugStringA((char *)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        if (ps_blob) {
            ps_blob->Release();
        }
        assert(false);
    }

    ID3D11VertexShader *vertex_shader = NULL;
    ID3D11PixelShader *pixel_shader = NULL;
    hr = device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), NULL, &vertex_shader);
    assert(SUCCEEDED(hr));

    hr = device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), NULL, &pixel_shader);
    assert(SUCCEEDED(hr));

    ID3D11InputLayout *input_layout = NULL;
    D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &input_layout);
    assert(SUCCEEDED(hr));
    
    Vertex vertices[] = {
        {XMFLOAT3(-1.0f, -1.0f, -1.0f), Colors::White},
        {XMFLOAT3(-1.0f, +1.0f, -1.0f), Colors::Black},
        {XMFLOAT3(+1.0f, +1.0f, -1.0f), Colors::Red},
        {XMFLOAT3(+1.0f, -1.0f, -1.0f), Colors::Green},
        {XMFLOAT3(-1.0f, -1.0f, +1.0f), Colors::Blue},
        {XMFLOAT3(-1.0f, +1.0f, +1.0f), Colors::Yellow},
        {XMFLOAT3(+1.0f, +1.0f, +1.0f), Colors::Cyan},
        {XMFLOAT3(+1.0f, -1.0f, +1.0f), Colors::Magenta},
    };
    UINT vertex_stride = sizeof(Vertex);
    UINT vertex_offset = 0;

    ID3D11Buffer *vertex_buffer = NULL;
    {
        D3D11_BUFFER_DESC vertex_buffer_desc = {};
        vertex_buffer_desc.ByteWidth = sizeof(vertices);
        vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
        vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA sr_data = {};
        sr_data.pSysMem = vertices;
        hr = device->CreateBuffer(&vertex_buffer_desc, &sr_data, &vertex_buffer);
        assert(SUCCEEDED(hr));
    }

    UINT indices[] = {
        //front face
        0, 1, 2,  0, 2, 3,
        // back face
        4, 6, 5,  4, 7, 6,
        // left face
        4, 5, 1,  4, 1, 0,
        // right face
        3, 2, 6,  3, 6, 7,
        // top face
        1, 5, 6,  1, 6, 2,
        // bottom face
        4, 0, 3,  4, 3, 7
    };
    UINT indices_count = ARRAYSIZE(indices);

    ID3D11Buffer *index_buffer = NULL;
    {
        D3D11_BUFFER_DESC index_buffer_desc = {};
        index_buffer_desc.ByteWidth = sizeof(indices);
        index_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
        index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA sr_data = {};
        sr_data.pSysMem = indices;
        hr = device->CreateBuffer(&index_buffer_desc, &sr_data, &index_buffer);
        assert(SUCCEEDED(hr));
    }

    ID3D11RasterizerState *rasterizer_state = NULL;
    {
        D3D11_RASTERIZER_DESC rasterizer_desc = {};
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_BACK;
        rasterizer_desc.DepthClipEnable = true;
        hr = device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);
        assert(SUCCEEDED(hr));
    }
    device_context->RSSetState(rasterizer_state);




    ID3D11Buffer *constant_buffer = NULL;
    {
        D3D11_BUFFER_DESC constant_desc = {};
        constant_desc.ByteWidth = sizeof(XMMATRIX);
        constant_desc.Usage = D3D11_USAGE_DYNAMIC;
        constant_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constant_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        D3D11_SUBRESOURCE_DATA sr_data = {};
        sr_data.pSysMem = &world_view_proj;
        hr = device->CreateBuffer(&constant_desc, &sr_data, &constant_buffer);
        assert(SUCCEEDED(hr));
    }
    device_context->VSSetConstantBuffers(0, 1, &constant_buffer);
    global_running = true;
    while (global_running) {
        MSG message;
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        float background_color[4] = { 0x39 / 255.0f, 0x54 / 255.0f, 0xD8 / 255.0f, 1.0f };
        device_context->ClearRenderTargetView(render_target_view, background_color);

        // Rasteriser Stage
        device_context->RSSetViewports(1, &viewport);

        // Output Merger
        device_context->OMSetRenderTargets(1, &render_target_view, NULL);

        // Input Assembler
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(input_layout);
        device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &vertex_stride, &vertex_offset);
        device_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);

        // Update view
        {
            // Convert spherical coordinates to cartesian
            float x = radius * sinf(phi) * cosf(theta);
            float z = radius * sinf(phi) * sinf(theta);
            float y = radius * cosf(phi);

            XMVECTOR eye = XMVectorSet(x, y, z, 1.0f);
            XMVECTOR target = XMVectorZero();
            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

            XMMATRIX V = XMMatrixLookAtLH(eye, target, up);
            XMStoreFloat4x4(&view, V);
        }

        // Update world_view_proj
        {
            XMMATRIX W = XMLoadFloat4x4(&world);
            XMMATRIX V = XMLoadFloat4x4(&view);
            XMMATRIX P = XMLoadFloat4x4(&projection);
            world_view_proj = W * V * P;
        }

        // Update constant buffer
        {
            D3D11_MAPPED_SUBRESOURCE mapped_resource;
            device_context->Map(constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource); // disables GPU access
            CopyMemory(mapped_resource.pData, &world_view_proj, sizeof(world_view_proj));
            device_context->Unmap(constant_buffer, 0);
        }
        device_context->VSSetConstantBuffers(0, 1, &constant_buffer);

        device_context->VSSetShader(vertex_shader, NULL, 0);
        device_context->PSSetShader(pixel_shader, NULL, 0);

        device_context->DrawIndexed(indices_count, 0, 0);

        swap_chain->Present(1, 0);

        theta += XMConvertToRadians(1.0f);
        Sleep(20);
    }

    return 0;
}
