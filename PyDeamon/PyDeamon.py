# 通过win32service做一些后台处理
import win32serviceutil 
import win32service 
import win32event 
import os
import socket
import time

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
        # 监视日志的变化, 当有变化时调用其他的分析和处理程序
        # 文件监视的程序就直接抄了
        import win32file
        import win32con

        ACTIONS = {1 : "Created", 2 : "Deleted", 3 : "Updated", 4 : "Renamed from something", 5 : "Renamed to something"}
        FILE_LIST_DIRECTORY = win32con.GENERIC_READ | win32con.GENERIC_WRITE
        path_to_watch = "P:/CQ/CQPRO/data/2154055060"

        hDir = win32file.CreateFile (
        path_to_watch,
        FILE_LIST_DIRECTORY,
        win32con.FILE_SHARE_READ | win32con.FILE_SHARE_WRITE | win32con.FILE_SHARE_DELETE,
        None,
        win32con.OPEN_EXISTING,
        win32con.FILE_FLAG_BACKUP_SEMANTICS,
        None
        )

  # ReadDirectoryChangesW takes a previously-created handle to a directory, a buffer size for results,
  # a flag to indicate whether to watch subtrees and a filter of what changes to notify.
  # NB Tim Juchcinski reports that he needed to up
  # the buffer size to be sure of picking up all events when a large number of files were deleted at once.
        def getChange(hDir, delay):
            while 1:
                results = win32file.ReadDirectoryChangesW (
                hDir,
                1024,
                True,
                win32con.FILE_NOTIFY_CHANGE_FILE_NAME |
                win32con.FILE_NOTIFY_CHANGE_DIR_NAME |
                win32con.FILE_NOTIFY_CHANGE_ATTRIBUTES |
                win32con.FILE_NOTIFY_CHANGE_SIZE |
                win32con.FILE_NOTIFY_CHANGE_LAST_WRITE |
                win32con.FILE_NOTIFY_CHANGE_SECURITY,
                None,
                None
                )
                for action, file in results:
                    full_filename = os.path.join (path_to_watch, file)
                    print (time.time(), full_filename, ACTIONS.get (action, "Unknown"))
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