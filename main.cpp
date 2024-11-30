#include <locale.h>
#include "private.h"
#include "main.h"
#include "text_editor.h"

HINSTANCE g_inst;
HWND g_main_window;
TextEditor *g_text_editor;

WCHAR *main_window_title = TEXT("IME-aware application sample (with IMM)");
WCHAR *main_window_class = TEXT("ImmPad");

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_LBUTTONDOWN:
        SetFocus(hWnd);
        break;

    case WM_SIZE:
        RECT rc;
        GetClientRect(hWnd, &rc);
        if (g_text_editor)
        {
            g_text_editor->Move(10, 10, rc.right - 20, (int)(rc.bottom / 2));
        }
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL Startup(HINSTANCE hInstance, int nCmdShow)
{
    g_inst = hInstance;

    SetProcessDPIAware();

    RAWINPUTDEVICE devices[1];
    memset(devices, 0, 1 * sizeof(RAWINPUTDEVICE));
    devices[0].usUsagePage = 0x01;
    devices[0].usUsage = 0x06;

    if (RegisterRawInputDevices(devices, 1, sizeof(RAWINPUTDEVICE)))
    {
        /* Success. */
    }
    else
    {
        printc("could not register for RawInput: %d\n", int(GetLastError()));
    }

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = main_window_class;
    wcex.hIconSm = NULL;
    RegisterClassEx(&wcex);

    TextEditor::RegisterClass(hInstance);

    // -----

    g_main_window = CreateWindow(main_window_class, main_window_title, WS_OVERLAPPEDWINDOW,
                                 (GetSystemMetrics(SM_CXSCREEN) - 800) / 2,
                                 (GetSystemMetrics(SM_CYSCREEN) - 600) / 2,
                                 800, 600, NULL, NULL, hInstance, NULL);

    if (!g_main_window)
    {
        return FALSE;
    }

    ImmAssociateContext(g_main_window, NULL);
    printc("IME is disabled for main window.\n");

    // -----

    g_text_editor = new TextEditor();
    g_text_editor->Create(g_main_window);

    // -----

    ShowWindow(g_main_window, nCmdShow);
    UpdateWindow(g_main_window);

    ShowWindow(g_text_editor->GetWnd(), SW_SHOW);

    SetFocus(g_text_editor->GetWnd());

    return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    AttachConsole(ATTACH_PARENT_PROCESS);

    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    setlocale(LC_ALL, ".UTF-8");

    // -----

    if (!Startup(hInstance, nCmdShow))
    {
        return FALSE;
    }

    // -----

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
