#pragma once

typedef struct tagCHARINFO
{
    RECT rc;
} CHARINFO;

typedef struct tagLINEINFO
{
    UINT pos;
    UINT count;
    CHARINFO *char_infos;
} LINEINFO;

class TextEditor
{
public:
    TextEditor();

    static ATOM RegisterClass(HINSTANCE instance);
    
    HWND Create(HWND parent);
    HWND GetWnd() { return m_hwnd; }

    void Move(int x, int y, int w, int h)
    {
        if (IsWindow(m_hwnd))
        {
            MoveWindow(m_hwnd, x, y, w, h, TRUE);
        }
    }

    static void SetThis(HWND hWnd, LPVOID _this_)
    {
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)_this_);
    }
    static TextEditor *GetThis(HWND hWnd)
    {
        TextEditor *p = (TextEditor *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        return p;
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(HDC hdc);
    LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
    LRESULT OnSetFocus(WPARAM wParam, LPARAM lParam);
    LRESULT OnKillFocus(WPARAM wParam, LPARAM lParam);
    LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
    LRESULT OnChar(WPARAM wParam, LPARAM lParam);
    LRESULT OnIMECompositionStart(WPARAM wParam, LPARAM lParam);
    LRESULT OnIMEComposition(WPARAM wParam, LPARAM lParam);
    LRESULT OnIMECompositionEnd(WPARAM wParam, LPARAM lParam);

    void UpdateCandidateWindowPos();

    void UpdateLayout();
    bool Layout(HDC hdc);
    bool RectFromCharPos(UINT nPos, RECT *prc);

    void UpdateUI();
    BOOL Render(HDC hdc);
    HPEN CreateUnderlinePen(BYTE bAttr, int nWidth);

    BOOL InsertText(LPWSTR text, UINT text_size);
    BOOL RemoveText(UINT count);
    BOOL SetCursor(UINT cursor_pos);

private:
    HWND m_hwnd;

    // 合成文本数据

    WCHAR *m_comp_str;
    UINT m_comp_str_size_e;

    UINT m_comp_cursor_pos;

    BYTE *m_comp_attrs;
    UINT m_comp_attrs_size_e;

    LONG *m_comp_clauses;
    UINT m_comp_clauses_size_e;

    UINT m_comp_start_pos;
    UINT m_comp_end_pos;

    // 文本数据

    WCHAR *m_text;
    UINT m_text_size;

    UINT m_cursor_pos;
    UINT m_cursor_line;
    UINT m_cursor_column;

    // 布局数据

    LOGFONT m_font;

    LINEINFO *m_lines;
    UINT m_line_count;

    int m_line_height;
};
