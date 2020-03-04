"""
计划消息
后端应用示例
按定时或环境条件发送群消息或者私聊消息
By 佚之狗 2020Feb24
"""

import time
import threading
import json
import requests
import ChineseCalendar
import sqlite3
import random

GroupsID = ["483537882"]  # 所有欲处理的群的群号
Events = {
    "onDateTime":
        lambda trgTime, nowStamp, skip: True if trgTime == nowStamp else False,
    "everyWorkingdayTime":
        lambda trgTime, nowStamp, skip: True if (Schedule.env_today['dayType'] == '0') and ((nowStamp - trgTime) % (86400 * (1 + skip)) == 0) and (nowStamp >= trgTime) else False,
    "everyWorkend":
        lambda trgTime, nowStamp, skip: True if (Schedule.env_today['dayType'] == '1') and ((nowStamp - trgTime) % (86400 * (1 + skip)) == 0) else False,
    "everyHours":
        lambda trgTime, nowStamp, skip: True if ((nowStamp - trgTime) % (3600 * (1 + skip)) == 0) and (nowStamp >= trgTime) else False,
    "everyMins":
        lambda trgTime, nowStamp, skip: True if ((nowStamp - trgTime) % (60 * (1 + skip)) == 0) and (nowStamp >= trgTime) else False,
    "everyWorkingHours":
        lambda trgTime, nowStamp, skip: True if (Schedule.env_today['dayType'] == '0') and (time.localtime(nowStamp)[3] in [8, 9, 10, 11, 14, 15, 16, 17, 18, 19, 20]) and ((nowStamp - trgTime) % (3600 * (1 + skip)) == 0) else False,
    "everyday":
        lambda trgTime, nowStamp, skip: True if ((nowStamp - trgTime) % (86400 * (1 + skip)) == 0) and (nowStamp >= trgTime) else False,
}
DayType = {'0': "工作日", '1': "周末", '2': "节假日", '3': "未知"}
dateServer_url = "http://www.easybots.cn/api/holiday.php?d="

def getDayType(timeStamp):  # 调用一个在线查询的api来确定某天的工作日类型
    date = time.strftime('%Y%m%d', time.localtime(timeStamp))
    retryLimit = 3
    dtype = "3"
    for t in range(retryLimit):
        try:
            req = requests.get(dateServer_url + date)
            vop_data = json.loads(req.text)
            dtype = vop_data[date]
            print("---debug--- 连接远程服务器查询到`%s→%s`" % (date, dtype), end="<br />\n")
            break
        except Exception as errInfo:
            print("---debug--- 查询服务器请求失败: %s" % errInfo, end="<br />\n")
            print(f"---debug--- 尝试第{t+1}次重试...", end="<br />\n")
    if dtype == '3':
        print("---Warning--- 查询工作日类型失败", end="<br />\n")
    return dtype


