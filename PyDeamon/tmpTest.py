import os
import win32file
import win32con
import time
import threading
import win32event

Tigger = "Global\\CQdbSync"

CQtigger = win32event.OpenEvent(win32event::EVENT_ALL_ACCESS, True, Tigger)

a = range(10, 30, 5)

for i in a:
    win32event.SetEvent(CQtigger)