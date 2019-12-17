# 通过win32service做一些后台处理
import win32serviceutil 
import win32service 
import win32event 
import os
import socket

class PythonService(win32serviceutil.ServiceFramework): # 框架就直接抄网上的了

    #服务名
    _svc_name_ = "MutBot"
    #服务显示名称
    _svc_display_name_ = "com.17ds8.AItest"
    #服务描述
    _svc_description_ = "MutBot后端服务程序测试"

    def __init__(self, args): 
        win32serviceutil.ServiceFramework.__init__(self, args) 
        self.hWaitStop = win32event.CreateEvent(None, 0, 0, None)
        self.isAlive = True

    def SvcDoRun(self):
        pass
        # 这里写自己的程序

    def SvcStop(self): 

        # 先告诉SCM停止这个过程
        self.ReportServiceStatus(win32service.SERVICE_STOP_PENDING) 
        # 设置事件
        win32event.SetEvent(self.hWaitStop) 
        self.isAlive = False

if __name__ == '__main__': 
    win32serviceutil.HandleCommandLine(PythonService)