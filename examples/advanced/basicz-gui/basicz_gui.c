// basicz_gui.c -- BASIC-Z GUI: a Win32 GUI rewrite inspired by CPU-Z.
// Built with BASICGOL macros on top of the Win32 API.
#include <basicgol.h>
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")

#define ID_TAB 1001

static HWND hwndTab;
static HWND hwndPanel;

static VAZIO preencher_info_cpu(char *buf, int sz) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    snprintf(buf, sz, "Logical processors: %lu\r\nPage size: %lu bytes",
             si.dwNumberOfProcessors, si.dwPageSize);
}

static VAZIO preencher_info_mem(char *buf, int sz) {
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    SE GlobalMemoryStatusEx(&ms) ENTAO
        snprintf(buf, sz, "Total physical: %llu MB\r\nAvailable: %llu MB",
                 ms.ullTotalPhys / (1024*1024), ms.ullAvailPhys / (1024*1024));
    SENAO
        snprintf(buf, sz, "Unable to query memory.");
    FIM_SE
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SE (msg == WM_CREATE) ENTAO
        INITCOMMONCONTROLSEX icc;
        icc.dwSize = sizeof(icc);
        icc.dwICC = ICC_TAB_CLASSES;
        InitCommonControlsEx(&icc);

        hwndTab = CreateWindowEx(0, WC_TABCONTROL, "",
            WS_CHILD | WS_VISIBLE | TCS_TABS,
            10, 10, 460, 30, hwnd, (HMENU)ID_TAB, NULL, NULL);

        TCITEM tci;
        tci.mask = TCIF_TEXT;
        tci.pszText = "CPU"; TabCtrl_InsertItem(hwndTab, 0, &tci);
        tci.pszText = "Caches"; TabCtrl_InsertItem(hwndTab, 1, &tci);
        tci.pszText = "Mainboard"; TabCtrl_InsertItem(hwndTab, 2, &tci);
        tci.pszText = "Memory"; TabCtrl_InsertItem(hwndTab, 3, &tci);
        tci.pszText = "Graphics"; TabCtrl_InsertItem(hwndTab, 4, &tci);
        tci.pszText = "Bench"; TabCtrl_InsertItem(hwndTab, 5, &tci);
        tci.pszText = "About"; TabCtrl_InsertItem(hwndTab, 6, &tci);

        hwndPanel = CreateWindowEx(0, "STATIC", "",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 50, 460, 240, hwnd, NULL, NULL, NULL);

        char buf[512];
        preencher_info_cpu(buf, sizeof(buf));
        SetWindowText(hwndPanel, buf);
        RETORNE 0;
    FIM_SE

    SE (msg == WM_NOTIFY) ENTAO
        LPNMHDR nm = (LPNMHDR)lParam;
        SE (nm->idFrom == ID_TAB && nm->code == TCN_SELCHANGE) ENTAO
            int sel = TabCtrl_GetCurSel(hwndTab);
            char buf[512];
            SE (sel == 0) ENTAO preencher_info_cpu(buf, sizeof(buf));
            SENAO_SE (sel == 3) ENTAO preencher_info_mem(buf, sizeof(buf));
            SENAO_SE (sel == 6) ENTAO snprintf(buf, sizeof(buf), "BASIC-Z 1.0 -- inspirado em CPU-Z\r\nEscrito em BASICGOL");
            SENAO snprintf(buf, sizeof(buf), "Nao implementado nesta versao.");
            FIM_SE
            SetWindowText(hwndPanel, buf);
        FIM_SE
        RETORNE 0;
    FIM_SE

    SE (msg == WM_DESTROY) ENTAO
        PostQuitMessage(0);
        RETORNE 0;
    FIM_SE

    RETORNE DefWindowProc(hwnd, msg, wParam, lParam);
}

INICIO
    HINSTANCE hInst = GetModuleHandle(NULL);

    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "BasicZGui";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "BasicZGui", "BASIC-Z",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 340,
        NULL, NULL, hInst, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    ENQUANTO GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    FIM_ENQUANTO
FIM
