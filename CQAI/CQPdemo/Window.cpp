#include "stdafx.h"

HWND createMainWindow(WNDPROC CallBackFunc)
{
    WNDCLASS  wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = CallBackFunc;
    wc.hInstance = GetModuleHandle(NULL);                                  // Ӧ�ó����ʵ�����
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = wcName;
    if (!RegisterClass(&wc))
    {
        return 0;
    }
    return CreateWindow(
        wcName,                                                 // ��������
        wcTitle,                                                // ���ڱ���
        WS_CHILD,                                               // ���������ʽ
        0,                                                      // ��Ը����ڵ�x����
        0,                                                      // ��Ը����ڵ�y����
        10,                                                     // ���ڿ��                        
        10,                                                     // ���ڸ߶�                    
        GetDesktopWindow(),                                     // �����ھ�� (���������洰����ֻ�ܵ���������....)
        nullptr,                                                // �˵����                
        wc.hInstance,                                           // ��ǰӦ�ó���ľ��                    
        nullptr                                                 // ��������                        
    );
    // ������������
    // ��Ϊһ�������Ĵ��ڱ���ӵ��һ����Ϣѭ��, ���������⻹���ܿ��߳�
    // ����ζ����ĳ�����뿨����
    // ����һ�ִ������⡪���Ӵ���
    // ��ʹ�ø����ڵ���Ϣѭ��
    // ���, ��ʹ��������Ϊ������, ������һ�����ɼ����Ӵ���

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
        return 0;                           // ����ʧ��
    }
    // ����������ھ�׼������
    system("pause");
    SendMessage(h, WM_USER, (WPARAM)233, (LPARAM)"����1234"); // һ��ϰ��WPARAMΪ��ֵ, LPARAMΪ��ַ
    SendMessage(h, WM_USER + 1, (WPARAM)666, (LPARAM)"ABCd"); // ������������Ϣ

    system("pause");
    return 0;
}
#endif // debugMode