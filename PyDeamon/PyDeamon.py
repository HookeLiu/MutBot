# 中间层: 配置和控制前端和后端
import win32event 
import os
import sys
import time, datetime
import threading
import sqlite3

os.system("title 中间层服务")

import logging
logFile = "./debug.log"
logFormat =  logging.Formatter('[%(levelname)s] | %(asctime)s @%(relativeCreated)d | "%(filename)s", line %(lineno)d, in `%(threadName)s` : %(message)s', '%d %b %Y %H:%M:%S')
printFormat = logging.Formatter('%(asctime)s : %(message)s')
fh = logging.FileHandler(logFile)
fh.setFormatter(logFormat)
ch = logging.StreamHandler() 
ch.setFormatter(printFormat)
# ch.setLevel(logging.INFO)
ch.setLevel(logging.DEBUG)

logger0 = logging.getLogger("all") # 根记录器把所有内容记录到文件
logger0.setLevel(logging.DEBUG)
logger0.addHandler(fh) 

logger1 = logging.getLogger("all.consleLog") # 控制台记录器的过滤器只显示info以上的, 所有记录会交到上层
logger1.addHandler(ch) 

logger1.info("--------程序开始运行--------")

import simpleTest # 打算以后做成读配置文件自动导入后端应用, 就像酷Q那样

AdminQQ = "2154055060" # 打算做成用配置文件的, 以后再说吧...

Flag_exit = False

logger1.debug("应用初始化完成, 准备创建通信事件")
# 先创建和打开用到的win32event
TriggerEvent = "Global\\CQdbSync" 
UpdatedEvent = "Global\\CQupdate" 
ExitEvent    = "Global\\CQexit"   

try:
    CQTigger  = win32event.CreateEvent(None, False, False, TriggerEvent )
    CQupdate  = win32event.CreateEvent(None, False, False, UpdatedEvent )
    CQexit    = win32event.CreateEvent(None, False, False, ExitEvent )
    logger1.debug("win32event创建成功, 程序开始通信")
except:
    logger1.error("win32event打开失败, 可能是前端未正确运行.")
    logger1.info("**********程序退出************")
    sys.exit(-233)

# 初始化一下, 避免之前残留触发的影响
win32event.ResetEvent(CQexit)
win32event.ResetEvent(CQupdate)
win32event.ResetEvent(CQTigger)

win32event.SetEvent(CQupdate) # 程序启动后给前端一个反馈以便前端判断后端状态

# 因为目前的设计是整个框架都使用数据库传递和记录数据, 所以这个中间层程序需要打开两个数据库: 前端app的db和酷Q自己的eventv2.db
# 不确定主程序和这个脚本在不在同一个目录, 又想要让各个模块间独立性尽可能强, 所以就搞成找文件的了... 也许这里以后得重新设计...
Time_start = time.time()
Path_script_root = os.path.abspath('.')[:3]
DB_CQ = "eventv2.db"
DB_APP = "app.db"
PATH_DB = []
count_files = 0
for path, folder, files in os.walk(Path_script_root):
    for file in files:
        if (file == DB_CQ and path.find(AdminQQ) > -1 ) or (file == DB_APP and path.find("cn.17ds8.AItest") > -1 ):
            path = os.path.join(path, file)
            PATH_DB.append(path)
            count_files += 1
            if count_files > 5:
                logger1.error("找到了过多的文件, 请检查运行环境")
                break

if count_files < 2 :
    logger1.critical("找不到数据库! 请确保运行环境正常. 如果是初次使用, 请尝试先用前端创建数据表.")
    logger1.info("**********程序退出************")
    sys.exit(-233)

        
logger1.debug( "找到了%d个db文件, 取最后两个( %s, %s ), 耗时%.3f秒" %(count_files, PATH_DB[-2], PATH_DB[-1], time.time() - Time_start ) )
        
# 用于触发酷Q的事件就在操作完成时SetEvent, 另外两个则需要开启两个线程等待酷Q触发

