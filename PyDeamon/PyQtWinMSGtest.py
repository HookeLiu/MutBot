import sys
from PyQt5.QtWidgets import QMainWindow, QApplication
import ctypes

class MainWindow(QMainWindow):

    def __init__(self):
        QMainWindow.__init__(self)
        
    def nativeEvent(self, eventType, msg):
        try:
            print("%d" % msg)
        except:
            msg = 0
        #message = ctypes.wintypes.MSG.from_address(msg.__int__())
        if 1 == 0 :#message.message == 0x219:  # WM_DEVICECHANGE
            if message.wParam == 0x8000:  # DBT_DEVICEARRIVAL
                print("in")
            elif message.wParam == 0x8004:  # DBT_DEVICEREMOVECOMPLETE
                print("out")
            elif message.wParam == 0x0007:  # DBT_DEVNODE_CHANGED
                print("node")
        return False, 0

app = QApplication(sys.argv)
window = MainWindow()
window.show()
sys.exit(app.exec_())