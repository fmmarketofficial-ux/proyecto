#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "Fonts.h"

#include "image.h"
#include <d3d11.h>
#include <D3DX11.h>
#pragma comment (lib, "d3dx11.lib")

#include "imgui_settings.h"

static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hwnd;
RECT rc;

float WIDTH = 800; // Loader Size X
float HEIGHT = 550; // Loader Size Y

ID3D11ShaderResourceView* bg = nullptr;

ID3D11ShaderResourceView* google_icon = nullptr;
ID3D11ShaderResourceView* twitter_icon = nullptr;

ID3D11ShaderResourceView* logo = nullptr;

ID3D11ShaderResourceView* gp = nullptr;

ID3D11ShaderResourceView* hd = nullptr;

static bool images_loaded = false;

ID3D11ShaderResourceView* Avatar = nullptr;

ID3D11ShaderResourceView* back_images[4];
ID3D11ShaderResourceView* product_images[4];

namespace notifications
{
    const char* notif_name;
    const char* notif_icon;
    static int notif_state;
    static float notif_rotate;

    static float notif_offset;

    static float notif_timer;

    static float notif_width;

    void Message(const char* name, const char* icon)
    {
        notif_name = name;
        notif_icon = icon;
        notif_state = 0;
        notif_rotate = 0;
        notif_offset = 150;
        notif_timer = 0;
    }

    void NotifyUpdate(ImVec2 p)
    {
        if (CalcTextSize(notif_name).x < 1.f)
            return;

        notif_offset = ImLerp(notif_offset, notif_state == 0 ? 0.f : 250.f, anim_speed);
        notif_rotate = ImLerp(notif_rotate, notif_state == 0 ? IM_PI / 2 : notif_state == 1 ? IM_PI / 2 - 0.6f : IM_PI / 2 + 0.6f, anim_speed * 2);

        notif_timer += anim_speed;

        if (notif_timer > 20.f)
            notif_state = 1;

        ImGui::PushFont(big_icon);
        float icon_width = CalcTextSize(notif_icon).x;
        ImGui::PopFont();

        notif_width = ImLerp(notif_width, CalcTextSize(notif_name).x / 2 + icon_width / 2 + 15.f, anim_speed * 3);

        ImRect notif_bb(p + ImVec2(WIDTH / 2 - notif_width, HEIGHT - 95 + notif_offset), ImVec2(WIDTH / 2 + notif_width, HEIGHT - 35 + notif_offset));

        ImRotateStart();
        rect_glow(ImGui::GetWindowDrawList(), notif_bb.Min, notif_bb.Max, ImColor(main_color.Value.x, main_color.Value.y, main_color.Value.z, 0.15f), ImGui::GetStyle().FrameRounding, 2.f);
        ImGui::GetWindowDrawList()->AddRectFilled(notif_bb.Min, notif_bb.Max, background_color, ImGui::GetStyle().FrameRounding);
        

        ImGui::PushFont(big_icon);
        if (notif_icon == "w")
        {
            ImGui::GetWindowDrawList()->AddShadowCircle(notif_bb.Min + ImVec2(30, 30), 10.f, ImColor(201, 80, 80, 200), 45.f, ImVec2(0,0));
            ImGui::GetWindowDrawList()->AddText(notif_bb.Min + ImVec2(30, 30) - CalcTextSize(notif_icon) / 2, ImColor(201, 80, 80, 255), notif_icon);
        }

        if (notif_icon == "s")
        {
            ImGui::GetWindowDrawList()->AddShadowCircle(notif_bb.Min + ImVec2(30, 30), 10.f, ImColor(82, 201, 80, 200), 45.f, ImVec2(0, 0));
            ImGui::GetWindowDrawList()->AddText(notif_bb.Min + ImVec2(30, 30) - CalcTextSize(notif_icon) / 2, ImColor(82, 201, 80, 255), notif_icon);
        }
        ImGui::PopFont();

        ImGui::GetWindowDrawList()->AddText(ImVec2(notif_bb.Min.x + 55, center_text(notif_bb.Min, notif_bb.Max, notif_name).y), text_color[0], notif_name);

        ImRotateEnd(notif_rotate);
    }
}

