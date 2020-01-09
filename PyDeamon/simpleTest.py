# 只是一个随意的示例...
import time

AdminQQ = "2154055060" # 打算做成用配置文件的, 以后再说吧...

def 简单应答器(Curs_APP, Curs_CQ, STATUS, LINK, EID, CONT):
    cmd = ""
    # 因为可能存在前端存了指令而后端没处理过的情况, 这种情况就直接忽略不再处理
    query_update = "UPDATE `main`.`event` SET `NOTE`= '忽略重复内容', `STATUS` = 500 WHERE `EID` < ? AND `CONT` = ? AND `STATUS` BETWEEN 233 AND 500 ;" 
    Curs_APP.execute(query_update, (EID, CONT))

    if LINK > 0:
        query_oriGrp = "SELECT `group` FROM `main`.`event` WHERE `id` = ?;"
        curs = Curs_CQ.execute(query_oriGrp, LINK)
        oriGrp = curs.fetchone()[0][9:]
        query_oriMSG = "SELECT `content` FROM `main`.`event` WHERE `id` = ?;"
        curs = Curs_CQ.execute(query_oriMSG, LINK)
        oriMSG = curs.fetchone()[0]

    if STATUS == 401:
        cmd = "send msg to" + AdminQQ + "?(" + str(LINK) + ")\n" + CONT + "?"
        query_update = "UPDATE `main`.`event` SET `CONT`= ?, `STATUS` = 233 WHERE `EID` = ?;"
        Curs_APP.execute(query_update, (cmd, EID))

    if STATUS == 302:
        query_update = "UPDATE `main`.`event` SET `NOTE`= '没做, 暂时先直接标记为已处理吧...', `STATUS` = 0 WHERE `EID` = ?;"
        Curs_APP.execute(query_update, EID)
        pass # 暂时没条件做那么多了, 先放着咕咕咕...
    if STATUS == 301:
        query_update = "UPDATE `main`.`event` SET `NOTE`= '没做, 暂时先直接标记为已处理吧...', `STATUS` = 0 WHERE `EID` = ?;"
        Curs_APP.execute(query_update, EID)
        pass
    if STATUS == 300:
        if oriMSG.find("蹲坑去了") > -1:
            cmd = 蹲坑计时(0, oriGrp)
            query_insert = "INSERT INTO `main`.`event` (`TYPE`, `CONT`, `STATUS`) VALUES (5234, '去蹲坑了', 266);"
            Curs_APP.execute(query_insert)
        if oriMSG.find("蹲坑回来了") > -1:
            query_select = "SELECT `EID`, `TIME`, `CONT`, `STATUS` FROM `main`.`event` WHERE `STATUS` = 266 AND `CONT` = '去蹲坑了' ORDER by `EID` DESC ;" # 可能会有多条未处理的"去蹲坑了", 只取最后一次, 下面处理完之后把所有的都置已处理
            curs = Curs_APP.execute(query_select)
            recordTime = curs.fetchone()
            if recordTime != None:
                recordTime = recordTime[1] # 0是EID, 1是时间
                tmp_timeArray = time.strptime(recordTime, "%y-%m-%d %H:%M:%S")
                recordTimeStamp = int(time.mktime(tmp_timeArray)) + 8 * 3600
                duration = time.time() - recordTimeStamp;
                cmd = 蹲坑计时(duration, oriGrp)
                query_update = "UPDATE `main`.`event` SET `NOTE`= '已处理', `STATUS` = 0 WHERE `STATUS` = 266 AND `CONT` = '去蹲坑了' ;"
                Curs_APP.execute(query_update)
            else:
                cmd = "send grp %s ?嗯？啥时候去蹲的？这里没记录....?" %  oriGrp 
        query_update = "UPDATE `main`.`event` SET `NOTE`= '已处理', `STATUS` = 0 WHERE `EID` = " + str(EID) + ";"
        Curs_APP.execute(query_update)

    if len(cmd) > 1:
        return cmd
    else :
        return None

def 蹲坑计时(dur, oriGrp):
    if dur < 1:
        cmd = "send grp %s ?开始计时, 现在时间是 %s ?" % ( oriGrp, time.strftime("%H:%M:%S", time.localtime())  )
    else :
        if dur > 60:
            cmd = "send grp %s ?计时结束, 蹲了 %d 分 %d 秒?" % ( oriGrp, dur / 60, dur % 60 )
        else:
            cmd = "嗯..."
    return cmd