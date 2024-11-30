#include "private.h"
#include "text_editor.h"

extern HINSTANCE g_inst;

WCHAR *text_editor_class = TEXT("TextEditorClassName");

#define TEXT_SIZE_MAX (1024 - 1)

TextEditor::TextEditor()
{
    m_hwnd = NULL;

    HFONT hfontTemp = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    GetObject(hfontTemp, sizeof(LOGFONT), &m_font);

    m_comp_str = NULL;
    m_comp_str_size_e = 0;

    m_comp_attrs = NULL;
    m_comp_attrs_size_e = 0;

    m_comp_clauses = NULL;
    m_comp_clauses_size_e = 0;

    m_comp_cursor_pos = -1;

    m_text = (WCHAR *)malloc(sizeof(WCHAR) * (TEXT_SIZE_MAX + 1));
    memset(m_text, 0, sizeof(WCHAR) * (TEXT_SIZE_MAX + 1));
    m_text_size = 0;

    m_cursor_pos = 0;
    m_cursor_line = 0;
    m_cursor_column = 0;

    m_lines = NULL;
    m_line_count = 0;

    m_line_height = 0;
}

ATOM TextEditor::RegisterClass(HINSTANCE instance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = instance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_MENU + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = text_editor_class;
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}

HWND TextEditor::Create(HWND parent)
{
    m_hwnd = CreateWindow(text_editor_class,
                          NULL,
                          WS_CHILD,
                          0, 0, 0, 0,
                          parent,
                          NULL,
                          g_inst,
                          this);

    return m_hwnd;
}

// ------------------------------------------------------------

