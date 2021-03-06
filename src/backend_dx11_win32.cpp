// this file is modified from imgui/examples/example_win32_directx11/main.cpp

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "application.h"
#include "platform.h"

#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
    // Data
    ID3D11Device* g_pd3dDevice                     = NULL;
    ID3D11DeviceContext* g_pd3dDeviceContext       = NULL;
    IDXGISwapChain* g_pSwapChain                   = NULL;
    ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

    // Forward declarations of helper functions
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    class PlatformWindow_Win32 final : public PlatformWindow
    {
    private:
        HWND window_;

    public:
        PlatformWindow_Win32(HWND window)
        {
            window_ = window;
        }

        virtual void SetTitle(const std::string& title) override
        {
            SetWindowTextA(window_, title.c_str());
        }

        virtual void SetSize(int width, int height) override
        {
            auto [xpos, ypos] = GetPosition();
            MoveWindow(window_, xpos, ypos, width, height, true);
        }
        virtual std::pair<int, int> GetSize() override
        {
            RECT rect;
            GetWindowRect(window_, &rect);

            int width  = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            return {width, height};
        }

        virtual void SetPosition(int x, int y) override
        {
            auto [width, height] = GetSize();
            MoveWindow(window_, x, y, width, height, true);
        }
        virtual std::pair<int, int> GetPosition() override
        {
            RECT rect;
            GetWindowRect(window_, &rect);
            return {rect.left, rect.top};
        }
    };

    class PlatformTexture_Dx11 final : public PlatformTexture
    {
    private:
        ID3D11ShaderResourceView* texSRV = nullptr;
        ID3D11Texture2D* tex             = nullptr;

    public:
        PlatformTexture_Dx11() = default;
        ~PlatformTexture_Dx11() override
        {
            Cleanup();
        }

        bool Initialize(int width, int height)
        {
            width_  = width;
            height_ = height;

            D3D11_TEXTURE2D_DESC desc = {};
            desc.Width                = width;
            desc.Height               = height;
            desc.MipLevels            = 1;
            desc.ArraySize            = 1;
            desc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count     = 1;
            desc.Usage                = D3D11_USAGE_DYNAMIC;
            desc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags       = D3D10_CPU_ACCESS_WRITE;

            HRESULT hr = g_pd3dDevice->CreateTexture2D(&desc, nullptr, &tex);

            if (SUCCEEDED(hr))
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
                SRVDesc.Format                          = DXGI_FORMAT_R8G8B8A8_UNORM;
                SRVDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
                SRVDesc.Texture2D.MipLevels             = 1;

                hr = g_pd3dDevice->CreateShaderResourceView(tex, &SRVDesc, &texSRV);
                if (SUCCEEDED(hr))
                {
                    id_ = texSRV;
                    return true;
                }
            }

            Cleanup();
            return false;
        }

        virtual void UpdateRgba(const void* p) override
        {
            D3D11_MAPPED_SUBRESOURCE mapped_resource;
            ZeroMemory(&mapped_resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
            HRESULT hr =
                g_pd3dDeviceContext->Map(tex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);

            if (SUCCEEDED(hr))
            {
                const uint8_t* p_u8 = reinterpret_cast<const uint8_t*>(p);
                uint8_t* buf_u8     = reinterpret_cast<uint8_t*>(mapped_resource.pData);
                unsigned p_pitch    = 4 * width_;
                unsigned buf_pitch  = mapped_resource.RowPitch;
                for (int i = 0; i < height_; ++i)
                {
                    memcpy(buf_u8, p_u8, 4 * width_);
                    p_u8 += p_pitch;
                    buf_u8 += buf_pitch;
                }

                g_pd3dDeviceContext->Unmap(tex, 0);
            }
        }

        // virtual void Update(std::function<void(void* p, int row_pitch)> f) override
        // {
        //     D3D11_MAPPED_SUBRESOURCE mapped_resource;
        //     ZeroMemory(&mapped_resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
        //     HRESULT hr = g_pd3dDeviceContext->Map(tex, 0, D3D11_MAP_WRITE_DISCARD, 0,
        //     &mapped_resource);

        //     if (SUCCEEDED(hr))
        //     {
        //         f(mapped_resource.pData, mapped_resource.RowPitch);

        //         // TODO: exception safety
        //         g_pd3dDeviceContext->Unmap(tex, 0);
        //     }
        // }

        void Cleanup()
        {
            if (texSRV != nullptr)
            {
                texSRV->Release();
                texSRV = nullptr;
            }
            if (tex != nullptr)
            {
                tex->Release();
                tex = nullptr;
            }

            Clear();
        }
    };

    static std::unique_ptr<PlatformWindow_Win32> CurrentWindow = nullptr;

    // Main Code
    int DoMain_Dx11_Win32(Application& app, const AppWindowConfig& window_config)
    {
        // Create application window
        WNDCLASSEX wc = {sizeof(WNDCLASSEX),         CS_CLASSDC, WndProc, 0L,   0L,
                         GetModuleHandle(NULL),      NULL,       NULL,    NULL, NULL,
                         window_config.name.c_str(), NULL};
        ::RegisterClassEx(&wc);
        HWND hwnd =
            ::CreateWindow(wc.lpszClassName, window_config.title.c_str(), WS_OVERLAPPEDWINDOW,
                           window_config.pos_x, window_config.pos_y, window_config.width,
                           window_config.height, NULL, NULL, wc.hInstance, NULL);

        // Initialize Direct3D
        if (!CreateDeviceD3D(hwnd))
        {
            CleanupDeviceD3D();
            ::UnregisterClass(wc.lpszClassName, wc.hInstance);
            return 1;
        }

        // Show the window
        ::ShowWindow(hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(hwnd);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load
        // multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select
        // the font among multiple.
        // - If the file cannot be loaded, the function will return NULL. Please handle those errors
        // in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a
        // texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
        // ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'misc/fonts/README.txt' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you
        // need to write a double backslash \\ ! io.Fonts->AddFontDefault();
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
        // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

        // Main loop
        CurrentWindow = std::make_unique<PlatformWindow_Win32>(hwnd);
        app.Initialize();

        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while (msg.message != WM_QUIT)
        {
            // Poll and handle messages (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear
            // imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main
            // application.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your
            // main application. Generally you may always pass all inputs to dear imgui, and hide
            // them from your application based on those two flags.
            if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                continue;
            }

            // Start the Dear ImGui frame
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // Update application state
            app.Update();

            // Rendering
            ImVec4 clear_color = app.RenderingConfig().bg_color;

            ImGui::Render();
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
            g_pd3dDeviceContext->ClearRenderTargetView(
                g_mainRenderTargetView, reinterpret_cast<const float*>(&clear_color));
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            g_pSwapChain->Present(1, 0); // Present with vsync

            // g_pSwapChain->Present(0, 0); // Present without vsync
        }

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        CurrentWindow = nullptr;

        return 0;
    }

    // Helper functions

    bool CreateDeviceD3D(HWND hWnd)
    {
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount                        = 2;
        sd.BufferDesc.Width                   = 0;
        sd.BufferDesc.Height                  = 0;
        sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator   = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow                       = hWnd;
        sd.SampleDesc.Count                   = 1;
        sd.SampleDesc.Quality                 = 0;
        sd.Windowed                           = TRUE;
        sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_0,
        };
        if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
                                          featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
                                          &g_pSwapChain, &g_pd3dDevice, &featureLevel,
                                          &g_pd3dDeviceContext) != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }

    void CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (g_pSwapChain)
        {
            g_pSwapChain->Release();
            g_pSwapChain = NULL;
        }
        if (g_pd3dDeviceContext)
        {
            g_pd3dDeviceContext->Release();
            g_pd3dDeviceContext = NULL;
        }
        if (g_pd3dDevice)
        {
            g_pd3dDevice->Release();
            g_pd3dDevice = NULL;
        }
    }

    void CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }

    void CleanupRenderTarget()
    {
        if (g_mainRenderTargetView)
        {
            g_mainRenderTargetView->Release();
            g_mainRenderTargetView = NULL;
        }
    }

    // Win32 message handler
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
            {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam),
                                            DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
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
} // namespace

PlatformWindow& GetCurrentWindow()
{
    return *CurrentWindow;
}

std::unique_ptr<PlatformTexture> AllocateTexture(int width, int height)
{
    auto result = std::make_unique<PlatformTexture_Dx11>();
    if (!result->Initialize(width, height))
    {
        return nullptr;
    }

    return result;
}

int RunApplication(Application& app, AppWindowConfig window_config)
{
    return DoMain_Dx11_Win32(app, window_config);
}