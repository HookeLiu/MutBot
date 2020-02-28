import sys
import time
from PyQt5.QtCore import QObject, pyqtSignal, QEventLoop, QTimer, QMimeData, QCoreApplication
from PyQt5.QtWidgets import QMainWindow, QPushButton, QApplication, QTextEdit, QMessageBox
from PyQt5.QtGui import *
import logging
import MainUi

'''
控制台输出定向到Qtextedit中
'''


def printHTML(self, *args, sep=' ', end='\n', file=None, style='color:#40ef3f'):
    print(f"<span style={style}>{self}</span><br />")


class Stream(QObject):
    """Redirects console output to text widget."""
    newText = pyqtSignal(str)

    def write(self, text):
        self.newText.emit(str(text))


class GenMast(QMainWindow, MainUi.Ui_PyDaemon):
    """Main application window."""

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.i_copyConsole.clicked.connect(self.copyConsloe)
        self.i_cleanConsole.clicked.connect(self.clearConsole)
        self.show()

        # Custom output stream.
        sys.stdout = Stream(newText=self.onUpdateText)

    def onUpdateText(self, text):
        """Write console output to text widget."""
        cursor = self.o_Console.textCursor()
        cursor.movePosition(QTextCursor.End)
        cursor.insertHtml(text)
        self.o_Console.setTextCursor(cursor)
        self.o_Console.ensureCursorVisible()

    def closeEvent(self, event):
        """Shuts down application on close."""
        # Return stdout to defaults.
        sys.stdout = sys.__stdout__
        super().closeEvent(event)

    def printhello(self):
        print('hello')

    def genMastClicked(self):
        """Runs the main function."""
        print('Running...')
        self.printhello()
        loop = QEventLoop()
        QTimer.singleShot(2000, loop.quit)
        loop.exec_()
        print('Done.')

    def copyConsloe(self):
        clipboard = QApplication.clipboard()
        clipboard.setText(self.o_Console.toHtml())

    def clearConsole(self):
        self.o_Console.clear()
        printHTML("---------")


if __name__ == '__main__':
    # Run the application.
    app = QApplication(sys.argv)
    app.aboutToQuit.connect(app.deleteLater)
    gui = GenMast()

    logFile = "./debug.log"
    logFormat = logging.Formatter(
        '[%(levelname)8s] | %(asctime)s @%(relativeCreated)8d | "%(filename)15s" line%(lineno)4d, in `%(threadName)10s` : %(message)s',
        '%d %b %Y %H:%M:%S')
    printFormat = logging.Formatter('<span style="color:#cfdfc1; background-color:#7587ee;">[%(levelname)8s] | %(asctime)s : %(message)s</span><br />')
    fh = logging.FileHandler(logFile)
    fh.setFormatter(logFormat)
    ch = logging.StreamHandler(sys.stdout)
    ch.setFormatter(printFormat)
    # ch.setLevel(logging.INFO) # 开发中为了方便查看先不用info等级了
    ch.setLevel(logging.DEBUG)

    logger0 = logging.getLogger("all")  # 根记录器把所有内容记录到文件
    logger0.setLevel(logging.DEBUG)
    logger0.addHandler(fh)

    logger1 = logging.getLogger("all.consleLog")  # 控制台记录器的过滤器只显示info以上的, 所有记录会交到上层
    logger1.addHandler(ch)

    logger1.info("--------程序开始运行--------")
    printHTML("234567890-")
    printHTML("2*pi = %0.3f" %(2 * 3.1415926) )
    sys.exit(app.exec_())
#