LRESULT CALLBACK TextEditor::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HIMC himc;
    TextEditor *ptic;

    switch (message)
    {
    case WM_CREATE:
    {
        SetThis(hWnd, ((CREATESTRUCT *)lParam)->lpCreateParams);
        GetThis(hWnd)->OnCreate(hWnd, wParam, lParam);

        return 0;
    }
    break;

    case WM_PAINT:
    {
        ptic = GetThis(hWnd);
        hdc = BeginPaint(hWnd, &ps);
        if (ptic)
            ptic->OnPaint(hdc);
        EndPaint(hWnd, &ps);

        return 0;
    }
    break;

    case WM_LBUTTONDOWN:
    {
        ptic = GetThis(hWnd);
        if (ptic)
            return ptic->OnLButtonDown(wParam, lParam);
    }
    break;

    case WM_SETFOCUS:
    {
        ptic = GetThis(hWnd);
        if (ptic)
            return ptic->OnSetFocus(wParam, lParam);
    }
    break;

    case WM_KILLFOCUS:
    {
        ptic = GetThis(hWnd);
        if (ptic)
            return ptic->OnKillFocus(wParam, lParam);
    }
    break;

    case WM_INPUT: /* UNUSED */
    {
        /**
         * 下面的代码和输入法没有什么关系，仅作为参考。
         * 如果你的程序使用 ToUnicodeEx() 获取按键对应的字符，
         * 则要注意其第二个参数 wScanCode 不能包含扩展标记。
         * 譬如：
         *   主键盘上的 "/" 键，对应的扫描码为 0x0035，虚拟按键为 VK_OEM_2。
         *   小键盘上的 "/" 键，对应的扫描码为 0xE035，虚拟按键为 VK_DIVIDE。
         * 传递给 ToUnicodeEx() 函数时，wScanCode 参数都应该传递 0x0035。
         * 即：
         *    ToUnicodeEx(VK_OEM_2, 0x0035, ...) [SUCCESS]
         *    ToUnicodeEx(VK_DIVIDE, 0x0035, ...) [SUCCESS]
         *    ToUnicodeEx(VK_DIVIDE, 0xE035, ...) [FAILED]
         *
         * 参考：https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-tounicodeex
         */
        RAWINPUT raw;
        RAWINPUT *raw_ptr = &raw;
        unsigned int rawSize = sizeof(RAWINPUT);

        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw_ptr, &rawSize, sizeof(RAWINPUTHEADER));

        switch (raw.header.dwType)
        {
        case RIM_TYPEKEYBOARD:
        {
            bool key_down = !(raw.data.keyboard.Flags & RI_KEY_BREAK) && raw.data.keyboard.Message != WM_KEYUP &&
                            raw.data.keyboard.Message != WM_SYSKEYUP;
            if (key_down)
            {
                printc(CCFY "RawInput (key down): 0x%X", raw.data.keyboard.VKey);
                printc(CCFY "  MakeCode: 0x%X %s", raw.data.keyboard.MakeCode,
                       raw.data.keyboard.Flags & RI_KEY_E0 ? "(Extend)" : "");
                printc(CCFY "  Flags: 0x%X", raw.data.keyboard.Flags);

                WCHAR charactor = MapVirtualKeyW(raw.data.keyboard.VKey, MAPVK_VK_TO_CHAR);
                switch (charactor)
                {
                case 0x001B: // ESC
                case 0x0008: // BS
                case 0x000A: // LF
                case 0x000D: // CR
                    break;
                default:
                    BYTE state[256] = {0};
                    GetKeyboardState((PBYTE)state);
                    wchar_t utf16[3] = {0};
                    int len;
                    len = ToUnicodeEx(raw.data.keyboard.VKey, raw.data.keyboard.MakeCode, state, utf16, 2, 0, GetKeyboardLayout(0));
                    if (len > 0)
                    {
                        printc(CCFY "ToUnicode (len: %d): \"%ls\"", len, utf16);
                    }
                    else
                    {
                        printc(CCFY "ToUnicode (len: %d): FAILED.", len);
                    }
                    break;
                }
            }
        }
        break;
        }
    }
    break;

    case WM_KEYDOWN:
    {

        ptic = GetThis(hWnd);
        if (ptic)
            return ptic->OnKeyDown(wParam, lParam);
    }
    break;

    case WM_CHAR:
    {
        ptic = GetThis(hWnd);
        if (ptic)
            return ptic->OnChar(wParam, lParam);
    }
    break;

    case WM_IME_SETCONTEXT: /* UNUSED */
    {
        /**
         * 按照微软文档的说法，如果自行绘制合成文本，则应该清除 ISC_SHOWUICOMPOSITIONWINDOW 标记，
         * 以便隐藏 IMM 自带的合成文本窗口。
         * 但是根据实际测试，自带的合成文本窗口仅在 WM_IME_STARTCOMPOSITION 消息返回 0 时隐藏。
         * 造成这个结果的原因未知。
         */

        // printc(CCFA "WM_IME_SETCONTEXT");
        // return DefWindowProc(hWnd, message, wParam, lParam & ~ISC_SHOWUICOMPOSITIONWINDOW);
    }
    break;

    case WM_IME_NOTIFY: /* UNUSED */
    {
        if (wParam == IMN_OPENCANDIDATE)
        {
            /**
             * 当程序接收到该通知时，应该使用 ImmSetCandidateWindow() 重新定位候选窗口。
             *
             * 注意：在本示例中，总是通过主动地更新候选窗口的位置，不理会该通知。
             *
             * 参考：https://learn.microsoft.com/en-us/windows/win32/intl/imn-opencandidate
             */

            // printc(CCFA "WM_IME_NOTIFY: IMN_OPENCANDIDATE");
        }
    }
    break;

    case WM_IME_REQUEST: /* UNUSED */
    {
        if (wParam == IMR_QUERYCHARPOSITION)
        {
            /**
             * 但程序接收到该请求时，应该将请求的字符位置写入到 lParam 指向的 IMECHARPOSITION 结构中。
             * 输入法会根据写入的位置重新定位候选窗口。
             * 如果没有修改 IMECHARPOSITION 结构，则输入法会保持候选窗口的位置。
             * 此时程序不能调用 ImmSetCandidateWindow() 设置候选窗口位置，否则会导致无限循环。
             *
             * 注意：在本示例中，总是通过主动地更新候选窗口的位置，不理会该请求。
             *
             * 参考：https://learn.microsoft.com/en-us/windows/win32/intl/imr-querycharposition
             */

            // printc(CCFA "WM_IME_REQUEST: IMR_QUERYCHARPOSITION");
        }
    }
    break;

    case WM_IME_STARTCOMPOSITION:
    {
        ptic = GetThis(hWnd);
        if (ptic)
            return ptic->OnIMECompositionStart(wParam, lParam);
    }
    break;

    case WM_IME_COMPOSITION:
    {
        ptic = GetThis(hWnd);
        if (ptic)
            return ptic->OnIMEComposition(wParam, lParam);
    }
    break;

    case WM_IME_ENDCOMPOSITION:
    {
        ptic = GetThis(hWnd);
        if (ptic)
            return ptic->OnIMECompositionEnd(wParam, lParam);
    }
    break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

// ------------------------------------------------------------

LRESULT TextEditor::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    UpdateLayout();

    return 0;
}

LRESULT TextEditor::OnPaint(HDC hdc)
{
    // printc("WM_PAINT");

    HFONT hFont = ::CreateFontIndirect(&m_font);

    if (hFont)
    {
        HFONT hFontOrg = (HFONT)::SelectObject(hdc, hFont);

        Render(hdc);

        ::SelectObject(hdc, hFontOrg);
        ::DeleteObject(hFont);
    }

    return 0;
}

LRESULT TextEditor::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    SetFocus(m_hwnd);

    return 0;
}

LRESULT TextEditor::OnSetFocus(WPARAM wParam, LPARAM lParam)
{
    printc(CCFB "WM_SETFOCUS");

    ImmAssociateContextEx(m_hwnd, NULL, IACE_DEFAULT);

    /* Create and show system caret. */
    UpdateCandidateWindowPos();

    /* Draw cursor. */
    UpdateUI();

    return 0;
}

LRESULT TextEditor::OnKillFocus(WPARAM wParam, LPARAM lParam)
{
    printc(CCFB "WM_KILLFOCUS");

    /**
     * 如果你使用的是 Direct UI，请在输入框失去焦点的时候强制完成或取消合成，并且停用输入法。
     *
     * 如果你使用的是 Lagacy UI，则无需进行任何操作。
     * - IMM 会自动强制完成合成。
     * - 窗口的输入法启停状态不影响新的焦点窗口的输入法启停状态。
     */

    // HIMC himc = ImmGetContext(m_hwnd);
    // if (himc)
    // {
    //     ImmNotifyIME(himc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0); // or CPS_CANCEL
    //     ImmReleaseContext(m_hwnd, himc);
    // }
    // ImmAssociateContextEx(m_hwnd, NULL, 0);

    /* Destroy system caret. */
    DestroyCaret();

    /* Erase cursor. */
    UpdateUI();

    return 0;
}