# `计划消息` -- 后端应用示例 -- 按定时或环境条件发送群消息或者私聊消息
class Schedule:
    env_today = {'dayType': getDayType(time.time()), 'lastUpdate': time.time()}
    lastTrigger = None
    triggerCount = 0
    G_RUN = True
    sQueue = {}
    idCount = 0
    file = ""
    _started = False

    def __init__(self, callBack=None, printer=print, file=""):
        self.id = 0
        self.time_nowStamp = int(time.time())
        self.enabled = False
        self.time_start = time.time()
        self.printer = printer
        if callBack is not None:
            self.callBack = callBack
        if type(file) == list:
            Schedule.file = file
        self.scheduleThread = threading.Thread(target=self.polling, args=())

    def CQexec(self, cmdStr: str):
        self.printer("[CQ]&gt;&gt;&gt; %s" % cmdStr)
        if len(Schedule.file) > 1:
            try:
                Conn_APP = sqlite3.connect(Schedule.file[-1])
            except Exception as errInfo:
                self.printer("定时消息执行--`APP`数据库连接失败: %s" % errInfo)
                return -233
            try:
                query_insert = "INSERT INTO `main`.`event` (`TYPE`, `CONT`, `STATUS`) VALUES (5233, ?, 233);"
                Conn_APP.execute(query_insert, (cmdStr,))
                Conn_APP.commit()
            except Exception as errInfo:
                self.printer("定时消息执行--通信失败: %s" % errInfo)
                return -233

    def polling(self):
        time.sleep(10)  # 等应用启动之后再开始
        while self.enabled and Schedule.G_RUN:
            self.time_nowStamp = int(time.time())
            if time.localtime()[-2] != time.localtime(Schedule.env_today['lastUpdate'])[-2]:  # 这说明跨天了, 就得更新环境信息. 其中, 索引-2是"tm_yday"
                Schedule.env_today['dayType'] = getDayType(time.time())
                Schedule.env_today['lastUpdate'] = time.time()
            for sch in Schedule.sQueue.items():
                if len(sch[1]) < 3:  # 说明这个任务被删了
                    continue
                condict = Events[sch[1][0]]
                trigger = bool(condict(sch[1][1], self.time_nowStamp, sch[1][4]))
                if trigger:
                    if sch[1][5] is not None:  # 说明这是个计次任务
                        if sch[1][5] < 1:  # 计次完了就删除
                            Schedule.sQueue[sch[0]] = [[b for b in sch[1]]]
                            continue
                        else:
                            sch[1][5] = sch[1][5] - 1
                    _tmp_content = sch[1][2]
                    if len(sch[1][3]) > 0:  # 说明这个任务的消息有附加动态参数:
                        for item in sch[1][3]:
                            if _tmp_content.find("|_|") < 1:
                                break
                            _tmp_content = _tmp_content.replace("|_|", str(eval(item)), 1)
                    self.CQexec(_tmp_content)
                    del _tmp_content
                    Schedule.lastTrigger = time.time()
                    Schedule.triggerCount += 1
                    if sch[1][0].find("on") > -1:  # 说明这是单次任务, 执行后就可以删除了
                        Schedule.sQueue[sch[0]] = [[b for b in sch[1]]]
                    if sch[1][6] is not None:  # 说明这个任务有自己的callBack
                        sch[1][6]()
                    self.callBack()
                    time.sleep(1)
            time.sleep(0.1)
        self.stop()
        return 0

    def start(self):
        if not Schedule._started:
            Schedule._started = True
            self.enabled = True
            self.printer(f"---{time.strftime('%X', time.localtime())} debug--- 计划将于10秒后开始执行...")
            self.scheduleThread.start()
        else:
            self.printer("计划只能启动一次")

    def stop(self):
        self.enabled = False

    def join(self, DurationSec: float = None):
        self.scheduleThread.join(DurationSec)
        self.stop()

    def showALL(self):
        self.printer(Schedule.sQueue)

    def show(self):
        tmp = {}
        for item in self.sQueue.items():
            if item[1] == 3:
                tmp[item[0]] = item[1]
        self.printer(tmp)

    def delete(self, queueID: int):
        self.sQueue[queueID] = [[b for b in Schedule.sQueue[queueID]]]

    def add(self, schedule: str, *args, action: str = "send", target: str = "616471607", msgType: str = "msg", content: str = "", sikp: int = 0, count: int = None, callBack = None, operator: str = "to"):
        Schedule.idCount += 1
        self.id = Schedule.idCount
        _tmp_content = " ?" + content + "?" if content != "" else ""
        _tmp_CQcmd = action + " " + msgType + " " + operator + " " + target + str(_tmp_content)
        del _tmp_content
        tmp_scheduleArr = schedule.split(":")
        _tmp_triggerEvent = tmp_scheduleArr[0]
        if len(tmp_scheduleArr) > 1:
            if len(tmp_scheduleArr[1]) > 7:
                _tmp_triggerTime = int(time.mktime(time.strptime(tmp_scheduleArr[1], "%Y%m%d %H%M%S")))
            else:
                tmp_prefix = time.strftime("%Y%m%d ", time.localtime()) + tmp_scheduleArr[1]
                _tmp_triggerTime = int(time.mktime(time.strptime(tmp_prefix, "%Y%m%d %H%M%S")))
                del tmp_prefix
        else:
            _tmp_triggerTime = int(time.time() + 10)
        del tmp_scheduleArr
        _tmp_existFlag = False
        for sch in Schedule.sQueue.values():
            if (sch[0] == _tmp_triggerEvent) and (sch[1] == _tmp_triggerTime or _tmp_triggerTime == int(time.time() + 10)) and (sch[2] == _tmp_CQcmd):
                self.printer("创建第%d个任务时错误: 已存在相同的计划`%s:%s→*%s*`, 跳过." %(self.id, _tmp_triggerEvent, _tmp_triggerTime, _tmp_CQcmd))
                _tmp_existFlag = True
                break
        if not _tmp_existFlag:
            if _tmp_triggerEvent in Events.keys():
                Schedule.sQueue[self.id] = [_tmp_triggerEvent, _tmp_triggerTime, _tmp_CQcmd, args, sikp, count, callBack]
                self.printer("第%d个计划`%s:%s→*%s*`添加成功" % (self.id, _tmp_triggerEvent, _tmp_triggerTime, _tmp_CQcmd))
            else:
                self.printer("!!!Warning: %s不在预设事件中. 跳过..." % _tmp_triggerEvent)
        del _tmp_existFlag
        del _tmp_CQcmd
        del _tmp_triggerEvent
        del _tmp_triggerTime


if __name__ == "__main__":
    def test():
        print(f"{'-'*10}{time.strftime('%X', time.localtime())} --- 第{Schedule.triggerCount}次触发{'-'*10}")

    def test2():
        print(ChineseCalendar.getLunar_today())

    print(ChineseCalendar.getLunar_today())

    sch = Schedule(callBack=test)
    sch.add("everyMins", msgType="like", sikp=1)
    sch.add("everyMins:110358", "time.strftime('%X', time.localtime())", "random.random()", content="~test----|_|-~-|_|~~", )
    sch.add("everyMins", msgType="like")
    sch.add("everyMins:110425", target="2139223150", content="这太难了...", count=3)
    sch.add("everyMins:110555", action="get", msgType="env", count=2, callBack=test2)
    sch.showALL()
    sch.start()












