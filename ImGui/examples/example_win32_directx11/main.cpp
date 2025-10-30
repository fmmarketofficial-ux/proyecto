#include "main.h"

#include <tchar.h>
#include <dwmapi.h>
#include <string>
#include <Windows.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"


char login[128] = "Lyapos";
char email[128];
char password[128];


static bool checkbox = false;

static int iProduct = 0;

static int current_image = 0;
static float images_alpha[4];

int main(int, char**)
{
    WNDCLASSEXW wc;
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = nullptr;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = L"ImGui";
    wc.lpszClassName = L"Example";
    wc.hIconSm = LoadIcon(0, IDI_APPLICATION);


    RegisterClassExW(&wc);
    hwnd = CreateWindowExW(NULL, wc.lpszClassName, L"Loader", WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) / 2) - (WIDTH / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (HEIGHT / 2), WIDTH, HEIGHT, 0, 0, 0, 0);

    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    POINT mouse;
    rc = { 0 };
    GetWindowRect(hwnd, &rc);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    SetWindowRgn(hwnd, CreateRoundRectRgn(0, 0, WIDTH, HEIGHT, 51, 51), FALSE);

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::ShowWindow(GetConsoleWindow(), SW_HIDE);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.Fonts->AddFontFromMemoryTTF(&InterMedium, sizeof InterMedium, 19, NULL, io.Fonts->GetGlyphRangesCyrillic());

    bold_font = io.Fonts->AddFontFromMemoryTTF(&InterBold, sizeof InterBold, 21, NULL, io.Fonts->GetGlyphRangesCyrillic());

    bold_big_font = io.Fonts->AddFontFromMemoryTTF(&InterBold, sizeof InterBold, 23, NULL, io.Fonts->GetGlyphRangesCyrillic());

    big_font = io.Fonts->AddFontFromMemoryTTF(&InterMedium, sizeof InterMedium, 27, NULL, io.Fonts->GetGlyphRangesCyrillic());

    big_icon = io.Fonts->AddFontFromMemoryTTF(&icomoon, sizeof icomoon, 35, NULL, io.Fonts->GetGlyphRangesCyrillic());

    default_font = io.Fonts->AddFontFromMemoryTTF(&InterMedium, sizeof InterMedium, 19, NULL, io.Fonts->GetGlyphRangesCyrillic());

    icon_font = io.Fonts->AddFontFromMemoryTTF(&icomoon, sizeof icomoon, 22, NULL, io.Fonts->GetGlyphRangesCyrillic());

    medium_icon_font = io.Fonts->AddFontFromMemoryTTF(&icomoon, sizeof icomoon, 32, NULL, io.Fonts->GetGlyphRangesCyrillic());

    icon_small = io.Fonts->AddFontFromMemoryTTF(&icomoon, sizeof icomoon, 18, NULL, io.Fonts->GetGlyphRangesCyrillic());

    dot_font = io.Fonts->AddFontFromMemoryTTF(&icomoon, sizeof icomoon, 7, NULL, io.Fonts->GetGlyphRangesCyrillic());

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 0.f);

    ImGuiStyle& s = ImGui::GetStyle();

    s.FramePadding = ImVec2(5, 5);
    s.ItemSpacing = ImVec2(20, 20);
    s.WindowPadding = ImVec2(0, 0);
    s.FrameRounding = 10.f;
    s.WindowRounding = 30.f;
    s.WindowBorderSize = 0.f;
    s.PopupBorderSize = 0.f;
    s.WindowPadding = ImVec2(0, 0);
    s.ChildBorderSize = 10;

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            LoadImages();
            ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT));
            ImGui::Begin("General", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
            {
                auto draw = ImGui::GetWindowDrawList();
                const auto& p = ImGui::GetWindowPos();

                move_window();

                anim_speed = ImGui::GetIO().DeltaTime * 8.f;

                static int menu_state = 0;

                static int state_autorization = 0;

                draw->AddRectFilled(p, p + ImVec2(WIDTH, HEIGHT), winbg, 32);

                static float rotation;
                rotation = ImLerp(rotation, rotation + 0.02f, anim_speed);

                Trinage_background();

                static float logotype_offset;
                static float product_page_offset = -500.f;

                logotype_offset = ImLerp(logotype_offset, state_autorization == 3 ? 476 : 0.f, anim_speed);
                product_page_offset = ImLerp(product_page_offset, menu_state == 2 ? 0.f : -500.f, anim_speed);

                if (logotype_offset > 475.f)
                    menu_state = 2;

                const char* product_icons[4] = { "d", "g", "t", "e" };
                const char* products_text[4] = { "Dota 2", " Counter: Strike\nGlobal Offensive", "Terraria", "Espace From Tarkov" };

                if (menu_state == 0) {
                    static bool login_page = true;

                    static float login_page_offset = 0.f;
                    static float reg_page_offset = 0.f;
                    static float images_offset = 0.f;

                    reg_page_offset = ImLerp(reg_page_offset, login_page ? 500.f : 0.f, anim_speed);
                    login_page_offset = ImLerp(login_page_offset, !login_page ? 500.f : state_autorization == 3 ? 450.f : 0.f, anim_speed);
                    images_offset = ImLerp(images_offset, state_autorization == 3 ? 450.f : 0.f, anim_speed);

                    if (state_autorization == 2)
                    {
                        static DWORD dwTickStart = GetTickCount();
                        if (GetTickCount() - dwTickStart > 5000)
                        {
                            inputtext_color[0] = background_color;
                            inputtext_color[1] = main_color;
                            inputtext_color[2] = text_color[0];
                            state_autorization = 0;
                            dwTickStart = GetTickCount();
                        }
                    }

                    static float icons_alpha[4];
                    for (int i = 0; i < 4; i++) {
                        images_alpha[i] = ImLerp(images_alpha[i], i == current_image ? 1.f : 0.f, anim_speed);

                        draw->AddImageRounded(back_images[i], p - ImVec2(12, 0) - ImVec2(images_offset, 0), p + ImVec2(325, 550) - ImVec2(images_offset, 0), ImVec2(0, 0), ImVec2(1, 1), ImColor(1.f, 1.f, 1.f, images_alpha[i]), -100.f);
                    }

                    draw->AddRectFilledMultiColor(p - ImVec2(15 - images_offset, 0), p + ImVec2(325 - images_offset, 550), ImColor(winbg.Value.x, winbg.Value.y, winbg.Value.z, 0.f), winbg, winbg, ImColor(winbg.Value.x, winbg.Value.y, winbg.Value.z, 0.f));


                    for (int i = 0; i < 4; i++)
                    {
                        icons_alpha[i] = ImLerp(icons_alpha[i], i == current_image ? 1.f : 0.f, anim_speed);
                        PushFont(big_icon);
                        draw->AddText(ImVec2(center_text(p, p + ImVec2(325, 550), product_icons[i]).x, p.y + 30) - ImVec2(images_offset, 0), ImColor(1.f, 1.f, 1.f, icons_alpha[i]), product_icons[i]);
                        PopFont();

                        PushFont(big_font);
                        draw->AddText(ImVec2(center_text(p, p + ImVec2(325, 550), products_text[i]).x, p.y + 75) - ImVec2(images_offset, 0), ImColor(1.f, 1.f, 1.f, icons_alpha[i]), products_text[i]);
                        PopFont();
                    }

                    if (login_page_offset < 400.f) {
                        ImGui::SetCursorPos(ImVec2(388 + login_page_offset, 135));
                        ImGui::BeginGroup();

                        ImGui::InputTextEx("Login", "c", login, 128, ImVec2(350, 50), 0);
                        ImGui::InputTextEx("Password", "b", password, 128, ImVec2(350, 50), ImGuiInputTextFlags_Password);

                        if (ImGui::Button("Sign in", ImVec2(350, 50), " 1"))
                        {
                            std::string login_n2 = login;

                            if (login_n2 != "Lyapos")
                            {
                                notifications::Message("Incorrect password or username", "w");
                                inputtext_color[0] = ImColor(35, 24, 30, 255);
                                inputtext_color[1] = ImColor(201, 80, 80, 255);
                                inputtext_color[2] = ImColor(201, 80, 80, 255);
                                state_autorization = 2;
                            }
                            else
                            {
                                notifications::Message("Succes autorization with login", "s");
                                state_autorization = 3;
                            }
                        }

                        IconButton("With Google", ImVec2(170, 50), 0, "a");
                        ImGui::SameLine();
                        IconButton("With Twitter", ImVec2(170, 50), 0, "9");

                        //ImGui::TextDisabled("No account?"); ImGui::SameLine(CalcTextSize("No account? ").x);
                        if (button_text(" No account? ", "SIGN UP"))
                        {
                            login_page = false;
                        }
                        ImGui::SetCursorPos(ImVec2(590 + login_page_offset, 456));
                        ImGui::SetCursorPos(ImVec2(590 + login_page_offset, 456));
                        ImGui::TextDisabled("Forgot password?");
                        ImGui::EndGroup();
                    }
                    if (reg_page_offset < 400.f)
                    {
                        ImGui::SetCursorPos(ImVec2(388 + reg_page_offset, 135));
                        ImGui::BeginGroup();

                        ImGui::InputTextEx("Login", "c", login, 128, ImVec2(350, 50), 0);
                        ImGui::InputTextEx("Email", "8", email, 128, ImVec2(350, 50), 0);
                        ImGui::InputTextEx("Password", "b", password, 128, ImVec2(350, 50), ImGuiInputTextFlags_Password);

                        if (ImGui::Button("Sign up", ImVec2(350, 50), " 1"))
                        {
                            std::string login_n2 = login;

                            if (login_n2 != "Lyapos")
                            {
                                inputtext_color[0] = ImColor(35, 24, 30, 255);
                                inputtext_color[1] = ImColor(201, 80, 80, 255);
                                inputtext_color[2] = ImColor(201, 80, 80, 255);
                                state_autorization = 2;
                            }
                        }

                        ImGui::SetCursorScreenPos(ImVec2(center_text(p + ImVec2(388 + reg_page_offset, 0), p + ImVec2(388 + 350 + reg_page_offset, 0), " Have account? LOG IN").x, ImGui::GetCursorPos().y));
                        if (button_text(" Have account? ", "LOG IN"))
                        {
                            login_page = true;
                        }
                        ImGui::EndGroup();
                    }

                    PushFont(icon_font);

                    if (current_image < 3) {
                        ImGui::SetCursorPos(ImVec2(282 - images_offset, 261));
                        ImGui::TextColored(text_color[1], "1");
                        if (ImGui::IsItemClicked())
                            current_image++;
                    }

                    if (current_image > 0) {
                        ImGui::SetCursorPos(ImVec2(20 - images_offset, 261));
                        ImGui::TextColored(text_color[1], "2");
                        if (ImGui::IsItemClicked())
                            current_image--;
                    }

                    PopFont();

                    draw->AddShadowCircle(p + ImVec2(WIDTH / 2, HEIGHT + 150), 150.f, main_color_shadow, 750.f, ImVec2(0, 0));
                }
                if (menu_state == 2)
                {

                    PushFont(bold_font);
                    draw->AddShadowRect(ImVec2(center_text(p, p + ImVec2(WIDTH, 0), login).x, p.y + 50.f - ImGui::GetScrollY()) + ImVec2(0, 5), ImVec2(center_text(p, p + ImVec2(WIDTH, 0), login).x, p.y + 50.f - ImGui::GetScrollY()) + CalcTextSize(login) - ImVec2(0, 3), ImColor(main_color.Value.x, main_color.Value.y, main_color.Value.z, 0.45f), 35.f, ImVec2(0,0));
                    draw->AddText(ImVec2(center_text(p, p + ImVec2(WIDTH, 0), login).x, p.y + 50.f - ImGui::GetScrollY()), main_color, login);
                    PopFont();

                    PushFont(bold_big_font);
                    draw->AddText(ImVec2(center_text(p, p + ImVec2(WIDTH, 0), "Available products").x, p.y + 68.f - ImGui::GetScrollY()), text_color[0], "Available products");
                    PopFont();

                    ImGui::SetCursorPos(ImVec2(50 + product_page_offset, 135));
                    ImGui::BeginGroup();

                    if(ImGui::Product(products_text[0], product_images[0], 0, "16.09.2023", "Today", "Yesterday"))
                        notifications::Message("Wrond HWID Reset on site", "w");

                    if(ImGui::Product(products_text[1], product_images[1], 1, "16.09.2023", "Today", "Yesterday"))
                        notifications::Message("Succes injected", "s");

                    if(ImGui::Product(products_text[2], product_images[2], 2, "16.09.2023", "Today", "Yesterday"))
                        notifications::Message("Unknown error", "w");

                    if(ImGui::Product(products_text[3], product_images[3], 3, "16.09.2023", "Today", "Yesterday"))
                        notifications::Message("Succes injected", "s");

                    ImGui::Dummy(ImVec2(700, 50));

                    ImGui::EndGroup();

                    draw->AddShadowCircle(p + ImVec2(WIDTH / 2, HEIGHT + 150), 150.f, main_color_shadow, 750.f, ImVec2(0, 0));

                    draw->AddRectFilledMultiColor(p + ImVec2(45, HEIGHT - 50) + ImVec2(product_page_offset, 0), p + ImVec2(WIDTH - 45, HEIGHT) - ImVec2(product_page_offset, 0), ImColor(winbg.Value.x, winbg.Value.y, winbg.Value.z, 0.f), ImColor(winbg.Value.x, winbg.Value.y, winbg.Value.z, 0.f), winbg, winbg);

                    draw->AddRectFilledMultiColor(p + ImVec2(45, 0) + ImVec2(product_page_offset, 0), p + ImVec2(WIDTH - 45, 45) - ImVec2(product_page_offset, 0), winbg, winbg, ImColor(winbg.Value.x, winbg.Value.y, winbg.Value.z, 0.f), ImColor(winbg.Value.x, winbg.Value.y, winbg.Value.z, 0.f));
                }

                draw->AddShadowCircle(p + ImVec2(563, 67) - ImVec2(logotype_offset, ImGui::GetScrollY()), 30.f, ImColor(main_color.Value.x, main_color.Value.y, main_color.Value.z, 0.5f), 75.f, ImVec2(0, 0));
                draw->AddImage(logo, p + ImVec2(531, 34) - ImVec2(logotype_offset, ImGui::GetScrollY()), ImVec2(563, 67) + ImVec2(35, 35) - ImVec2(logotype_offset, ImGui::GetScrollY()), ImVec2(0, 0), ImVec2(1, 1), ImColor(1.f, 1.f, 1.f, 1.f));

                ImRotateStart();
                draw->AddShadowCircle(p + ImVec2(WIDTH + 150, HEIGHT / 2), 200.f, main_color_shadow, 600.f, ImVec2(0, 0));
                draw->AddShadowCircle(p + ImVec2(WIDTH + 150, HEIGHT / 2 - 100.f), 200.f, main_color_shadow, 600.f, ImVec2(0, 0));

                draw->AddShadowCircle(p + ImVec2(WIDTH + 450, HEIGHT / 2), 200.f, main_color_shadow, 600.f, ImVec2(0, 0));
                draw->AddShadowCircle(p + ImVec2(WIDTH + 450, HEIGHT / 2 + 100.f), 200.f, main_color_shadow, 600.f, ImVec2(0, 0));
                ImRotateEnd(rotation);

                notifications::NotifyUpdate(p);
            }

            ImGui::End();
            //RenderBlur(hwnd);
        }

        ImGui::Render();

        const float clear_color_with_alpha[4] = { 0 };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
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

    UINT createDeviceFlags = 0;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
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
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
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