LRESULT TextEditor::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    {
        /**
         * 下面的代码仅用于输出调试信息。
         */

        wchar_t key_name[256] = L"??";
        GetKeyNameTextW(lParam, (LPWSTR)&key_name, 256);

        WORD key_flags = HIWORD(lParam);
        USHORT scan_code = LOBYTE(key_flags);
        // extended-key flag, 1 if scancode has 0xE0 prefix
        if ((key_flags & KF_EXTENDED) == KF_EXTENDED)
        {
            scan_code = MAKEWORD(scan_code, 0xE0);
        }
        UINT original_key = MapVirtualKey(scan_code, MAPVK_VSC_TO_VK_EX);

        printc(CCFA "WM_KEYDOWN: \"%ls\" (0x%X)%s", key_name, original_key,
               (wParam == VK_PROCESSKEY ? " [VK_PROCESSKEY]" : ""));
    }

    if (wParam == VK_PROCESSKEY)
    {
        return 0;
    }

    switch (0xff & wParam)
    {
    case VK_LEFT:
        SetCursor(m_cursor_pos - 1);
        break;

    case VK_RIGHT:
        SetCursor(m_cursor_pos + 1);
        break;

    case VK_BACK:
        RemoveText(1);
        break;
    }

    return 0;
}

LRESULT TextEditor::OnChar(WPARAM wParam, LPARAM lParam)
{
    WCHAR charactor = (WCHAR)wParam;

    switch ((WCHAR)wParam)
    {
    case 0x001B: // ESC
    case 0x0008: // BS
    case 0x000A: // LF
        return 0;
    case 0x000D: // CR
        printc(CCFA "WM_CHAR: [<CR>]");

        charactor = (WCHAR)'\n';
        InsertText(&charactor, 1);
        break;
    default:
        printc(CCFA "WM_CHAR: [%C]", (WCHAR)wParam);

        InsertText(&charactor, 1);
        break;
    }

    return 0;
}

LRESULT TextEditor::OnIMECompositionStart(WPARAM wParam, LPARAM lParam)
{
    printc(CCFA "WM_IME_STARTCOMPOSITION");

    return 0;
}