void Trinage_background()
{

    ImVec2 screen_size = { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };

    static ImVec2 partile_pos[100];
    static ImVec2 partile_target_pos[100];
    static float partile_speed[100];
    static float partile_size[100];
    static float partile_radius[100];
    static float partile_rotate[100];

    for (int i = 1; i < 100; i++)
    {
        if (partile_pos[i].x == 0 || partile_pos[i].y == 0)
        {
            partile_pos[i].x = rand() % (int)screen_size.x + 1;
            partile_pos[i].y = 15.f;
            partile_speed[i] = 1 + rand() % 25;
            partile_radius[i] = rand() % 4;
            partile_size[i] = rand() % 8;

            partile_target_pos[i].x = rand() % (int)screen_size.x;
            partile_target_pos[i].y = screen_size.y * 2;
        }

        partile_pos[i] = ImLerp(partile_pos[i], partile_target_pos[i], ImGui::GetIO().DeltaTime * (partile_speed[i] / 60));
        partile_rotate[i] += ImGui::GetIO().DeltaTime;

        if (partile_pos[i].y > screen_size.y)
        {
            partile_pos[i].x = 0;
            partile_pos[i].y = 0;
            partile_rotate[i] = 0;
        }

        ImRotateStart();
        ImGui::GetWindowDrawList()->AddShadowCircle(partile_pos[i], 1.f, main_color, partile_size[i] * 2, ImVec2(0,0), 0, 1);
        ImGui::GetWindowDrawList()->AddCircleFilled(partile_pos[i], partile_size[i], main_color, 1);
        ImRotateEnd(partile_rotate[i]);
    }
}

void LoadImages()
{
    if (images_loaded)
        return;

    D3DX11_IMAGE_LOAD_INFO iInfo;
    ID3DX11ThreadPump* threadPump{ nullptr };
    // From Bytes
    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, avatar_bytes, sizeof(avatar_bytes), &iInfo, threadPump, &Avatar/*shader*/, 0);

    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, dotaimage, sizeof(dotaimage), &iInfo, threadPump, &back_images[0]/*shader*/, 0);
    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, csimage, sizeof(csimage), &iInfo, threadPump, &back_images[1]/*shader*/, 0);
    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, terrariaimage, sizeof(terrariaimage), &iInfo, threadPump, &back_images[2]/*shader*/, 0);
    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, tarkovimage, sizeof(tarkovimage), &iInfo, threadPump, &back_images[3]/*shader*/, 0);


    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, productdota, sizeof(productdota), &iInfo, threadPump, &product_images[0]/*shader*/, 0);
    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, productcs, sizeof(productcs), &iInfo, threadPump, &product_images[1]/*shader*/, 0);
    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, productterraria, sizeof(productterraria), &iInfo, threadPump, &product_images[2]/*shader*/, 0);
    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, producttarkov, sizeof(producttarkov), &iInfo, threadPump, &product_images[3]/*shader*/, 0);

    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logotype, sizeof(logotype), &iInfo, threadPump, &logo/*shader*/, 0);

    images_loaded = true;
}

void move_window() {

    GetWindowRect(hwnd, &rc);
    MoveWindow(hwnd, rc.left + ImGui::GetWindowPos().x, rc.top + ImGui::GetWindowPos().y, WIDTH, HEIGHT, TRUE);
    ImGui::SetWindowPos(ImVec2(0.f, 0.f));
}

void RenderBlur(HWND hwnd)
{
    struct ACCENTPOLICY
    {
        int na;
        int nf;
        int nc;
        int nA;
    };
    struct WINCOMPATTRDATA
    {
        int na;
        PVOID pd;
        ULONG ul;
    };

    const HINSTANCE hm = LoadLibrary(L"user32.dll");
    if (hm)
    {
        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hm, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute)
        {
            ACCENTPOLICY policy = { 3, 0, 0, 0 };

            WINCOMPATTRDATA data = { 19, &policy,sizeof(ACCENTPOLICY) };
            SetWindowCompositionAttribute(hwnd, &data);
        }
        FreeLibrary(hm);
    }
}
