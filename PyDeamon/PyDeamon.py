"""
中间层服务程序
与前端通信, 前置处理并调用适当的后端应用
维护一些后台任务, 使用窗口消息与前端通信触发和处理
By 佚之狗 Feb. 2020
"""

import os
import sqlite3
import sys
import threading
import time
from ctypes.wintypes import MSG, WPARAM
import win32api
import win32gui
from win32con import *
from PyQt5.QtWidgets import QMainWindow, QPushButton, QApplication, QTextEdit, QDialog, QMessageBox
from PyQt5.QtGui import *
from PyQt5.QtCore import Qt, QObject, pyqtSignal, QEventLoop, QTimer, QThread
import logging
import MainUi

import ChineseCalendar

# import simpleTest  # 后端应用示例
import scheduleCast

# TODO: 计划做成按配置文件动态加载和调用的

# 全局变量, 常量设置
AdminQQ, logginQQ = "2139223150", "2426046106"  # TODO: 打算做成用配置文件的, 以后再说吧...
AppUpdate = 0x1000  # CQ更新数据后通知后端处理
AppExit = 0x996
AppSync = 0x300  # 后端处理完后让CQ端同步处理
HandSk = 0x233
Breath = 0x500
CQreturn = 0x8888
CQTitle = "CQ_WindowMsg_Ex"
Timeout = 120e3  # 毫秒 (120秒)
CQwHandle = None
GBwHandle = None
WPA = None
LPA = None
PATH_DB = ["", ""]
Flag_exit = False
Setting_ConsoleAutoRoll = True
G_CommuCounter = 0
Time_G_Start = time.time()
Time_LastCommu = time.time()
Time_LastError = -1
Status_Hint = "正在初始化..."
if len(sys.argv) > 1:
    pwd = os.path.abspath(sys.argv[1])
else:
    pwd = '../../data'

def printc(self, style='color:#40ef3f;'):
    if type(self) == str:
        self.replace(" ", "&nbsp;")
    print(f"<span style={style}><em style='color:yellow'>⚡{time.strftime('%X', time.localtime())}⚡</em> {self}</span><br />")


# 给控制台窗口设个标题
os.system("title 中间层服务")

logFile = "./debug.log"
logFormat = logging.Formatter(
    '[%(levelname)8s] | %(asctime)s @%(relativeCreated)8d | "%(filename)15s" line%(lineno)4d, in `%(threadName)10s` : %(message)s',
    '%d %b %Y %H:%M:%S')
printFormat = logging.Formatter(
    '<span style="color:#015599; background-color:#f0e05e;">[%(levelname)8s] | %(asctime)s : %(message)s</span><br />')
fh = logging.FileHandler(logFile)
fh.setFormatter(logFormat)

logger0 = logging.getLogger("all")  # 根记录器把所有内容记录到文件
logger0.setLevel(logging.DEBUG)
logger0.addHandler(fh)

logger0.info("--------程序开始运行--------")


# 自定义类
class SysOutRedirect(QObject):  # 重定向标准输出到textWidget, 以便输出到图形界面
    newText = pyqtSignal(str)

    def write(self, text):
        self.newText.emit(str(text))

    def flush(self):
        sys.__stdout__.flush()

class ThreadWithReturn(threading.Thread):  # 带返回值的线程
    def __init__(self, target=None, args=()):
        super(ThreadWithReturn, self).__init__()
        self.func = target
        self.args = args
        self.result = None

    def run(self):
        self.result = self.func(*self.args)

    def getResult(self):
        try:
            return self.result
        except Exception as errInfo:
            logger1.error("遇到错误: ", errInfo)
            return None

