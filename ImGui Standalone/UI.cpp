#include "UI.h"

#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "font.h"

ID3D11Device* UI::pd3dDevice = nullptr;
ID3D11DeviceContext* UI::pd3dDeviceContext = nullptr;
IDXGISwapChain* UI::pSwapChain = nullptr;
ID3D11RenderTargetView* UI::pMainRenderTargetView = nullptr;

LPCSTR lpWindowName = "Lime Solutions";
ImVec2 vWindowSize = { 800, 500 };
ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize /* | ImGuiWindowFlags_NoTitleBar*/;
bool loader_active = true;

bool rememberme = false;
bool animate = false;

char username[50] = "";
char password[50] = "";

const char* items[]{ " Fivem" };
int selecteditem = 0;

const char* items1[]{ " FiveM"," Spinayy", " yessir", " easy", " Rainbow 6", " Call Of Duty"};
int selecteditem1 = 0;

bool UI::CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const UINT createDeviceFlags = 0;
    
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void UI::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer != nullptr)
    {
        pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pMainRenderTargetView);
        pBackBuffer->Release();
    }
}

void UI::CleanupRenderTarget()
{
    if (pMainRenderTargetView)
    {
        pMainRenderTargetView->Release();
        pMainRenderTargetView = nullptr;
    }
}

void UI::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (pSwapChain)
    {
        pSwapChain->Release();
        pSwapChain = nullptr;
    }

    if (pd3dDeviceContext)
    {
        pd3dDeviceContext->Release();
        pd3dDeviceContext = nullptr;
    }

    if (pd3dDevice)
    {
        pd3dDevice->Release();
        pd3dDevice = nullptr;
    }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

LRESULT WINAPI UI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;

    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;

    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;

    default:
        break;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