LRESULT TextEditor::OnIMEComposition(WPARAM wParam, LPARAM lParam)
{
    {
        /**
         * 下面的代码仅用于输出调试信息。
         */

        printc(CCFA "WM_IME_COMPOSITION");

        HIMC himc = ImmGetContext(m_hwnd);
        if (himc == NULL)
        {
            printc(CCFR "  himc == NULL");
            return 0;
        }

        int i = 0;
        LPWSTR __comp_str = NULL;

        // wParam is DBSC, useless
        // printc(CCZ0 "  wParam: %lX, [%C]", (ULONG)wParam, (WCHAR)wParam);

        printc(CCFA "  lParam: 0x%lX", (ULONG)lParam);

        if (lParam & GCS_RESULTSTR)
        {
            LONG __result_str_size_b = ::ImmGetCompositionString(himc, GCS_RESULTSTR, NULL, 0);
            LPWSTR __result_str = (LPWSTR)malloc(__result_str_size_b + sizeof(WCHAR));
            __result_str[__result_str_size_b / sizeof(WCHAR)] = (WCHAR)'\0';
            ::ImmGetCompositionString(himc, GCS_RESULTSTR, __result_str, __result_str_size_b);

            printc(CCFA "    GCS_RESULTSTR: [%ls]", __result_str);

            free(__result_str);
        }
        if (lParam & GCS_RESULTSTR)
        {
            printc(CCFA "    GCS_RESULTCLAUSE");
        }
        if (lParam & GCS_RESULTREADSTR)
        {
            printc(CCFA "    GCS_RESULTREADSTR");
        }
        if (lParam & GCS_RESULTREADCLAUSE)
        {
            printc(CCFA "    GCS_RESULTREADCLAUSE");
        }

        if (lParam & GCS_COMPSTR)
        {
            LONG __comp_str_size_b = ::ImmGetCompositionString(himc, GCS_COMPSTR, NULL, 0);
            __comp_str = (LPWSTR)malloc(__comp_str_size_b + sizeof(WCHAR));
            __comp_str[__comp_str_size_b / sizeof(WCHAR)] = (WCHAR)'\0';
            ::ImmGetCompositionString(himc, GCS_COMPSTR, __comp_str, __comp_str_size_b);

            printc(CCFA "    GCS_COMPSTR: [%ls]", __comp_str);
        }
        if (lParam & GCS_CURSORPOS)
        {
            LONG __cursor_pos = ::ImmGetCompositionString(himc, GCS_CURSORPOS, NULL, 0);
            printc(CCFA "    GCS_CURSORPOS: %ld", __cursor_pos);
        }
        if (lParam & GCS_DELTASTART)
        {
            LONG __delta_start = ::ImmGetCompositionString(himc, GCS_DELTASTART, NULL, 0);
            printc(CCZ0 "    GCS_DELTASTART: %ld", __delta_start);
        }
        if (lParam & GCS_COMPATTR)
        {
            // https://learn.microsoft.com/zh-cn/windows/win32/intl/composition-string
            LONG __attributes_size_b = ::ImmGetCompositionString(himc, GCS_COMPATTR, NULL, 0);
            UINT __attributes_size = __attributes_size_b / sizeof(BYTE);
            printc(CCFA "    GCS_COMPATTR: %d", __attributes_size);
            if (__attributes_size_b != 0)
            {
                BYTE *__attrs = (BYTE *)malloc(__attributes_size_b);
                ::ImmGetCompositionString(himc, GCS_COMPATTR, __attrs, __attributes_size_b);

                for (i = 0; i < __attributes_size; i++)
                {
                    switch (__attrs[i])
                    {
                    case ATTR_INPUT:
                        printc(CCFA "      [%02d]: ATTR_INPUT [%C]", i, __comp_str[i]);
                        break;
                    case ATTR_INPUT_ERROR:
                        printc(CCFP "      [%02d]: ATTR_INPUT_ERROR [%C]", i, __comp_str[i]);
                        break;
                    case ATTR_TARGET_CONVERTED:
                        printc(CCFG "      [%02d]: ATTR_TARGET_CONVERTED [%C]", i, __comp_str[i]);
                        break;
                    case ATTR_TARGET_NOTCONVERTED:
                        printc(CCFG "      [%02d]: ATTR_TARGET_NOTCONVERTED [%C]", i, __comp_str[i]);
                        break;
                    case ATTR_CONVERTED:
                        printc(CCFP "      [%02d]: ATTR_CONVERTED [%C]", i, __comp_str[i]);
                        break;
                    case ATTR_FIXEDCONVERTED:
                        printc(CCFP "      [%02d]: ATTR_FIXEDCONVERTED [%C]", i, __comp_str[i]);
                        break;
                    default:
                        printc(CCFR "      [%02d]: unknow [%C]", i, __comp_str[i]);
                        break;
                    }
                }

                free(__attrs);
            }
        }
        if (lParam & GCS_COMPCLAUSE)
        {
            LONG __clauses_size_b = ::ImmGetCompositionString(himc, GCS_COMPCLAUSE, NULL, 0);
            UINT __clauses_size = __clauses_size_b / sizeof(LONG);
            printc(CCFA "    GCS_COMPCLAUSE: %d + 1", __clauses_size - 1);
            if (__clauses_size_b != 0)
            {
                LONG *__clauses = (LONG *)malloc(__clauses_size_b);
                ::ImmGetCompositionString(himc, GCS_COMPCLAUSE, __clauses, __clauses_size_b);

                for (i = 0; i < __clauses_size; i++)
                {
                    if (i < __clauses_size - 1)
                    {
                        printc(CCFA "      [%02d]: %ld", i, __clauses[i]);
                    }
                    else
                    {
                        printc(CCFB "      [%02d]: %ld", i, __clauses[i]);
                    }
                }

                free(__clauses);
            }
        }
        if (lParam & GCS_COMPREADSTR)
        {
            printc(CCZ0 "    GCS_COMPREADSTR");
        }
        if (lParam & GCS_COMPREADATTR)
        {
            printc(CCZ0 "    GCS_COMPREADATTR");
        }
        if (lParam & GCS_COMPREADCLAUSE)
        {
            printc(CCZ0 "    GCS_COMPREADCLAUSE");
        }

        // 没有发现哪个输入法会使用该特性
        if (lParam & CS_INSERTCHAR)
        {
            printc(CCFY "    CS_INSERTCHAR");
        }
        if (lParam & CS_NOMOVECARET)
        {
            printc(CCFY "    CS_NOMOVECARET");
        }

        if (__comp_str)
        {
            free(__comp_str);
        }

        ImmReleaseContext(m_hwnd, himc);
    }

    // -----

    HIMC himc = ImmGetContext(m_hwnd);
    if (himc)
    {
        if (m_comp_str)
        {
            m_cursor_pos = m_comp_end_pos;
            RemoveText(m_comp_str_size_e);
            m_comp_start_pos = 0;
            m_comp_end_pos = 0;

            free(m_comp_str);
            m_comp_str = NULL;
            m_comp_str_size_e = 0;
        }
        if (m_comp_attrs)
        {
            free(m_comp_attrs);
            m_comp_attrs = NULL;
            m_comp_attrs_size_e = 0;
        }
        if (m_comp_clauses)
        {
            free(m_comp_clauses);
            m_comp_clauses = NULL;
            m_comp_clauses_size_e = 0;
        }
        m_comp_cursor_pos = 0;

        if (lParam & GCS_RESULTSTR)
        {
            LONG result_str_size_b = ::ImmGetCompositionString(himc, GCS_RESULTSTR, NULL, 0);
            if (result_str_size_b != 0)
            {
                LPWSTR result_str = (LPWSTR)malloc(result_str_size_b);
                ::ImmGetCompositionString(himc, GCS_RESULTSTR, result_str, result_str_size_b);

                InsertText(result_str, result_str_size_b / sizeof(WCHAR));

                free(result_str);
            }
        }

        if ((lParam & GCS_COMPSTR))
        {
            LONG comp_str_size_b = ::ImmGetCompositionString(himc, GCS_COMPSTR, NULL, 0);
            // 当取消合成时，获取得到的合成字符串就是空的。
            if (comp_str_size_b != 0)
            {
                LPWSTR comp_str = (LPWSTR)malloc(comp_str_size_b);
                ::ImmGetCompositionString(himc, GCS_COMPSTR, comp_str, comp_str_size_b);

                if (comp_str)
                {
                    m_comp_str = comp_str;
                    m_comp_str_size_e = comp_str_size_b / sizeof(WCHAR);

                    BYTE *attrs = NULL;
                    LONG attributes_size_b = ::ImmGetCompositionString(himc, GCS_COMPATTR, NULL, 0);
                    if (attributes_size_b != 0)
                    {
                        attrs = (BYTE *)malloc(attributes_size_b);
                        ::ImmGetCompositionString(himc, GCS_COMPATTR, attrs, attributes_size_b);
                    }
                    m_comp_attrs = attrs;
                    m_comp_attrs_size_e = attributes_size_b / sizeof(BYTE);

                    LONG *clauses = NULL;
                    LONG clauses_size_b = ::ImmGetCompositionString(himc, GCS_COMPCLAUSE, NULL, 0);
                    if (clauses_size_b != 0)
                    {
                        clauses = (LONG *)malloc(clauses_size_b);
                        ::ImmGetCompositionString(himc, GCS_COMPCLAUSE, clauses, clauses_size_b);
                    }
                    m_comp_clauses = clauses;
                    m_comp_clauses_size_e = clauses_size_b / sizeof(LONG);

                    LONG cursor_pos = ::ImmGetCompositionString(himc, GCS_CURSORPOS, NULL, 0);
                    m_comp_cursor_pos = cursor_pos;

                    m_comp_start_pos = m_cursor_pos;
                    m_comp_end_pos = m_cursor_pos + m_comp_str_size_e;
                    InsertText(m_comp_str, m_comp_str_size_e);

                    SetCursor(m_comp_start_pos + m_comp_cursor_pos);
                }
            }
        }

        ImmReleaseContext(m_hwnd, himc);
    }

    return 0;
}