class MainWindow(QMainWindow, MainUi.Ui_PyDaemon):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.t_Log.setColumnWidth(0, 25)
        self.t_Log.setColumnWidth(1, 45)
        self.t_Log.setColumnWidth(2, 45)
        self.t_Log.setColumnWidth(3, 433)
        self.o_Console.setWindowOpacity(0.616471607)
        self.i_copyConsole.clicked.connect(self.copyConsole)
        self.i_cleanConsole.clicked.connect(self.clearConsole)
        self.i_ConsoleAutoRoll.stateChanged.connect(self.setConsoleAutoRoll)
        self.i_cmdType.currentIndexChanged.connect(lambda: self.cmdEditerExec(process="edit"))
        self.i_execCMD.clicked.connect(lambda: self.cmdEditerExec(process="exec"))
        sys.stdout = SysOutRedirect(newText=self.onUpdateText)
        self.show()
        self.displayTimer = QTimer(self)
        self.displayTimer.setInterval(100)
        self.displayTimer.timeout.connect(self.displayRefuresh)

    def onUpdateText(self, text):
        """Write console output to text widget."""
        if len(text) < 1:
            ntext = "<br />"
        else:
            ntext = text

        if text.find("background-color:#f0e05e") > 0:  # logger的输出里有空格, 为了显示原始格式需要HTML空格转义
            ntext = ""
            tlen = len(text)
            for item in range(tlen):
                if item < 54 or item > tlen - 13 or text[item] != " ":
                    ntext += text[item]
                else:
                    ntext += "&nbsp;"
        cursor = self.o_Console.textCursor()
        cursor.movePosition(QTextCursor.End)
        self.o_currentStatus.setText(Status_Hint)
        cursor.insertHtml(ntext)
        if Setting_ConsoleAutoRoll:
            self.o_Console.setTextCursor(cursor)
            self.o_Console.ensureCursorVisible()

    def closeEvent(self, event):
        """Shuts down application on close."""
        # Return stdout to defaults.
        logger1.info("*****窗口被关闭, 开始退出流程...*****")
        global Flag_exit
        Flag_exit = True
        super().closeEvent(event)
        sys.stdout = sys.__stdout__

    def nativeEvent(self, eventType, msg):  # 定制nativeEvent使其处理本地(Windows)窗口消息
        message = MSG.from_address(int(msg))
        if message.message == WM_USER:
            self.displayTimer.stop()
            global G_CommuCounter, Time_LastCommu, CQwHandle
            G_CommuCounter += 1
            Time_LastCommu = time.time()
            self.n_triggerCount.display(G_CommuCounter)
            if message.wParam == Breath:  # 回应前端的心跳包
                try:
                    win32api.PostMessage(CQwHandle, WM_USER, Breath, 0x79)
                    self.s_signal_out.setChecked(True)
                    self.displayTimer.start(300)
                except Exception as errInfo:
                    global Time_LastError, Status_Hint
                    self.displayRefuresh()
                    self.displayTimer.stop()
                    Status_Hint = "发生异常, 等待处理..."
                    self.o_currentStatus.setText(Status_Hint)
                    logger1.error("回应前端心跳包出错: %s" % errInfo)
                    Time_LastError = time.time()
                    logger1.info("尝试更新前端窗口句柄")
                    try:
                        CQwHandle = win32gui.FindWindow(None, CQTitle)
                        logger1.info("更新前端窗口句柄成功. 新句柄 → 0x%x" % CQwHandle)
                        Status_Hint = "等待新任务中..."
                        self.o_currentStatus.setText(Status_Hint)
                    except Exception as errInfo:
                        logger1.error("尝试重新查找前端句柄失败: %s" % errInfo)
            else:
                global WPA, LPA
                WPA, LPA = message.wParam, message.lParam
                if WPA == CQreturn:
                    printc("---前端已执行命令, 返回结果: `%s`" % LPA)
                elif WPA == AppUpdate:
                    proc = ThreadWithReturn(target=Update, args=())
                    proc.start()
                    pass
                else:
                    printc(f"---前端通信{message.message}(WM_USER): {WPA}({hex(WPA)}), {LPA}({hex(LPA)})---",
                           style="color:#6daa6d")
            self.s_signal_in.setChecked(True)
            self.displayTimer.start(300)
        return False, 0

    def copyConsole(self):
        clipboard = QApplication.clipboard()
        clipboard.setText(self.o_Console.toPlainText())

    def clearConsole(self):
        self.o_Console.clear()
        printc("-" * 5 + f"于{time.strftime('%H:%m:%d', time.localtime(time.time()))}清屏" + "-" * 5,
               style="color:#655366")

    def setConsoleAutoRoll(self):
        global Setting_ConsoleAutoRoll
        Setting_ConsoleAutoRoll = bool(self.i_ConsoleAutoRoll.isChecked())

    def displayRefuresh(self):
        global Time_LastError, Status_Hint
        if (time.time() - Time_LastCommu) > 3:
            global Status_Hint, Time_LastError
            Status_Hint = "前端心跳包超时, 等待处理..."
            logger1.warning("与前端的通信中断")
            Time_LastError = time.time()
            self.displayTimer.stop()
        if Time_LastError > 0 and (time.time() - Time_LastError) > 10:
            if time.time() - Time_LastCommu < 3:
                Status_Hint = "等待新任务中..."
        self.o_currentStatus.setText(Status_Hint)
        self.s_signal_in.setChecked(False)
        self.s_signal_out.setChecked(False)

    def cmdEditerExec(self, cmdType: int = -1, process: str = "edit", cmdStr: str = ""):
        global G_CommuCounter
        if len(cmdStr) > 6:
            cmd = cmdStr
        else:
            cmd = self.i_cmdEditer.text()
        if cmdType != -1:
            typ = cmdType
        else:
            typ = self.i_cmdType.currentIndex()
        if typ == 0:  # SQL操作
            if process == "edit":
                self.i_cmdEditer.setText("INSERT INTO `main`.`event` (`TYPE`, `CONT`, `STATUS`) VALUES (?, '?', ?);")
            if process == "exec":
                printc("[SQL]&gt;&gt;&gt; %s" % cmd)
                try:
                    Conn_APP = sqlite3.connect(PATH_DB[-1])
                    Conn_APP.execute(cmd)
                    Conn_APP.commit()
                except Exception as errInfo:
                    logger1.error("控制台SQL语句执行--(%s)\n执行失败: %s" % (cmd, errInfo))
                    return -233
        if typ == 1:  # 前端命令
            if process == "edit":
                self.i_cmdEditer.setText("send msg to 616471607 ?测试2333?")
            if process == "exec":
                printc("[CQ]&gt;&gt;&gt; %s" % cmd)
                try:
                    Conn_APP = sqlite3.connect(PATH_DB[-1])
                except Exception as errInfo:
                    logger1.error("控制台前端命令执行--`APP`数据库连接失败: %s" % errInfo)
                    return -233
                try:
                    query_insert = "INSERT INTO `main`.`event` (`TYPE`, `CONT`, `STATUS`) VALUES (5233, ?, 233);"
                    Conn_APP.execute(query_insert, (cmd,))
                    Conn_APP.commit()
                    win32api.PostMessage(CQwHandle, WM_USER, AppSync, 0x11)
                    Time_LastCommu = time.time()
                    G_CommuCounter += 1
                    self.s_signal_out.setChecked(True)
                    self.displayTimer.start(300)
                except Exception as errInfo:
                    logger1.error("控制台前端命令执行--通信失败: %s" % errInfo)
                    return -233
        if typ == 2:  # Py语句
            if process == "edit":
                self.i_cmdEditer.setText("printc(time.localtime(time.time()))")
            if process == "exec":
                printc("&gt;&gt;&gt; %s" % cmd)
                try:
                    eval(cmd)
                except Exception as errInfo:
                    logger1.error("控制台Py语句执行--(%s)\n执行失败: %s" % (cmd, errInfo))
                    return -233