# 酷Q发出退出信号之后中间层应该保证后端都退出并随后退出
def Gexit():
    global Flag_exit
    waitFlag = 233
    while waitFlag != win32event.WAIT_OBJECT_0: 
        waitFlag = win32event.WaitForSingleObject( CQexit, 60000 ) # 虽然需要它一直等待, 但设置超时能防止无法退出, 超时了再重新等待就好了
        if waitFlag == win32event.WAIT_OBJECT_0 :
            logger1.info("收到退出信号, 程序准备结束并退出")
            Flag_exit = True
            pass # 这里放程序退出逻辑
    return 0

# 酷Q发出更新信号时说明有需要立即处理的事件, 中间层需要做预处理或移交给后端
def Update():
    global Flag_exit
    waitFlag = 233
    while Flag_exit != True: 
        waitFlag = win32event.WaitForSingleObject( CQupdate, 1000 )
        if waitFlag == win32event.WAIT_OBJECT_0 :
            logger1.info("收到更新信号, 开始后台处理...")
            try:
                Conn_APP = sqlite3.connect(PATH_DB[-1])
                Coon_CQ  = sqlite3.connect(PATH_DB[-2])
                Curs_APP = Conn_APP.cursor()
                Curs_CQ  = Coon_CQ.cursor()
            except OperationalError :
                logger1.error("数据库打开失败")
                logger1.info("**********程序退出************")
                sys.exit(-233)
            except NameError :
                logger1.error("数据库游标创建失败")
                logger1.info("**********程序退出************")
                sys.exit(-233)
            tasks = getNewTasks(Curs_CQ)
            if tasks == 0:
                return
            else :
                logger1.info("找到%s个任务, 开始处理..." % len(tasks))
                count_cmd = 0
                for EID, LINK, CONT, STATUS in tasks:
                    logger1.debug("任务的事件ID→%s; 对应链接→%s; 内容→%s; 状态号→%s" %(EID, LINK, CONT, STATUS) )
                    cmd = simpleTest.应用选择器(Curs_APP, Curs_CQ, STATUS, LINK, EID, CONT)
                    if cmd != None:
                        query_insert = "INSERT INTO `main`.`event` (`TYPE`, `CONT`, `STATUS`) VALUES (5233, '" + cmd + "', 233);"
                        Curs_APP.execute(query_insert)
                        logger1.debug("中间层返回了命令→%s" %(cmd) )
                        count_cmd += 1
                pass # 这里写要做的处理
            logger1.info("任务处理结束, 继续睡觉...")
            Coon_APP.commit()
            if count_cmd > 0:
                logger1.debug("置CQTigger通知前端取命令...")
                win32event.SetEvent(CQTigger)
            Curs_APP = None
            Conn_APP.close()
            Coon_CQ.commit()
            Curs_CQ = None
            Coon_CQ.close()
            win32event.ResetEvent(CQupdate)
    return

def getNewTasks(Curs_APP):
    query_selectTasks = "SELECT `EID`, `LINK`, `CONT`, `STATUS` FROM `main`.`event` WHERE `STATUS` > 266 ORDER BY `STATUS` DESC;"
    curs = Curs_APP.execute(query_selectTasks)
    reslut = curs.fetchall()
    rows = len(reslut)
    if rows > 0 :
        return reslut
    return 0
     
logger1.debug("为了方便通知前端后端程序在线, 先睡30秒")
time.sleep(30)
win32event.ResetEvent(CQupdate)
logger1.debug("开始创建退出检测线程和任务处理线程")
Thread_waitForExit = threading.Thread(target = Gexit, args = ())
Thread_waitForTigg = threading.Thread(target = Update, args = ())

Thread_waitForExit.start()
Thread_waitForTigg.start()

Thread_waitForTigg.join()
Thread_waitForExit.join()
logger1.info("**********程序退出************")
sys.exit(0)