LRESULT TextEditor::OnIMECompositionEnd(WPARAM wParam, LPARAM lParam)
{
    printc(CCFA "WM_IME_ENDCOMPOSITION");

    if (m_comp_str)
    {
        /* { */
        /**
         * 这段代码目前主要应对旧版韩语输入法。
         * 旧版韩语输入法在完成合成时，会将最后的 WM_IME_COMPOSITION 消息放到 WM_IME_ENDCOMPOSITION 之后，
         * 而不是之前，导致之前插入到文本中的临时合成文本没有被正确清除，会导致多余的字符产生。
         * 为此在 WM_IME_ENDCOMPOSITION 时需要确保临时合成文本被删除。
         */

        m_cursor_pos = m_comp_end_pos;
        RemoveText(m_comp_str_size_e);
        m_comp_start_pos = 0;
        m_comp_end_pos = 0;

        /* } */

        free(m_comp_str);
        m_comp_str = NULL;
        m_comp_str_size_e = 0;
    }
    if (m_comp_attrs)
    {
        free(m_comp_attrs);
        m_comp_attrs = NULL;
        m_comp_attrs_size_e = 0;
    }
    if (m_comp_clauses)
    {
        free(m_comp_clauses);
        m_comp_clauses = NULL;
        m_comp_clauses_size_e = 0;
    }
    m_comp_cursor_pos = 0;

    return 0;
}

// ------------------------------------------------------------

