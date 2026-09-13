// ==WindhawkMod==
// @id              rainbow-titlebar
// @name            Rainbow Titlebar
// @description     Animated RGB rainbow title bars
// @version         1.0.1
// @author          Wxx
// @include         *
// @exclude         devenv.exe
// @exclude         systemsettings.exe
// @exclude         applicationframehost.exe
// @exclude         startmenuexperiencehost.exe
// @exclude         searchhost.exe
// @exclude         shellexperiencehost.exe
// @compilerOptions -ldwmapi
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*...*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*...*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <dwmapi.h>
#include <windhawk_utils.h>

#include <atomic>

#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif

// ============================================================
// GLOBALS
// ============================================================

static std::atomic<bool> g_running(false);
static HANDLE g_thread = nullptr;


// ============================================================
// RAINBOW
// ============================================================

static COLORREF GetRainbowColor()
{
    DWORD time = GetTickCount();

    // Vitesse du Rainbow
    int p = (int)((time / 2) % 1536);

    BYTE r = 0;
    BYTE g = 0;
    BYTE b = 0;

    if (p < 256)
    {
        // Rouge -> Jaune
        r = 255;
        g = (BYTE)p;
        b = 0;
    }
    else if (p < 512)
    {
        // Jaune -> Vert
        r = (BYTE)(511 - p);
        g = 255;
        b = 0;
    }
    else if (p < 768)
    {
        // Vert -> Cyan
        r = 0;
        g = 255;
        b = (BYTE)(p - 512);
    }
    else if (p < 1024)
    {
        // Cyan -> Bleu
        r = 0;
        g = (BYTE)(1023 - p);
        b = 255;
    }
    else if (p < 1280)
    {
        // Bleu -> Violet
        r = (BYTE)(p - 1024);
        g = 0;
        b = 255;
    }
    else
    {
        // Violet -> Rouge
        r = 255;
        g = 0;
        b = (BYTE)(1535 - p);
    }

    return RGB(r, g, b);
}


// ============================================================
// FENÊTRE COMPATIBLE
// ============================================================

static bool IsSupportedWindow(HWND hwnd)
{
    if (!IsWindow(hwnd))
        return false;

    if (!IsWindowVisible(hwnd))
        return false;

    LONG style = GetWindowLongW(
        hwnd,
        GWL_STYLE
    );

    if (!(style & WS_CAPTION))
        return false;

    if (style & WS_CHILD)
        return false;

    LONG exStyle = GetWindowLongW(
        hwnd,
        GWL_EXSTYLE
    );

    if (exStyle & WS_EX_TOOLWINDOW)
        return false;

    WCHAR className[256] = {};

    if (GetClassNameW(
            hwnd,
            className,
            256))
    {
        if (wcscmp(
                className,
                L"ApplicationFrameWindow") == 0)
            return false;

        if (wcscmp(
                className,
                L"Windows.UI.Core.CoreWindow") == 0)
            return false;

        if (wcscmp(
                className,
                L"DesktopWindowXamlSource") == 0)
            return false;

        if (wcscmp(
                className,
                L"Microsoft.UI.Content.DesktopChildSiteBridge") == 0)
            return false;
    }

    return true;
}


// ============================================================
// APPLIQUER LA COULEUR
// ============================================================

static void ApplyRainbow(HWND hwnd)
{
    if (!IsSupportedWindow(hwnd))
        return;

    COLORREF color =
        GetRainbowColor();

    DwmSetWindowAttribute(
        hwnd,
        DWMWA_CAPTION_COLOR,
        &color,
        sizeof(color)
    );
}


// ============================================================
// ENUMERATION DES FENÊTRES
// ============================================================

static BOOL CALLBACK EnumRainbowWindows(
    HWND hwnd,
    LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    DWORD pid = 0;

    GetWindowThreadProcessId(
        hwnd,
        &pid
    );

    if (pid != GetCurrentProcessId())
        return TRUE;

    ApplyRainbow(hwnd);

    return TRUE;
}


static void UpdateAllWindows()
{
    EnumWindows(
        EnumRainbowWindows,
        0
    );
}


// ============================================================
// RESTAURATION DES COULEURS
// ============================================================

static BOOL CALLBACK RestoreWindows(
    HWND hwnd,
    LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    DWORD pid = 0;

    GetWindowThreadProcessId(
        hwnd,
        &pid
    );

    if (pid != GetCurrentProcessId())
        return TRUE;

    if (!IsWindow(hwnd))
        return TRUE;

    COLORREF defaultColor =
        DWMWA_COLOR_DEFAULT;

    DwmSetWindowAttribute(
        hwnd,
        DWMWA_CAPTION_COLOR,
        &defaultColor,
        sizeof(defaultColor)
    );

    return TRUE;
}


// ============================================================
// THREAD RAINBOW
// ============================================================

static DWORD WINAPI RainbowThread(
    LPVOID lpParameter)
{
    UNREFERENCED_PARAMETER(lpParameter);

    while (g_running)
    {
        UpdateAllWindows();

        // Animation fluide
        Sleep(10);
    }

    return 0;
}


// ============================================================
// INITIALISATION
// ============================================================

BOOL Wh_ModInit()
{
    Wh_Log(
        L"Rainbow Titlebar: starting"
    );

    BOOL enabled =
        Wh_GetIntSetting(
            L"rainbowEnabled"
        );

    if (!enabled)
    {
        Wh_Log(
            L"Rainbow is disabled"
        );

        return TRUE;
    }

    g_running = true;

    g_thread = CreateThread(
        nullptr,
        0,
        RainbowThread,
        nullptr,
        0,
        nullptr
    );

    if (!g_thread)
    {
        g_running = false;

        Wh_Log(
            L"Failed to create Rainbow thread"
        );

        return FALSE;
    }

    Wh_Log(
        L"Rainbow Titlebar started"
    );

    return TRUE;
}


// ============================================================
// APRÈS INITIALISATION
// ============================================================

void Wh_ModAfterInit()
{
    UpdateAllWindows();
}


// ============================================================
// PARAMÈTRES MODIFIÉS
// ============================================================

void Wh_ModSettingsChanged()
{
    BOOL enabled =
        Wh_GetIntSetting(
            L"rainbowEnabled"
        );

    if (enabled)
    {
        if (!g_running)
        {
            g_running = true;

            if (!g_thread)
            {
                g_thread = CreateThread(
                    nullptr,
                    0,
                    RainbowThread,
                    nullptr,
                    0,
                    nullptr
                );
            }
        }

        UpdateAllWindows();
    }
    else
    {
        g_running = false;
    }
}


// ============================================================
// ARRÊT
// ============================================================

void Wh_ModUninit()
{
    Wh_Log(
        L"Rainbow Titlebar: stopping"
    );

    g_running = false;

    if (g_thread)
    {
        WaitForSingleObject(
            g_thread,
            2000
        );

        CloseHandle(
            g_thread
        );

        g_thread = nullptr;
    }

    // Restaurer les couleurs Windows
    EnumWindows(
        RestoreWindows,
        0
    );

    Wh_Log(
        L"Rainbow Titlebar: stopped"
    );
}