void UI::Render()
{
    ImGui_ImplWin32_EnableDpiAwareness();
    const WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("ImGui Standalone"), nullptr };
    ::RegisterClassEx(&wc);
    const HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("ImGui Standalone"), WS_OVERLAPPEDWINDOW, 100, 100, 50, 50, NULL, NULL, wc.hInstance, NULL);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return;
    }

    ::ShowWindow(hwnd, SW_HIDE);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 4.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }



    const HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO info = {};
    info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &info);
    const int monitor_height = info.rcMonitor.bottom - info.rcMonitor.top;

    if (monitor_height > 1080)
    {
        const float fScale = 2.0f;
        ImFontConfig cfg;
        cfg.SizePixels = 13 * fScale;
        ImGui::GetIO().Fonts->AddFontDefault(&cfg);
    }

    ImGui::GetIO().Fonts->AddFontFromMemoryTTF(FontData, sizeof FontData, 17);

    ImFont* big;
    big = io.Fonts->AddFontFromMemoryTTF(FontData, sizeof FontData, 47);

    //bigfont = io.Fonts->AddFontFromMemoryTTF(FontData, sizeof FontData, 47);

    ImGui::GetIO().IniFilename = nullptr;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(pd3dDevice, pd3dDeviceContext);

    const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    bool bDone = false;

    int tabs = 0;
    int innertabs = 0;

    while (!bDone)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                bDone = true;
        }
        if (bDone)
            break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::SetNextWindowSize(vWindowSize);
            ImGui::SetNextWindowBgAlpha(1.0f);
            ImGui::Begin(lpWindowName, &loader_active, WindowFlags);
            {
                /*if (ImGui::Button("Popup Button", { 120, 0 }))
                {
                    ImGui::OpenPopup("test");
                }

                if (ImGui::BeginPopupModal("test", NULL, ImGuiWindowFlags_Popup))
                {
                    if (ImGui::Button("Close Popup", { 120, 0 }))
                    {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }*/
                //ImGui::Checkbox("Test", &rememberme);

                if (tabs == 0)
                {
                    //logo here
                    ImGui::SetCursorPos({ 338, 140 });
                    ImGui::PushFont(big);
                    ImGui::Text("Lime");
                    ImGui::PopFont();

                    ImGui::SetCursorPos({ 260, 230 });
                    ImGui::SetNextItemWidth(250);
                    ImGui::InputText("Username", username, IM_ARRAYSIZE(username));

                    ImGui::SetCursorPos({ 260, 260 });
                    ImGui::SetNextItemWidth(250);
                    ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

                    ImGui::SetCursorPos({ 260, 291 });
                    if (ImGui::Button("Login", { 121, 30 }))
                    {
                        animate = true;
                    }
                    ImGui::SameLine();
                    ImGui::Button("Register", { 121, 30 });

                    static float progress = 0.0f;
                    static float progress_dir = 1.0f;
                    if (animate)
                    {
                        progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
                    }
                    if (animate)
                    {
                        ImGui::SetCursorPos({ 260, 327 });
                        ImGui::SetNextItemWidth(250);
                        ImGui::ProgressBar(progress, ImVec2(0.0f, 30.0f));
                    }
                    if (progress >= 1.0f)
                    {
                        tabs = 1;
                    }
                }
                if (tabs == 1)
                {
                    if (ImGui::BeginTabBar("#tabs"))
                    {

                        if(ImGui::BeginTabItem("  Spoofer | Cleaner  "))
                        {
                            innertabs = 1;
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("  Loader  "))
                        {
                            innertabs = 0;
                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }

                    if (innertabs == 0)
                    {
                        ImGui::SetCursorPos({ 205, 201 });
                        ImGui::BeginChild("##1", ImVec2(ImGui::GetContentRegionAvail().x / 4, ImGui::GetContentRegionAvail().y / 1.8), true, ImGuiWindowFlags_NoResize);

                        ImGui::Text("Cheat Selection");

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        ImGui::Spacing();

                        ImGui::SetNextItemWidth(130);
                        ImGui::ListBox("##Game", &selecteditem, items, IM_ARRAYSIZE(items));

                        ImGui::EndChild();


                        ImGui::SameLine();


                        ImGui::BeginChild("##2", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y / 1.8), true, ImGuiWindowFlags_NoResize);

                        ImGui::Text("Account Information");

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        ImGui::Spacing();

                        ImGui::Text("Username");
                        ImGui::TextDisabled("vavy");

                        ImGui::Spacing();

                        ImGui::Text("Days Left");
                        ImGui::TextDisabled("29 days");

                        ImGui::EndChild();

                        ImGui::SetCursorPos({ 205, 370 });
                        ImGui::Button("Inject Into Game", { 370, 30 });
                    }
                    else if (innertabs == 1)
                    {
                        ImGui::SetCursorPos({ 205, 151 });
                        ImGui::BeginChild("##3", ImVec2(ImGui::GetContentRegionAvail().x / 4, ImGui::GetContentRegionAvail().y / 1.5), true, ImGuiWindowFlags_NoResize);

                        ImGui::Text("Game Selection");

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        ImGui::Spacing();

                        ImGui::SetNextItemWidth(130);
                        ImGui::ListBox("##GamesToSpoof", &selecteditem1, items1, IM_ARRAYSIZE(items1));

                        ImGui::EndChild();


                        ImGui::SameLine();


                        ImGui::BeginChild("##4", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y / 1.5), true, ImGuiWindowFlags_NoResize);

                        ImGui::Text("Game Information");

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        ImGui::Spacing();

                        if (selecteditem1 == 0) //FiveM
                        {
                            ImGui::Text("Status");
                            ImGui::TextDisabled("Detected / Not Working");
                        }
                        else if (selecteditem1 == 1) //Apex
                        {
                            ImGui::Text("Status");
                            ImGui::TextDisabled("Undectected / Working");
                        }
                        else if (selecteditem1 == 2) //Valorant
                        {
                            ImGui::Text("Status");
                            ImGui::TextDisabled("Undectected / Working");
                        }
                        else if (selecteditem1 == 3) //Fortnite
                        {
                            ImGui::Text("Status");
                            ImGui::TextDisabled("Undectected / Working");
                        }
                        else if (selecteditem1 == 4) //Rainbow Six Siege
                        {
                            ImGui::Text("Status");
                            ImGui::TextDisabled("Undectected / Working");
                        }
                        else if (selecteditem1 == 5) //Call Of Duty
                        {
                            ImGui::Text("Status");
                            ImGui::TextDisabled("Undectected / Working");
                        }

                        /*ImGui::Spacing();

                        ImGui::Text("Days Left");
                        ImGui::TextDisabled("29 days");*/

                        ImGui::EndChild();

                        ImGui::SetCursorPos({ 205, 385 });
                        if (selecteditem1 == 0)
                        {

                        }
                        else {
                            if (ImGui::Button("Clean", { 147, 30 }))
                            {
                                if (selecteditem1 == 1) //Apex
                                {
                                    
                                }
                                else if (selecteditem1 == 2) //Valorant
                                {
                                    
                                }
                                else if (selecteditem1 == 3) //Fortnite
                                {
                                    
                                }
                                else if (selecteditem1 == 4) //Rainbow Six Siege
                                {
                                    
                                }
                                else if (selecteditem1 == 5) //Call Of Duty
                                {
                                    
                                }
                            }
                        }
                        

                        ImGui::SetCursorPos({ 359, 385 });
                        if (ImGui::Button("Spoof", { 217, 30 }))
                        {
                            if (selecteditem1 == 0) //FiveM
                            {
                                
                            }
                            else if (selecteditem1 == 1) //Apex
                            {
                                
                            }
                            else if (selecteditem1 == 2) //Valorant
                            {
                                
                            }
                            else if (selecteditem1 == 3) //Fornite
                            {
                                
                            }
                            else if (selecteditem1 == 4) //Rainbow Six Siege
                            {
                                
                            }
                            else if (selecteditem1 == 5) //Call Of Duty
                            {
                                

                            }
                        }
                    }
                }
            }
            ImGui::End();
        }
        ImGui::EndFrame();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        pd3dDeviceContext->OMSetRenderTargets(1, &pMainRenderTargetView, nullptr);
        pd3dDeviceContext->ClearRenderTargetView(pMainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (!loader_active) {
           /* msg.message = WM_QUIT;

            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();

            CleanupDeviceD3D();
            ::DestroyWindow(hwnd);
            ::UnregisterClass(wc.lpszClassName, wc.hInstance);*/

            exit(0);
        }

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    #ifdef _WINDLL
    ExitThread(0);
    #endif
}