void TextEditor::UpdateCandidateWindowPos()
{
    printc("UpdateCandidateWindowPos");

    /**
     * 在以下情况应该及时更新候选窗口的位置：
     * - 如果当前处于非合成状态，则当游标改变时更新候选窗口位置；
     * - 如果当前处于合成状态，则当 WM_IME_COMPOSITION 处理完毕时更新候选窗口位置。
     * - 控件在激活之时需要更新候选窗口的位置。这样做有两个原因：
     *   - 系统光标是全局资源，
     *     因此其它控件激活时，可能已经修改系统光标的位置，
     *     所以控件激活时需要重新定位候选窗口，确保系统光标的位置正确。
     *     控件失活时应该销毁系统光标，否则系统光标会一直显示。
     *   - 对于 Direct UI 框架下的控件，他们共用同一个窗口，
     *     因此其它控件激活时，可能已经修改候选窗口的位置，
     *     所以控件激活时需要重新定位候选窗口，确保候选窗口的位置正确。
     *     控件失活时无需更新候选窗口的位置。
     *
     * 注意：不要使用系统光标作为文本光标，
     * 因为系统光标用来帮助某些输入法定位候选窗口位置，
     * 而候选窗口的位置和文本光标的位置有时不在同一个位置。
     *
     * Update candidate window position in this situation:
     * - If not compositing:
     *   - After cursor changed.
     * - If compositing:
     *   - After composition updated. (WM_IME_COMPOSITION)
     *
     * Note: Do not use the system caret as the text cursor,
     * because the system caret use as the anchor of the candidate window for some IME,
     * in some situation, the position of text cursor and system caret is different.
     */

    /**
     * 当在合成时发生窗口焦点转移时，当前窗口会在 WM_KILLFOCUS 消息后收到 WM_IME_COMPOSITION 消息。
     * 系统光标已经在 WM_KILLFOCUS 销毁，但是在处理 WM_IME_COMPOSITION 消息时可能会重新显示系统光标，
     * 因此在此处需要判断当前窗口是否获得焦点，如果获得焦点才创建系统光标。
     */
    if (GetFocus() != m_hwnd)
    {
        return;
    }

    HIMC himc = ImmGetContext(m_hwnd);
    if (himc)
    {
        CANDIDATEFORM cf;
        cf.dwIndex = 0;
        cf.dwStyle = CFS_EXCLUDE;

        RECT rc_anchor;
        memset(&rc_anchor, 0, sizeof(RECT));

        RECT rc_editor;
        GetWindowRect(m_hwnd, &rc_editor);

        if (m_comp_str == NULL)
        {
            RectFromCharPos(m_cursor_pos, &rc_anchor);
        }
        else
        {
            /**
             * 如果存在转换目标（ATTR_TARGET_XXX），
             * 则将候选窗口对齐到转换目标的开头。
             * 否则如果存在分句且数量在两个及以上，
             * 则将候选你窗口对齐到光标所在的分句的开头（用于模拟TSF下输入法？？？？。
             * 否则候选窗口对齐到合成文本的开头。
             */

            int target_pos = -1;

            if (m_comp_attrs)
            {
                for (int i = 0; i < m_comp_attrs_size_e; i++)
                {
                    if (m_comp_attrs[i] == ATTR_TARGET_CONVERTED || m_comp_attrs[i] == ATTR_TARGET_NOTCONVERTED)
                    {
                        target_pos = i;
                        break;
                    }
                }
            }

            if (target_pos == -1 && m_comp_clauses && (m_comp_clauses_size_e - 1) >= 2)
            {
                if (m_comp_cursor_pos == m_comp_clauses[m_comp_clauses_size_e - 1])
                {
                    target_pos = m_comp_clauses[m_comp_clauses_size_e - 2];
                }
                else
                {
                    for (int i = 0; i < m_comp_clauses_size_e - 1; i++)
                    {
                        if (m_comp_clauses[i] <= m_comp_cursor_pos && m_comp_cursor_pos < m_comp_clauses[i + 1])
                        {
                            target_pos = m_comp_clauses[i];
                            break;
                        }
                    }
                }
            }

            if (target_pos == -1)
            {
                target_pos = 0;
            }

            RectFromCharPos(m_comp_start_pos + target_pos, &rc_anchor);
        }

        cf.ptCurrentPos.x = rc_anchor.left;
        cf.ptCurrentPos.y = rc_anchor.bottom;
        cf.rcArea.left = 0;
        cf.rcArea.right = rc_editor.right - rc_editor.left;
        cf.rcArea.top = rc_anchor.top;
        cf.rcArea.bottom = rc_anchor.bottom;
        ImmSetCandidateWindow(himc, &cf);

        DestroyCaret();
        CreateCaret(m_hwnd, NULL, 1, rc_anchor.bottom - rc_anchor.top);
        SetCaretPos(rc_anchor.left, rc_anchor.top);
        /* Visualizing the position of anchor points in debuging. */
        ShowCaret(m_hwnd);
    }
    ImmReleaseContext(m_hwnd, himc);
}

// ------------------------------------------------------------
// 布局渲染

void TextEditor::UpdateLayout()
{
    const LOGFONT *plf = &m_font;

    HDC hdc = ::GetDC(m_hwnd);
    if (hdc)
    {
        HFONT hFont = ::CreateFontIndirect(plf);
        if (hFont)
        {
            HFONT hFontOrg = (HFONT)::SelectObject(hdc, hFont);

            Layout(hdc);

            ::SelectObject(hdc, hFontOrg);
            ::DeleteObject(hFont);
        }
        ::ReleaseDC(m_hwnd, hdc);
    }
}

