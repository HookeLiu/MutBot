#include "stdafx.h"

HWND createMainWindow(WNDPROC CallBackFunc)
{
    WNDCLASS  wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = CallBackFunc;
    wc.hInstance = GetModuleHandle(NULL);                                  // 应用程序的实例句柄
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = wcName;
    if (!RegisterClass(&wc))
    {
        return 0;
    }
    return CreateWindow(
        wcName,                                                 // 窗口类名
        wcTitle,                                                // 窗口标题
        WS_CHILD,                                               // 窗口外观样式
        0,                                                      // 相对父窗口的x坐标
        0,                                                      // 相对父窗口的y坐标
        10,                                                     // 窗口宽度                        
        10,                                                     // 窗口高度                    
        GetDesktopWindow(),                                     // 父窗口句柄 (可怜的桌面窗口又只能当背锅侠了....)
        nullptr,                                                // 菜单句柄                
        wc.hInstance,                                           // 当前应用程序的句柄                    
        nullptr                                                 // 附加数据                        
    );
    // 这里是这样的
    // 因为一个独立的窗口必须拥有一个消息循环, 而且这玩意还不能跨线程
    // 这意味着你的程序必须卡在那
    // 但是一种窗口例外——子窗口
    // 它使用父窗口的消息循环
    // 因此, 我使用桌面作为父窗口, 创建了一个不可见的子窗口

}

#ifdef __On_Debug__

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_USER:
        printf("| 0x%x| msg: %u; wParam: %u; lParam: %s \n", hWnd, msg, wParam, lParam);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return (LRESULT)0;
}

int main()
{
    HWND h = createMainWindow(WndProc);
    if (!h)
    {
        return 0;                           // 创建失败
    }
    // 下面这个窗口就准备好了
    system("pause");
    SendMessage(h, WM_USER, (WPARAM)233, (LPARAM)"测试1234"); // 一般习惯WPARAM为数值, LPARAM为地址
    SendMessage(h, WM_USER + 1, (WPARAM)666, (LPARAM)"ABCd"); // 可以连续发消息

    system("pause");
    return 0;
}
#endif // debugMode