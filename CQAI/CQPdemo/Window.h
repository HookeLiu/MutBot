#pragma once

/* CQ�˵Ĵ������� */
static TCHAR wcName[] = _T("HIDDEN_WINDOW_CLASS");
static TCHAR wcTitle[] = _T("CQ_WindowMsg_Ex");

/* PY�˵Ĵ������� */
static TCHAR PyWC[] = _T("PyWin32GUI_WINDOW_CLASS");
static TCHAR PyTitle[] = _T("MutBot�м��������");

extern HWND createMainWindow(WNDPROC CallBackFunc);