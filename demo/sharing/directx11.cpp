#include <backends/imgui_impl_sdl2.h>
#include <imgui.h>
#include <SDL.h>
#include <stb_image.h>
#include <stdio.h>

#include <filesystem>
#include <functional>

#include <compute/compute.hpp>

static SDL_Window* g_window = nullptr;

void create_window()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        // printf("Error: %s\n", SDL_GetError());
        // return -1;
    }
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    g_window = SDL_CreateWindow("Dear ImGui SDL2+DirectX11 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (g_window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        // return -1;
    }
    // return _window;
}

void process_events(bool& done, const std::function<void()>& resize_buffers)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            done = true;
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(g_window))
            done = true;
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(g_window)) {
            resize_buffers();
        }
    }
}

struct texture_data {
    std::size_t width;
    std::size_t height;
    std::size_t channels;
    std::vector<unsigned char> pixels;
};

texture_data load_texture(const std::filesystem::path& filename)
{
    texture_data _texture;
    std::string _spath = filename.string();
    const char* _cpath = _spath.c_str();
    int _width, _height, _channels;
    unsigned char* _data = stbi_load(_cpath, &_width, &_height, &_channels, 4);
    if (_channels != 4) {
        throw std::runtime_error("Impossible to create texture because channels count is invalid (expecting 4)");
    }
    _texture.width = static_cast<unsigned int>(_width);
    _texture.height = static_cast<unsigned int>(_height);
    _texture.channels = static_cast<unsigned int>(_channels);
    _texture.pixels = std::vector<unsigned char>(_data, _data + _channels * _width * _height);
    stbi_image_free(_data);
    return _texture;
}

//
//
//
//

#include <SDL_syswm.h>
#include <backends/imgui_impl_dx11.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

void d3d11_create_render_target()
{
    ID3D11Texture2D* _backbuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&_backbuffer));
    g_pd3dDevice->CreateRenderTargetView(_backbuffer, nullptr, &g_mainRenderTargetView);
    _backbuffer->Release();
}

void d3d11_create_device(SDL_Window* window)
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = (HWND)wmInfo.info.win.window;
    DXGI_SWAP_CHAIN_DESC _swapchain_desc;
    ZeroMemory(&_swapchain_desc, sizeof(_swapchain_desc));
    _swapchain_desc.BufferCount = 2;
    _swapchain_desc.BufferDesc.Width = 0;
    _swapchain_desc.BufferDesc.Height = 0;
    _swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    _swapchain_desc.BufferDesc.RefreshRate.Numerator = 60;
    _swapchain_desc.BufferDesc.RefreshRate.Denominator = 1;
    _swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    _swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    _swapchain_desc.OutputWindow = hwnd;
    _swapchain_desc.SampleDesc.Count = 1;
    _swapchain_desc.SampleDesc.Quality = 0;
    _swapchain_desc.Windowed = TRUE;
    _swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    D3D_FEATURE_LEVEL _feature_level;
    const D3D_FEATURE_LEVEL _feature_level_array[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, _feature_level_array, 2, D3D11_SDK_VERSION, &_swapchain_desc, &g_pSwapChain, &g_pd3dDevice, &_feature_level, &g_pd3dDeviceContext) != S_OK) {
        std::runtime_error("");
    }
    d3d11_create_render_target();
}

void d3d11_cleanup_render_target()
{
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

void d3d11_cleanup_device()
{
    d3d11_cleanup_render_target();
    if (g_pSwapChain) {
        g_pSwapChain->Release();
        g_pSwapChain = nullptr;
    }
    if (g_pd3dDeviceContext) {
        g_pd3dDeviceContext->Release();
        g_pd3dDeviceContext = nullptr;
    }
    if (g_pd3dDevice) {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }
}

void d3d11_resize_render_target()
{
    d3d11_cleanup_render_target();
    g_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    d3d11_create_render_target();
}

ID3D11ShaderResourceView* d3d11_create_srv(ID3D11Texture2D* texture)
{
    ID3D11ShaderResourceView* _srv = nullptr;
    HRESULT _d3d11_status = g_pd3dDevice->CreateShaderResourceView(texture, nullptr, &_srv);
    if (FAILED(_d3d11_status)) {
        throw std::runtime_error("Failed to create srv");
    }
    return _srv;
}

void d3d11_render_srv(ID3D11ShaderResourceView* srv, const std::size_t width, const std::size_t height)
{
    ImTextureID _imgui_texture_id = reinterpret_cast<ImTextureID>(srv);
    ImVec2 _image_size(static_cast<float>(width), static_cast<float>(height));
    ImGui::Begin("Texture Viewer");
    ImGui::Image(_imgui_texture_id, _image_size);
    ImGui::End();
}

void d3d11_new_frame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void d3d11_present()
{
    ImGui::Render();
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_pSwapChain->Present(0, 0); // without vsync
}

//
//
//
//

int main(int argc, char** argv)
{
    create_window();
    d3d11_create_device(g_window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsLight();
    ImGui_ImplSDL2_InitForD3D(g_window);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    //
    //

    texture_data _texture_data = load_texture("C:/Users/adri/desktop/495820.png");

    const char* _kernel_source = R"(
        kernel void drawBox(__write_only image2d_t output, float fDimmerSwitch)
        {
            int x = get_global_id(0);
            int y = get_global_id(1);
            int xMin = 0, xMax = 600, yMin = 0, yMax = 50;
            if((x >= xMin) && (x <= xMax) && (y >= yMin) && (y <= yMax))
            {      
                write_imagef(output, (int2)(x, y), (float4)(0.f, 0.f, fDimmerSwitch, 1.f));
            }
        } 
    )";

    compute::context _context = compute::context::create_d3d11(g_pd3dDevice, g_pd3dDeviceContext);
    compute::texture_2d _shared_texture(_context, _texture_data.width, _texture_data.height, _texture_data.pixels);
    compute::kernel _kernel(_context, _kernel_source, "drawBox");

    ID3D11ShaderResourceView* _srv = d3d11_create_srv(_shared_texture.get_d3d11());

    bool _done = false;
    while (!_done) {
        process_events(_done, d3d11_resize_render_target);

        float _dimmer_switch = 0.4f;
        _kernel.set_arg(0, _shared_texture);
        _kernel.set_arg(1, sizeof(float), &_dimmer_switch);
        _shared_texture.acquire();
        _kernel.run({ _texture_data.width, _texture_data.height });
        _shared_texture.release();

        d3d11_new_frame();
        d3d11_render_srv(_srv, 400, 400);
        d3d11_present();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    d3d11_cleanup_device();
    SDL_DestroyWindow(g_window);
    SDL_Quit();
    return 0;
}