
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
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

bool global_running = false;

LRESULT CALLBACK
main_window_callback(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    switch (message) {
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
        RECT client_rect = {0, 0, 720, 720};
        AdjustWindowRect(&client_rect, WS_OVERLAPPEDWINDOW, false);
        window = CreateWindowExA(0, window_class.lpszClassName, "HelloD3D11", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, client_rect.right, client_rect.bottom, NULL, NULL, instance, 0);
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
        {XMFLOAT3(-0.5f, -0.5f, 1.0f), Colors::Red},
        {XMFLOAT3(-0.5f, +0.5f, 1.0f), Colors::White},
        {XMFLOAT3(+0.5f, +0.5f, 1.0f), Colors::Green},
        {XMFLOAT3(+0.5f, -0.5f, 1.0f), Colors::Blue},
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
        0, 1, 2,
        0, 2, 3,
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

    XMMATRIX transform = XMMatrixIdentity();

    ID3D11Buffer *constant_buffer = NULL;
    {
        D3D11_BUFFER_DESC constant_desc = {};
        constant_desc.ByteWidth = sizeof(XMMATRIX);
        constant_desc.Usage = D3D11_USAGE_DYNAMIC;
        constant_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constant_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        D3D11_SUBRESOURCE_DATA sr_data = {};
        sr_data.pSysMem = &transform;
        hr = device->CreateBuffer(&constant_desc, &sr_data, &constant_buffer);
        assert(SUCCEEDED(hr));
    }
    device_context->VSSetConstantBuffers(0, 1, &constant_buffer);

    float theta = 0.0f;
    
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
        RECT client_rect;
        GetClientRect(window, &client_rect);
        float client_width = (float)(client_rect.right - client_rect.left);
        float client_height = (float)(client_rect.bottom - client_rect.top);
        D3D11_VIEWPORT viewport = { 0, 0, client_width, client_height, 0.0f, 1.0f };
        device_context->RSSetViewports(1, &viewport);

        // Output Merger
        device_context->OMSetRenderTargets(1, &render_target_view, NULL);

        // Input Assembler
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(input_layout);
        device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &vertex_stride, &vertex_offset);
        device_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);

        // Update transform matrix
        {
            XMFLOAT2 scaling = {1.0f, 1.0f};
            XMFLOAT2 rotation_origin = {0.0f, 0.0f};
            XMFLOAT2 translation = {0.0f, 0.0f};
            XMVECTOR scaling_vector = XMLoadFloat2(&scaling);
            XMVECTOR rotation_origin_vector = XMLoadFloat2(&rotation_origin);
            XMVECTOR translation_vector = XMLoadFloat2(&translation);
            transform = XMMatrixAffineTransformation2D(scaling_vector, rotation_origin_vector, XMConvertToRadians(theta), translation_vector);
        }

        // Update constant buffer
        {
            D3D11_MAPPED_SUBRESOURCE mapped_resource;
            device_context->Map(constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource); // disables GPU access
            CopyMemory(mapped_resource.pData, &transform, sizeof(transform));
            device_context->Unmap(constant_buffer, 0);
        }
        device_context->VSSetConstantBuffers(0, 1, &constant_buffer);

        device_context->VSSetShader(vertex_shader, NULL, 0);
        device_context->PSSetShader(pixel_shader, NULL, 0);

        device_context->DrawIndexed(indices_count, 0, 0);

        swap_chain->Present(1, 0);

        theta += 3.0f;

        Sleep(20);
    }

    return 0;
}