bool TextEditor::Layout(HDC hdc)
{
    // printc("Layout");

    /* Clean. */

    if (m_lines)
    {
        for (int i = 0; i < m_line_count; i++)
        {
            if (m_lines[i].char_infos)
            {
                free(m_lines[i].char_infos);
            }
        }
        free(m_lines);
    }
    m_lines = NULL;
    m_line_count = 0;

    /* Count Line, only supports 'LF'. */

    int line_count = 0;

    // Empty string.
    if (m_text_size == 0)
    {
        line_count++;
    }
    else
    {
        bool new_line = true;
        for (int i = 0; i < m_text_size; i++)
        {
            switch (m_text[i])
            {
            case (WCHAR)'\n': // LF, '\n', 0x000A
            {
                if (!new_line)
                {
                    new_line = true;
                }
                else // empty line
                {
                    line_count++;
                }
            }
            break;
            default:
            {
                if (new_line)
                {
                    line_count++;
                }
                new_line = false;
            }
            break;
            }
        }

        // The last charactor is '\n'.
        if (m_text_size != 0 && m_text[m_text_size - 1] == (WCHAR)'\n')
        {
            line_count++;
        }
    }

    m_lines = (LINEINFO *)malloc(line_count * sizeof(LINEINFO));
    memset(m_lines, 0, line_count * sizeof(LINEINFO));
    m_line_count = line_count;

    /* Count character of each line. */

    if (m_text_size == 0)
    {
        m_lines[0].pos = 0;
        m_lines[0].count = 0;
    }
    else
    {
        int current_line = -1;
        bool new_line = true;
        for (int i = 0; i < m_text_size; i++)
        {
            switch (m_text[i])
            {
            case (WCHAR)'\n':
            {
                if (!new_line)
                {
                    new_line = true;
                }
                else
                {
                    current_line++;
                    m_lines[current_line].pos = i;
                    m_lines[current_line].count = 0;
                }
            }
            break;
            default:
            {
                if (new_line)
                {
                    current_line++;
                    m_lines[current_line].pos = i;
                    m_lines[current_line].count = 1;
                }
                else
                {
                    m_lines[current_line].count++;
                }
                new_line = false;
            }
            break;
            }
        }
        if (m_text_size != 0 && m_text[m_text_size - 1] == (WCHAR)'\n')
        {
            current_line++;
            m_lines[current_line].pos = m_text_size;
            m_lines[current_line].count = 0;
        }
    }

    /* Get the rectangle of each characters. */

    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);

    m_line_height = tm.tmHeight + tm.tmExternalLeading;

    POINT co;
    co.x = 0;
    co.y = 0;

    for (int i = 0; i < m_line_count; i++)
    {
        m_lines[i].char_infos = NULL;

        if (m_lines[i].count)
        {
            m_lines[i].char_infos = (CHARINFO *)malloc(m_lines[i].count * sizeof(CHARINFO));
            memset(m_lines[i].char_infos, 0, m_lines[i].count * sizeof(CHARINFO));

            POINT co_prev = co;
            for (int j = 0; j < m_lines[i].count; j++)
            {
                SIZE size;
                GetTextExtentPoint32(hdc, m_text + m_lines[i].pos, j + 1, &size);
                co.x = size.cx;
                m_lines[i].char_infos[j].rc.left = co_prev.x;
                m_lines[i].char_infos[j].rc.top = co_prev.y;
                m_lines[i].char_infos[j].rc.right = co.x;
                m_lines[i].char_infos[j].rc.bottom = co_prev.y + m_line_height;

                co_prev = co;
            }
        }

        co.x = 0;
        co.y += m_line_height;
    }

    return true;
}

bool TextEditor::RectFromCharPos(UINT pos, RECT *r_rect)
{
    memset(r_rect, 0, sizeof(*r_rect));

    if (m_line_count == 0)
    {
        r_rect->left = 0;
        r_rect->right = 0;
        r_rect->top = 0;
        r_rect->bottom = r_rect->top + m_line_height;
        return TRUE;
    }
    else
    {
        for (int l = 0; l < m_line_count; l++)
        {
            if (pos < m_lines[l].pos)
            {
                continue;
            }

            if (pos == m_lines[l].pos)
            {
                r_rect->left = 0;
                r_rect->right = 0;
                r_rect->top = l * m_line_height;
                r_rect->bottom = r_rect->top + m_line_height;
                return TRUE;
            }
            else if (pos > m_lines[l].pos && pos <= m_lines[l].pos + m_lines[l].count)
            {
                if (pos == m_lines[l].pos + m_lines[l].count)
                {
                    *r_rect = m_lines[l].char_infos[pos - m_lines[l].pos - 1].rc;
                    r_rect->left = r_rect->right;
                }
                else
                {
                    *r_rect = m_lines[l].char_infos[pos - m_lines[l].pos].rc;
                    r_rect->right = r_rect->left;
                }
                return TRUE;
            }
        }
    }

    return FALSE;
}

void TextEditor::UpdateUI()
{
    InvalidateRect(m_hwnd, NULL, TRUE);
}

