#pragma once

/* CQ端的窗口命名 */
static TCHAR wcName[] = _T("HIDDEN_WINDOW_CLASS");
static TCHAR wcTitle[] = _T("CQ_WindowMsg_Ex");

/* PY端的窗口命名 */
static TCHAR PyWC[] = _T("PyWin32GUI_WINDOW_CLASS");
static TCHAR PyTitle[] = _T("MutBot中间层服务程序");

extern HWND createMainWindow(WNDPROC CallBackFunc);