logger0.debug("应用初始化完成, 准备创建通信窗口...")
# 先创建用通信的窗口
# 考虑到这个中间层服务以后应该是要做个窗口的, 为了方便开发所以使用PyQt
QApplication.setAttribute(Qt.AA_EnableHighDpiScaling)
app = QApplication(sys.argv)
MWnd = MainWindow()

ch = logging.StreamHandler(sys.stdout)
ch.setFormatter(printFormat)
# ch.setLevel(logging.INFO) # 开发中为了方便查看先不用info等级了
ch.setLevel(logging.DEBUG)
logger1 = logging.getLogger("all.consleLog")  # 控制台记录器的过滤器只显示info以上的, 所有记录会交到上层
logger1.addHandler(ch)

Status_Hint = "查找数据库文件中..."
logger1.debug("开始查找数据库")
# 因为目前的设计是整个框架都使用数据库传递和记录数据, 所以这个中间层程序需要打开两个数据库: 前端app的db和酷Q自己的eventv2.db
# 不确定主程序和这个脚本在不在同一个目录, 又想要让各个模块间独立性尽可能强, 所以就搞成找文件的了... 也许这里以后得重新设计...
Time_start = time.time()
Path_scriptDir = os.path.abspath(pwd)
logger1.debug("查找路径 → %s" % Path_scriptDir)
DB_CQ = "eventv2.db"
DB_APP = "app.db"
count_files = 0
for path, folder, files in os.walk(Path_scriptDir):
    for file in files:
        if (file == DB_CQ and path.find(logginQQ) > -1) or (file == DB_APP and path.find("cn.17ds8.AItest") > -1):
            path = os.path.join(path, file)
            PATH_DB.append(path)
            count_files += 1
            if count_files > 5:
                logger1.error("找到了过多的文件, 请检查运行环境")
                break