BOOL TextEditor::Render(HDC hdc)
{
    // printc("Render");

    POINT co;
    co.x = 0;
    co.y = 0;

    // 绘制文本

    for (UINT i = 0; i < m_line_count; i++)
    {
        if (m_lines[i].count)
        {
            TextOut(hdc, co.x, co.y,
                    m_text + m_lines[i].pos,
                    m_lines[i].count);
        }
        co.x = 0;
        co.y += m_line_height;
    }

    // 绘制光标

    if (GetFocus() == m_hwnd)
    {
        LOGBRUSH lbr;
        lbr.lbStyle = BS_SOLID;
        lbr.lbColor = 0;
        lbr.lbHatch = 0;
        lbr.lbColor = RGB(0, 128, 128);

        HPEN hpen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_FLAT, 2, &lbr, 0, NULL);

        HPEN hpenOrg;
        hpenOrg = (HPEN)SelectObject(hdc, hpen);

        RECT rc;
        RectFromCharPos(m_cursor_pos, &rc);

        POINT pts[2];
        pts[0].x = rc.left;
        pts[0].y = rc.top;
        pts[1].x = rc.left;
        pts[1].y = rc.bottom;
        Polyline(hdc, pts, 2);

        SelectObject(hdc, hpenOrg);
    }

    // 绘制合成文本下划线

    for (UINT i = 0; i < m_line_count; i++)
    {
        if (m_comp_attrs &&
            (m_comp_start_pos >= m_lines[i].pos) &&
            (m_comp_end_pos <= m_lines[i].pos + m_lines[i].count))
        {
            UINT nCompStartInLine = 0;
            UINT nCompEndInLine = m_lines[i].count;
            int nBaseLineWidth = (m_line_height / 18) + 1;

            if (m_comp_start_pos > m_lines[i].pos)
                nCompStartInLine = m_comp_start_pos - m_lines[i].pos;

            if (m_comp_end_pos < m_lines[i].pos + m_lines[i].count)
                nCompEndInLine = m_comp_end_pos - m_lines[i].pos;

            for (UINT j = nCompStartInLine; j < nCompEndInLine; j++)
            {
                HPEN hpen = NULL;
                UINT uCurrentCompPos = m_lines[i].pos + j - m_comp_start_pos;
                BOOL bClause = FALSE;

                if (m_comp_clauses)
                {
                    for (LONG k = 0; k < m_comp_clauses_size_e; k++)
                    {
                        if (uCurrentCompPos + 1 == (UINT)m_comp_clauses[k])
                        {
                            bClause = TRUE;
                        }
                    }
                }

                hpen = CreateUnderlinePen(m_comp_attrs[uCurrentCompPos], nBaseLineWidth);
                if (hpen)
                {
                    HPEN hpenOrg;
                    hpenOrg = (HPEN)SelectObject(hdc, hpen);
                    RECT rc = m_lines[i].char_infos[j].rc;

                    POINT pts[2];
                    pts[0].x = rc.left;
                    pts[0].y = rc.bottom;
                    pts[1].x = rc.right - (bClause ? nBaseLineWidth : 0);
                    pts[1].y = rc.bottom;
                    Polyline(hdc, pts, 2);

                    SelectObject(hdc, hpenOrg);
                }
            }
        }
    }

    return TRUE;
}

HPEN TextEditor::CreateUnderlinePen(BYTE attr, int width)
{
    const DWORD s_dwDotStyles[] = {1, 2};
    const DWORD s_dwDashStyles[] = {3, 2};

    DWORD dwPenStyle = PS_GEOMETRIC | PS_SOLID;
    DWORD dwStyles = 0;
    const DWORD *lpdwStyles = NULL;
    switch (attr)
    {
    case ATTR_INPUT:
        dwPenStyle = PS_GEOMETRIC | PS_USERSTYLE | PS_ENDCAP_FLAT;
        dwStyles = 2;
        lpdwStyles = s_dwDotStyles;
        break;

    case ATTR_TARGET_CONVERTED:
    case ATTR_TARGET_NOTCONVERTED:
        dwPenStyle = PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_FLAT;
        width *= 2;
        break;

    case ATTR_CONVERTED:
    case ATTR_INPUT_ERROR:
    case ATTR_FIXEDCONVERTED:
    default:
        dwPenStyle = PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_FLAT;
        break;
    }

    LOGBRUSH lbr;
    lbr.lbStyle = BS_SOLID;
    lbr.lbColor = 0;
    lbr.lbHatch = 0;
    lbr.lbColor = RGB(0, 0, 0);

    return ExtCreatePen(dwPenStyle, width, &lbr, dwStyles, lpdwStyles);
}

// ------------------------------------------------------------
// 文本操作

BOOL TextEditor::InsertText(LPWSTR text, UINT text_size)
{
    // printc("InsertText: %d", text_size);

    if (m_cursor_pos + text_size > TEXT_SIZE_MAX)
    {
        return FALSE;
    }

    UINT right_count = sizeof(WCHAR) * (m_text_size - m_cursor_pos);
    memmove(&m_text[m_cursor_pos + text_size], &m_text[m_cursor_pos], right_count);
    memcpy(&m_text[m_cursor_pos], text, sizeof(WCHAR) * text_size);

    m_text_size += text_size;
    m_text[m_text_size] = (WCHAR)'\0';

    m_cursor_pos += text_size;

    UpdateLayout();

    UpdateCandidateWindowPos();

    UpdateUI();

    return TRUE;
}

BOOL TextEditor::RemoveText(UINT text_size)
{
    if (((INT)m_cursor_pos - (INT)text_size) < 0)
    {
        return FALSE;
    }

    UINT right_count = sizeof(WCHAR) * (m_text_size - m_cursor_pos);
    memmove(&m_text[m_cursor_pos - text_size], &m_text[m_cursor_pos], right_count);

    m_text_size -= text_size;
    m_text[m_text_size] = (WCHAR)'\0';

    m_cursor_pos -= text_size;

    UpdateLayout();

    UpdateCandidateWindowPos();

    UpdateUI();

    return TRUE;
}

BOOL TextEditor::SetCursor(UINT cursor_pos)
{
    if (((INT)cursor_pos) < 0 || cursor_pos > m_text_size)
    {
        return FALSE;
    }

    m_cursor_pos = cursor_pos;

    UpdateCandidateWindowPos();

    UpdateUI();

    return TRUE;
}
