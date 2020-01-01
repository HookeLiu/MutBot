# 只是一个随意的示例...
import time

词库 = {
    "名词":{
            "人名":["龙狼", "狗狗"],
            "地名":["厕所", "卫生间", "坑"],
            },
    "动词":{
            "行为":["蹲", "起", "去", "回"],
        },
    "否定":{"一般否定":["不", "没"]},
    "疑问":{"疑问结尾":["吗", "?", "？"]},
}
语料模板 = [
    "龙狼去蹲坑了", "龙狼蹲坑回来了", "龙狼去厕所了", "龙狼在蹲坑吗"
]

def 应用选择器(Curs_APP, Curs_CQ, STATUS, LINK, EID, CONT):
    cmd = ""
    if LINK > 0:
        query_oriGrp = "SELECT `group` FROM `main`.`event` WHERE `id` = " + str(LINK) + ";"
        curs = Curs_CQ.execute(query_oriGrp)
        oriGrp = curs.fetchone()[0][9:]
        query_oriMSG = "SELECT `content` FROM `main`.`event` WHERE `id` = " + str(LINK) + ";"
        curs = Curs_CQ.execute(query_oriMSG)
        oriMSG = curs.fetchone()[0]

    if STATUS == 401:
        cmd = "send msg to" + AdminQQ + "?(" + str(LINK) + ")\n" + CONT + "?"
        query_update = "UPDATE `main`.`event` SET `CONT`= '" + cmd + "', `STATUS` = 233 WHERE `EID` = " + str(EID) + ";"
        Curs_APP.execute(query_update)

    if STATUS == 302:
        pass # 暂时没条件做那么多了, 先放着咕咕咕...
    if STATUS == 301:
        pass
    if STATUS == 300:
        if CONT.find("龙狼蹲坑去了") > 0:
            cmd = 蹲坑计时(0, oriGrp)
            query_insert = "INSERT INTO `main`.`event` (`TYPE`, `CONT`, `STATUS`) VALUES (5234, '龙狼去蹲坑了', 266);"
            Curs_APP.execute(query_insert)
            Coon_APP.commit()
            return cmd
        if CONT.find("龙狼蹲坑回来了") > 0:
            query_select = "SELECT `EID`, `TIME`, `CONT`, `STATUS` FROM `main`.`event` WHERE `STATUS` = 266 AND `CONT` = '龙狼去蹲坑了' ;"
            curs = Curs_APP.execute(query_select)
            recordTime = curs.fetchone()[1]
            tmp_timeArray = time.strptime(recordTime, "%Y-%m-%d %H:%M:%S")
            recordTimeStamp = int(time.mktime(timeArray)) + 8 * 3600
            duration = time.time() - recordTimeStamp;
            cmd = 蹲坑计时(duration, oriGrp)
        pass
    if len(cmd) > 1:
        return cmd
    else :
        return None



def 蹲坑计时(dur, oriGrp):
    if dur < 1:
        cmd = "send grp to %s ?开始计时, 现在时间是 %s ?" % ( oriGrp, time.strftime("%H:%M:%S", time.localtime())  )
    else :
        cmd = "send grp to %s ?计时结束, 蹲了 %s 秒?" % ( oriGrp, sta )
    return cmd