if count_files < 2:
    logger1.critical("找不到数据库! 请确保运行环境正常. 如果是初次使用, 请尝试先用前端创建数据表.")
    logger1.info("**********程序退出************")
    os.system("pause")
    sys.exit(-233)

logger1.debug(
    "找到了%d个db文件, 取最后两个( %s, %s ), 耗时%.3f秒" % (count_files, PATH_DB[-2], PATH_DB[-1], time.time() - Time_start))


# 酷Q发出退出信号之后中间层应该询问是否保留服务, 若不保留则应确保全部退出
def changeVariableValueByName(Variable, hint):
    currentGlobals = globals().copy()
    VariablesID = {}
    for VariableName, VariableValue in currentGlobals.items():
        VariablesID[id(VariableValue)] = VariableName
    try:
        VariableName = VariablesID[id(Variable)]
    except KeyError:
        logger1.warning("***debug: 在不同的线程中运行, 获取不到出入变量的名字")
        VariableName = None
    newValue = str(input(hint)).upper()
    if len(newValue) > 0:
        if VariableName is not None:
            globals()[VariableName] = newValue
    else:
        newValue = None
    return [VariableName, newValue]


def G_exit():
    global Flag_exit, Status_Hint
    exitConfirm = "u"
    Status_Hint = "等待退出确认..."
    logger1.info("收到退出信号, 等待退出确认...")
    waitForConfirm = ThreadWithReturn(target=changeVariableValueByName,
                                      args=(exitConfirm, "收到了退出信号, 默认10秒后退出, 是否现在退出呢? (Y/n) 请输入: ",))
    waitForConfirm.start()
    try:
        for t in range(9):
            exitConfirm = waitForConfirm.getResult()[1]
            if exitConfirm == "u":
                sleep = QEventLoop()
                QTimer.singleShot(1000, sleep.quit)
                sleep.exec_()
            elif exitConfirm == "Y" or exitConfirm == "N":
                break
            else:
                logger1.warning("退出确认得到了非期望的输入(%s). 期望Y或N." % exitConfirm)
        if exitConfirm is None:
            raise AttributeError
        logger1.debug("***debug, got: %s" % exitConfirm)
    except Exception as errInfo:
        logger1.debug("***debug: %s", str(errInfo))
        exitConfirm = "u"
    if exitConfirm == "u":
        logger1.info("等待超时, 开始退出流程...")
        exitConfirm = "Ytt"
    if exitConfirm == "Ytt" or exitConfirm == "Y":
        if exitConfirm == "Y":
            logger1.info("确认退出, 开始退出流程...")
            pass  # 这里放程序退出逻辑
        sys.exit(233)
    if exitConfirm == "n":
        logger1.info("取消退出, 继续运行...")
        pass  # 这里放继续运行的逻辑
    return 0


# 酷Q发出更新信号时说明有需要立即处理的事件, 中间层需要做预处理或移交给后端
def Update():
    global Status_Hint, Time_LastCommu, G_CommuCounter
    Status_Hint = "新任务处理中..."
    logger1.info("收到更新信号, 开始后台处理...")
    try:
        Conn_APP = sqlite3.connect(PATH_DB[-1])
        Conn_CQ = sqlite3.connect(PATH_DB[-2])
    except sqlite3.OperationalError:
        logger1.error("数据库打开失败")
        logger1.info("**********程序退出************")
        sys.exit(-233)
    except NameError:
        logger1.error("数据库游标创建失败")
        logger1.info("**********程序退出************")
        sys.exit(-233)
    tasks = getNewTasks(Conn_APP)
    if tasks == 0:
        return 233
    else:
        logger1.info("找到%s个任务, 开始处理..." % len(tasks))
        count_cmd = 0
        for EID, LINK, CONT, STATUS in tasks:
            logger1.debug("任务的事件ID→%s; 对应链接→%s; 内容→%s; 状态号→%s" % (EID, LINK, CONT, STATUS))
            cmd = None  # simpleTest.简单应答器(Curs_APP, Curs_CQ, STATUS, LINK, EID, CONT)
            # TODO: 打算后端应用组做好之后这里再加一些调用机制什么的. 比如根据配置通过线程的方式运行某个后端程序, 这个后端程序有着自己独立的数据库/数据处理机制, 去分析和处理酷Q自身的日志再返回命令, 中间层将应用返回的命令插入前端的数据库并通知前端处理.
            if cmd != None:
                logger1.debug("应用返回了命令→%s" % (cmd))
                query_insert = "INSERT INTO `main`.`event` (`TYPE`, `CONT`, `STATUS`) VALUES (5233, '?', 233);"
                Conn_APP.execute(query_insert, cmd)
                count_cmd += 1
        Conn_APP.commit()
        pass  # 这里写要做的处理
    logger1.info("任务处理结束, 继续睡觉...")
    if count_cmd > 0:
        logger1.debug("置CQTigger通知前端取命令...")
        win32api.PostMessage(CQwHandle, WM_USER, AppSync, 0x32)
        G_CommuCounter += 1
        Time_LastCommu = time.time()
        MWnd.s_signal_in.setChecked(True)
        MWnd.displayTimer.start(300)
    Conn_APP.close()
    Conn_CQ.close()
    return 0

