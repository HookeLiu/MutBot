# 中间层: 配置和控制前端和后端
import win32event 
import os
import sys
import time
import threading
import sqlite3

import simpleTest # 打算以后做成读配置文件自动导入后端应用, 就像酷Q那样

AdminQQ = "2154055060" # 打算做成用配置文件的, 以后再说吧...

Flag_exit = False


# 因为目前的设计是整个框架都使用数据库传递和记录数据, 所以这个中间层程序需要打开两个数据库: 前端app的db和酷Q自己的eventv2.db
# 不确定主程序和这个脚本在不在同一个目录, 又想要让各个模块间独立性尽可能强, 所以就搞成找文件的了... 也许这里以后得重新设计...
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
        
Conn_APP = sqlite3.connect(PATH_DB[-2])
Coon_CQ  = sqlite3.connect(PATH_DB[-1])
Curs_APP = Conn_APP.cursor()
Curs_CQ  = Coon_CQ.cursor()

# 先创建和打开用到的win32event
TriggerEvent = "Global\\CQdbSync" 
UpdatedEvent = "Global\\CQupdate" 
ExitEvent    = "Global\\CQexit"   

CQtrigger = win32event.CreateEvent(None, False, False, TriggerEvent )
time.sleep(10)
CQupdate  = win32event.OpenEvent(win32event.EVENT_ALL_ACCESS , True , UpdatedEvent )
CQexit    = win32event.OpenEvent(win32event.EVENT_ALL_ACCESS , True , ExitEvent )
        
# 用于触发酷Q的事件就在操作完成时SetEvent, 另外两个则需要开启两个线程等待酷Q触发

# 酷Q发出退出信号之后中间层应该保证后端都退出并随后退出
def Gexit():
    global Flag_exit
    waitFlag = 233
    while waitFlag != win32event.WAIT_OBJECT_0: 
        waitFlag = win32event.WaitForSingleObject( CQexit, 60000 ) # 虽然需要它一直等待, 但设置超时能防止无法退出, 超时了再重新等待就好了
        if waitFlag == win32event.WAIT_OBJECT_0 :
            Flag_exit = True
            pass # 这里放程序退出逻辑
    return

# 酷Q发出更新信号时说明有需要立即处理的事件, 中间层需要做预处理或移交给后端
def Update():
    global Flag_exit
    waitFlag = 233
    while Flag_exit != True: 
        waitFlag = win32event.WaitForSingleObject( CQupdate, 60000 )
        if waitFlag == win32event.WAIT_OBJECT_0 :
            tasks = getNewTasks()
            if tasks == 0:
                return
            for EID, LINK, CONT, STATUS in reslut:
                if STATUS == 401:
                    cmd = "send msg to" + AdminQQ + "?(" + str(LINK) + ")\n" + CONT + "?"
                    query_update = "UPDATE `main`.`event` SET `CONT`= '" + cmd + "', `STATUS` = 233 WHERE `EID` = " + str(EID) + ";"
                    curs = Curs_CQ.execute(query_update)
                if STATUS == 302:
                    pass # 暂时管不了那么多了...先实现基础功能demo好了...
                if STATUS == 301:
                    #query_oriMSG = "SELECT `content` FROM `main`.`event` WHERE `id` = " + LINK + ";"
                    #curs = Curs_CQ.execute(query_oriMSG)
                    #oriMSG = curs.fetchone()[0]
                    pass
                if STATUS == 300:
                    sta = simpleTest.蹲坑计时()
                    query_oriGrp = "SELECT `group` FROM `main`.`event` WHERE `id` = " + str(LINK) + ";"
                    curs = Curs_CQ.execute(query_oriGrp)
                    oriGrp = curs.fetchone()[0][9:]
                    if sta < 1:
                        cmd = "send grp to %s ?开始计时, 现在时间是 %s ?" % ( oriGrp, time.strftime("%H:%M:%S", time.localtime())  )
                    else :
                        cmd = "send grp to %s ?计时结束, 蹲了 %s 秒?" % ( oriGrp, sta )
                    query_insert = "INSERT INTO `main`.`event` (`TYPE`, `CONT`, `STATUS`) VALUES (5233, '" + cmd + "', 233);"
            pass # 这里写要做的处理
    return



def getNewTasks():
    query_selectTasks = "SELECT `EID`, `LINK`, `CONT`, `STATUS` FROM `main`.`event` ORDER BY `STATUS` DESC;"
    curs = Curs_APP.execute(query_selectTasks)
    reslut = curs.fetchall()
    rows = len(reslut)
    if rows > 0 :
        return reslut
    return 0;
          
Thread_waitForExit = threading.Thread(target = Gexit, args = ())
Thread_waitForTigg = threading.Thread(target = Update, args = ())

Thread_waitForExit.start()
Thread_waitForTigg.start()

Thread_waitForExit.join()