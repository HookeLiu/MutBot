# 中间层: 配置和控制前端和后端
import win32event 
import os
import sys
import time, datetime
import threading
import sqlite3

import logging
logFile = "./debug.log"
logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
                    datefmt='%a, %d %b %Y %H:%M:%S',
                    filename=logFile,
                    filemode='a+')

import simpleTest # 打算以后做成读配置文件自动导入后端应用, 就像酷Q那样

AdminQQ = "2154055060" # 打算做成用配置文件的, 以后再说吧...

Flag_exit = False

logging.info("--------程序启动--------")
logging.debug("程序启动, 开始监听前端事件")
# 先创建和打开用到的win32event
TriggerEvent = "Global\\CQdbSync" 
UpdatedEvent = "Global\\CQupdate" 
ExitEvent    = "Global\\CQexit"   

try:
    CQTigger  = win32event.OpenEvent(win32event.EVENT_ALL_ACCESS , True , TriggerEvent )
    CQupdate  = win32event.OpenEvent(win32event.EVENT_ALL_ACCESS , True , UpdatedEvent )
    CQexit    = win32event.OpenEvent(win32event.EVENT_ALL_ACCESS , True , ExitEvent )
    logging.debug("win32event打开成功, 程序开始监听")
except:
    logging.error("win32event打开失败, 可能是前端未正确运行.")
    logging.info("**********程序退出************")
    sys.exit(-233)

win32event.SetEvent(CQupdate) # 程序启动后给前端一个反馈以便前端判断后端状态gitgit 

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
                logging.error("找到了过多的文件, 请检查运行环境")
                break
        
logging.debug( "找到了%d个db文件, 取最后两个( %s, %s ), 耗时%.3f秒" %(count_files, PATH_DB[-2], PATH_DB[-1], time.time() - Time_start ) )
        
# 用于触发酷Q的事件就在操作完成时SetEvent, 另外两个则需要开启两个线程等待酷Q触发

# 酷Q发出退出信号之后中间层应该保证后端都退出并随后退出
def Gexit():
    global Flag_exit
    waitFlag = 233
    while waitFlag != win32event.WAIT_OBJECT_0: 
        waitFlag = win32event.WaitForSingleObject( CQexit, 60000 ) # 虽然需要它一直等待, 但设置超时能防止无法退出, 超时了再重新等待就好了
        if waitFlag == win32event.WAIT_OBJECT_0 :
            logging.info("收到退出信号, 程序准备结束并退出")
            Flag_exit = True
            pass # 这里放程序退出逻辑
    return 0

# 酷Q发出更新信号时说明有需要立即处理的事件, 中间层需要做预处理或移交给后端
def Update():
    global Flag_exit
    waitFlag = 233
    while Flag_exit != True: 
        waitFlag = win32event.WaitForSingleObject( CQupdate, 60000 )
        if waitFlag == win32event.WAIT_OBJECT_0 :
            logging.info("收到更新信号, 开始后台处理...")
            try:
                Conn_APP = sqlite3.connect(PATH_DB[-2])
                Coon_CQ  = sqlite3.connect(PATH_DB[-1])
                Curs_APP = Conn_APP.cursor()
                Curs_CQ  = Coon_CQ.cursor()
            except OperationalError :
                logging.error("数据库打开失败")
                logging.info("**********程序退出************")
                sys.exit(-233)
            except NameError :
                logging.error("数据库游标创建失败")
                logging.info("**********程序退出************")
                sys.exit(-233)
            tasks = getNewTasks()
            if tasks == 0:
                return
            for EID, LINK, CONT, STATUS in tasks:
                cmd = simpleTest.应用选择器(STATUS, LINK, EID)
                query_insert = "INSERT INTO `main`.`event` (`TYPE`, `CONT`, `STATUS`) VALUES (5233, '" + cmd + "', 233);"
                Curs_APP.execute(query_insert)
            pass # 这里写要做的处理
            Curs_APP = None
            Conn_APP.close()
            Curs_CQ = None
            Coon_CQ.close()
    return



def getNewTasks():
    query_selectTasks = "SELECT `EID`, `LINK`, `CONT`, `STATUS` FROM `main`.`event` WHERE `STATUS` > 233 ORDER BY `STATUS` DESC;"
    curs = Curs_APP.execute(query_selectTasks)
    reslut = curs.fetchall()
    rows = len(reslut)
    if rows > 0 :
        return reslut
    return 0
          
Thread_waitForExit = threading.Thread(target = Gexit, args = ())
Thread_waitForTigg = threading.Thread(target = Update, args = ())

Thread_waitForExit.start()
Thread_waitForTigg.start()

Thread_waitForTigg.join()
Thread_waitForExit.join()
logging.info("**********程序退出************")
sys.exit(0)