def getNewTasks(Conn_APP):
    query_selectTasks = "SELECT `EID`, `LINK`, `CONT`, `STATUS` FROM `main`.`event` WHERE `STATUS` > 266 ORDER BY `STATUS` DESC;"
    curs = Conn_APP.execute(query_selectTasks)
    reslut = curs.fetchall()
    rows = len(reslut)
    if rows > 0:
        return reslut
    return 0


Status_Hint = "等待前端握手..."
logger1.info("中间层服务准备就绪, 开始等待前端握手消息...")


def waitForStart():
    global Status_Hint, CQwHandle, Time_G_Start, Time_LastError, G_CommuCounter, Time_LastCommu
    for _ in range(int(Timeout)):  # 不严格的大致超时限制
        CQwHandle = win32gui.FindWindow(None, CQTitle)
        if CQwHandle > 0:
            logger1.debug("找到CQ端通信窗口, 发送握手消息...")
            win32api.PostMessage(CQwHandle, WM_USER, 0x200, 0x11)
            G_CommuCounter += 1
            Time_LastCommu =time.time()
            Time_G_Start = time.time()
            time.sleep(3.5)
            break
        time.sleep(1e-3)
    if WPA == HandSk or WPA == Breath:
        Status_Hint = "等待新任务中..."
        logger1.info("中间层服务启动完成")
    else:
        Status_Hint = "握手失败, 等待处理"
        logger1.warning("找到了前端通信窗口, 但未得到期望的回应.")
        Time_LastError = time.time()
    if CQwHandle < 1:
        logger1.error("等待超时, 请确保前端正确运行")
        logger1.info("**********未启动前端应用, 程序退出************")
        sys.exit(404)
    logger1.debug(f"握手结果: {WPA}, {LPA}")


waitS = ThreadWithReturn(target=waitForStart, args=())
waitS.start()

def scheduleCallback():
    global G_CommuCounter, Time_LastCommu
    win32api.PostMessage(CQwHandle, WM_USER, AppSync, 0x11)
    G_CommuCounter += 1
    Time_LastCommu = time.time()
    MWnd.s_signal_out.setChecked(True)


# TODO: 在这加入后端应用的调用过程
schedules = scheduleCast.Schedule(callBack=scheduleCallback, printer=printc, file=PATH_DB)
schedules.add("everyWorkingHours:090030", target="1837107998", content="整点偷揉~")
_tmp_MSGstr = "亲爱的群友们早上好呀~ 今天是农历|_|, 又是充满希望的一天呢~"
schedules.add("everyWorkingdayTime:063030", "ChineseCalendar.getLunar_today()", target="483537882", msgType="grp", content=_tmp_MSGstr)
del _tmp_MSGstr
schedules.add("everyWorkend:083030", target="483537882", msgType="grp", content="亲爱的群友们周末快乐呀~")
schedules.add("everyWorkend:193030", target="483537882", msgType="grp", content="亲爱的群友们晚上好呀~ 大家今天都做了些啥呢? 分享分享呗~")
schedules.start()

rc = app.exec_()
scheduleCast.Schedule.G_RUN = False
logger1.info("**********程序退出************")
os.system("pause")
sys.exit(rc)
















