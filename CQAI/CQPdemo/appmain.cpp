/*
*	17ds8多功能AI的QQ接口. 选用的是酷Q官方SDK的VC++版本, 因为这个版本的封装最少最接近底层, API和结构最清晰. 没有选用易语言的一个原因是难以做通用开发(关键原因是易语言要付费且界面累赘, 个人也不习惯易语言那种汉化和命名方式)
*	这个多功能AI项目的本体在于后端处理程序,
*	这个VC++工程基本只是在酷Q官方给的SDK上做一个简易的数据接口, 作为后端AI程序和QQ用户的桥梁.
*	所以基本上就是clone过来加了点内容, 也没啥改动. 后端主要是用Python, 消息处理主体依靠神经网络.
*	(需要注意的是, 酷Q和这个工程用的都是GB2312, 数据库以及后端程序用的都是UTF-8, 
*	所以这里用了个网上抄来的简易转码函数).
*	By 佚之狗 Dec. 2019
*/
/*
* CoolQ Demo for VC++ 
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/

#include "stdafx.h"
#include "string"
#include "cqp.h"
#include "appmain.h" //应用AppID等信息，请正确填写，否则酷Q可能无法加载

#define OK  0
#define Err 1

#pragma comment(lib,"Winmm.lib")

using namespace std;

sqlite3* db;
char*    zErrMsg = NULL;
int      rc = -1, GroupCount = 0, Gflag = -1, GtmpCounter = 0, DEVflag = -1;

uint64_t toQQ  = -1;
uint64_t toGp = -1;

void respons(const char* );
void polling();
char* U2G(const char* );
char* G2U(const char* );
void respTimer();
bool cmdExec(const char* command);
extern CQcmd mainParse(std::string cmd);

string   APPpath, pBuff;

time_t CQSTATRTTIME = time(NULL);

time_t Time_lastTrigger;

const int64_t AdminQQ = 2139223150; // 以后打算通过读配置文件之类的方式来动态设置

int ac = -1; //AuthCode 调用酷Q的方法时需要用到
bool enabled = false;


/* 
* 返回应用的ApiVer、Appid，打包后将不会调用
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/* 
* 接收应用AuthCode，酷Q读取应用信息后，如果接受该应用，将会调用这个函数并传递AuthCode。
* 不要在本函数处理其他任何代码，以免发生异常情况。如需执行初始化代码请在Startup事件中执行（Type=1001）。
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}


/*
* Type=1001 酷Q启动
* 无论本应用是否被启用，本函数都会在酷Q启动后执行一次，请在这里执行应用初始化代码。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
*/
CQEVENT(int32_t, __eventStartup, 0)() {

	APPpath = CQ_getAppDirectory(ac);
	pBuff = "当前应用目录→" + APPpath;
	CQ_addLog(ac, CQLOG_DEBUG, "运行环境", pBuff.c_str());

	FILE* flog;
	char buff[256];
	time_t curTime = time(NULL);

	pBuff = APPpath + "status.log";
	try{
		fopen_s(&flog, pBuff.c_str(), "r+");
	}
	catch (const std::exception& ){

	}
	

	if (flog != NULL) {
		fseek(flog, 0, SEEK_END);
		int len_file = (int)ftell(flog);
		rewind(flog);
		if (len_file < 11 || len_file > 12) {
			pBuff = "酷Q启动记录文件长度为" + to_string(len_file) + ", 可能存在问题";
			CQ_addLog(ac, CQLOG_WARNING, "运行环境", pBuff.c_str());
		}
		else {
			char sta[2], tim[11];
			fseek(flog, 0, SEEK_SET);
			fread(&sta, 1, 1, flog);
			sta[1] = '\0';
			fread(&tim, 1, 10, flog);
			tim[10] = '\0';
			sprintf_s(buff, "酷Q启动成功, 上次记录状态→%s, 上次记录时间→%s", sta, tim);
			CQ_addLog(ac, CQLOG_DEBUG, "运行环境", buff);
		}
		fseek(flog, 0, SEEK_SET);
		sprintf_s(buff, "1 %I64d", curTime);
		fwrite(buff, 1, strlen(buff), flog);
		fclose(flog);
	}
	else {
			CQ_addLog(ac, CQLOG_ERROR, "运行环境", "状态文件打开失败, 请检查目录以及权限");
			pBuff = "当前APP目录→" + APPpath;
			CQ_addLog(ac, CQLOG_INFO, "运行环境", pBuff.c_str());
	}
	if(flog != NULL)
		fclose(flog);

	return 0;
}


/*
* Type=1002 酷Q退出
* 无论本应用是否被启用，本函数都会在酷Q退出前执行一次，请在这里执行插件关闭代码。
* 本函数调用完毕后，酷Q将很快关闭，请不要再通过线程等方式执行其他代码。
*/
CQEVENT(int32_t, __eventExit, 0)() {
	char buff[256];
	FILE* flog;
	time_t curTime = time(NULL);

	pBuff = APPpath + "status.log";
	fopen_s(&flog, pBuff.c_str(), "w");
	if (flog != NULL) {
		sprintf_s(buff, "0 %I64d", curTime);
		fwrite(buff, 1, strlen(buff), flog);
		fclose(flog);
	}
	if (db != NULL) {
		sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
		sqlite3_close(db);
		sqlite3_free(zErrMsg);
	}
	return 0;
}

/*
* Type=1003 应用已被启用
* 当应用被启用后，将收到此事件。
* 如果酷Q载入时应用已被启用，则在_eventStartup(Type=1001,酷Q启动)被调用后，本函数也将被调用一次。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
*/
CQEVENT(int32_t, __eventEnable, 0)() {
	enabled = true;

	char *outbuff;

	pBuff = CQ_getAppDirectory(ac) ;
	pBuff += "app.db";
	rc = sqlite3_open(pBuff.c_str(), &db);
	pBuff = "数据库(" + pBuff + ")连接状态→" + to_string(rc);
	CQ_addLog(ac, CQLOG_DEBUG, "数据库", pBuff.c_str());

	if ( rc == SQLITE_OK ) {
		// 得先检查一下数据库是不是正确的, 不是的话就有可能是第一次使用, 就还得初始化一下
		bool dbIscorrect = FALSE;
		// 下面这段是抄来的
		char strFindTable[128];
		sprintf_s(strFindTable, 127, "SELECT * FROM `sqlite_master` where type = 'table' and name = 'event'");
		sqlite3_stmt* stmt0 = NULL;
		if (sqlite3_prepare_v2(db, strFindTable, strlen(strFindTable), &stmt0, NULL) != SQLITE_OK) {
			if (stmt0) {
				sqlite3_finalize(stmt0);
				sqlite3_close(db);
				CQ_addLog(ac, CQLOG_ERROR, "运行环境", "数据库查询初始化错误");
			}
		}
		int r = sqlite3_step(stmt0);
		//判断表存在，结果集是否已到末尾
		//通过sqlite3_step命令执行创建表的语句。对于DDL和DML语句而言， sqlite3_step执行正确的返回值
		//只有SQLITE_DONE，对于SELECT查询而言，如果有数据返回SQLITE_ROW，当到达结果集末尾时则返回SQLITE_DONE。
		if (r == SQLITE_DONE) {
			dbIscorrect = FALSE;
			CQ_addLog(ac, CQLOG_WARNING, "数据库", "没有找到数据表, 准备初始化数据库...");
		}
		else if (r == SQLITE_ROW) 
			dbIscorrect = TRUE;
		// 上面这段是抄来的

		// 这一段是app.db的结构
		if (dbIscorrect != TRUE) {
			rc = sqlite3_exec(db, "CREATE TABLE  `event` (`EID` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,`TYPE` INTEGER NOT NULL,`TIME` NUMERIC NOT NULL DEFAULT CURRENT_TIMESTAMP,`LINK` INTEGER,`CONT` TEXT,`NOTE` TEXT,`STATUS` INTEGER);", NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				pBuff = "数据库初始化失败(" + to_string(rc) + "):" + zErrMsg;
				CQ_addLog(ac, CQLOG_ERROR, "运行环境", pBuff.c_str());
			}
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			rc = sqlite3_exec(db, "CREATE TABLE `relationship` (`QQ` INTEGER NOT NULL UNIQUE,`Nickname` TEXT NOT NULL,`level` INTEGER NOT NULL,`amity` INTEGER NOT NULL,`from` TEXT,`note` TEXT,`lastActiv` NUMERIC DEFAULT CURRENT_TIMESTAMP,PRIMARY KEY(`QQ`));", NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				pBuff = "数据库初始化失败(" + to_string(rc) + "):" + zErrMsg;
				CQ_addLog(ac, CQLOG_ERROR, "运行环境", pBuff.c_str());
			}
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			rc = sqlite3_exec(db, "CREATE INDEX `Activ` ON `relationship` (`lastActiv` DESC);", NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				pBuff = "数据库初始化失败(" + to_string(rc) + "):" + zErrMsg;
				CQ_addLog(ac, CQLOG_ERROR, "运行环境", pBuff.c_str());
			}
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			rc = sqlite3_exec(db, "CREATE INDEX `power` ON `relationship` (`level` ASC);", NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				pBuff = "数据库初始化失败(" + to_string(rc) + "):" + zErrMsg;
				CQ_addLog(ac, CQLOG_ERROR, "运行环境", pBuff.c_str());
			}
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			rc = sqlite3_exec(db, "CREATE INDEX `priority` ON `event` (`STATUS` DESC);", NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				pBuff = "数据库初始化失败(" + to_string(rc) + "):" + zErrMsg;
				CQ_addLog(ac, CQLOG_ERROR, "运行环境", pBuff.c_str());
			}
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			sqlite3_free(zErrMsg);
		}

		char sql[] = { \
"INSERT INTO `main`.`event`" \
"(`TYPE`, `NOTE`, `STATUS`)"  \
"VALUES(1003, 'app_on_Enable', 200); " };
		rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
		if (rc == SQLITE_OK) {
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			char hint[] = { "SQL(\n%s\n)执行成功" };
			int len_buff = strlen(sql) + strlen(hint);
			outbuff = (char*)malloc(len_buff + 8);
			if (outbuff != NULL) {
				sprintf_s(outbuff, len_buff, hint, sql);
				CQ_addLog(ac, CQLOG_DEBUG, "数据库", outbuff);
			}
			free(outbuff);
			outbuff = NULL;
		}
		else {
			char hint[] = { "因为%s,\nSQL(\n%s\n)执行失败" };
			int len_buff = strlen(sql) + strlen(hint);
			outbuff = (char*)malloc(len_buff + 8);
			if (outbuff != NULL) {
				sprintf_s(outbuff, len_buff, hint, zErrMsg, sql);
				CQ_addLog(ac, CQLOG_ERROR, "数据库", outbuff);
				sqlite3_free(zErrMsg);
			}
			free(outbuff);
			outbuff = NULL;
		}
	}
	else {
		pBuff = "数据库连接失败(" + to_string(rc) + ")";
		CQ_addLog(ac, CQLOG_ERROR, "运行环境", pBuff.c_str());
	}

	respons("on_Enable");

	thread Timer60s(respTimer);
	Timer60s.detach();

	return 0;
}


/*
* Type=1004 应用将被停用
* 当应用被停用前，将收到此事件。
* 如果酷Q载入时应用已被停用，则本函数*不会*被调用。
* 无论本应用是否被启用，酷Q关闭前本函数都*不会*被调用。
*/
CQEVENT(int32_t, __eventDisable, 0)() {
	enabled = false;
	return 0;
}


/*
* Type=21 私聊消息
* subType 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
*/

// 主要的应答回复是由后端生成的, 这里只针对私聊和群艾特这样高优先级的事件做一些转发和指令处理.

CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char *msg, int32_t font) {

	//如果要回复消息，请调用酷Q方法发送，并且这里 return EVENT_BLOCK - 截断本条消息，不再继续处理  注意：应用优先级设置为"最高"(10000)时，不得使用本返回值
	//如果不回复消息，交由之后的应用/过滤器处理，这里 return EVENT_IGNORE - 忽略本条消息

	string cmd = msg;
	int32_t id = msgId;
	

	if (fromQQ == AdminQQ) {

		if (cmd.find("命令模式") != string::npos) {
			if (DEVflag == 233) {
				if (cmd.find("退出") != string::npos) {
					DEVflag = -1;
					CQ_sendPrivateMsg(ac, fromQQ, "已退出命令模式");
					return EVENT_BLOCK;
				}
				else
					CQ_sendPrivateMsg(ac, fromQQ, "已处于命令模式, 发送退出来退出");
			}
			if (cmd.find("进入") != string::npos && DEVflag == -1) {
				DEVflag = 233;
				CQ_sendPrivateMsg(ac, fromQQ, "已进入命令模式");
				return EVENT_BLOCK;
			}
		}

		if (DEVflag == 233) {
			bool isExeced;
			isExeced = cmdExec(msg);
			if (isExeced == OK)
				CQ_sendPrivateMsg(ac, fromQQ, "命令已执行");
			else
				CQ_sendPrivateMsg(ac, fromQQ, "命令执行失败");
			return EVENT_BLOCK;
		}
			
		regex parm1("\\d{6,12}");
		smatch result;
		char buff[2048];

		if (cmd.find("单行") < 16 && cmd.find(':') != string::npos) {
			unsigned int pos;
			pos = cmd.find(':');
	
			sprintf_s(buff, 2047,"管理员单行私聊模式:%s长%d字符, 内容→\n%s", cmd.c_str(), cmd.length(), cmd.substr(pos).c_str());
			CQ_addLog(ac, CQLOG_INFO, "管理操作", buff);

			if (regex_search(cmd, result, parm1)) 
				toQQ = stoll(result.str(), 0, 10);
			if (toQQ != -1 && (cmd.size() > pos)) 
				CQ_sendPrivateMsg(ac, toQQ, cmd.substr(pos + 1).c_str());
			else {
				CQ_addLog(ac, CQLOG_ERROR, "运行环境", "指令分析失败");
				pBuff = "内容(\n" + cmd.substr(pos + 1) + "\n)发送失败";
				CQ_sendPrivateMsg(ac, AdminQQ, pBuff.c_str());
			}	
		}

		if ((cmd.find("结束") < 6 || cmd.find("退出") < 6) && Gflag != -1) {
			if (Gflag == 233) {
				toQQ = -1;
				toGp = -1;
				Gflag = -1;
				CQ_sendPrivateMsg(ac, AdminQQ, "已退出对话模式");
			}
		}

		if (Gflag == 233 && toQQ != -1) 
			CQ_sendPrivateMsg(ac, toQQ, cmd.c_str());
		if (Gflag == 233 && toGp != -1)
			CQ_sendGroupMsg(ac, toGp, cmd.c_str());

		if (cmd.find("对话") < 16 && Gflag == -1) {
			regex parm1("\\d{6,12}");
			smatch result;
			
			if (regex_search(cmd, result, parm1)) {
				if (cmd.find("群") < 16 && toGp == -1) {
					toQQ = -1;
					toGp = stoll(result.str(), 0, 10);
					sprintf_s(buff, 2047, "\n管理员与 %I64d 的群聊对话开始", toGp);
					CQ_addLog(ac, CQLOG_INFO, "管理操作", "进入群聊对话模式");
					CQ_sendPrivateMsg(ac, AdminQQ, "已进入群聊对话模式");
				}
				else if (toQQ == -1) {
					toGp = -1;
					toQQ = stoll(result.str(), 0, 10);
					sprintf_s(buff, 2047, "\n管理员与 %I64d 的私聊对话开始", toQQ);
					CQ_addLog(ac, CQLOG_INFO, "管理操作", "进入私聊对话模式");
					CQ_sendPrivateMsg(ac, AdminQQ, "已进入私聊对话模式");
				}
				Gflag = 233;
			}
			else {
				CQ_addLog(ac, CQLOG_ERROR, "运行环境", "指令分析失败");
				CQ_sendPrivateMsg(ac, AdminQQ, "指令分析失败");
				Gflag = -1;
			}
		}
	}
	else {// 别人私聊的内容都转发到主号并转发主号的应答
		// 测试阶段先直接转发
		char* obuff = NULL;
		char buff[256] = {'\0'};
		int len_buff = 0;

		char hint[] = { "来自%I64d的私聊消息:" };
		sprintf_s(buff, 255, hint, fromQQ);

		CQ_sendPrivateMsg(ac, AdminQQ, buff);

		// 然后存到数据库里让后端程序处理
		string sql;

		sql = "INSERT INTO `main`.`event` (`TYPE`, `LINK`, `CONT`, `NOTE`, `STATUS`)"\
			"VALUES(6001," + to_string(id) + ", '" + G2U( cmd.substr(0, 128).c_str() ) + "', '" + G2U("来自私聊, 调试阶段统一人工转发") + "', 301); ";

		rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
		if (rc == SQLITE_OK) {
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			char hint[] = { "\nSQL(\n%s\n)执行成功" };
			len_buff = sql.length() + strlen(hint);
			obuff = (char*)malloc(len_buff + 8);
			if (obuff != NULL) {
				sprintf_s(obuff, len_buff, hint, sql.c_str());
				CQ_addLog(ac, CQLOG_DEBUG, "数据库", obuff);
			}
			free(obuff);
			obuff = NULL;
		}
		else {
			char hint[] = { "因为%s,\nSQL(\n%s\n)执行失败" };
			len_buff = sql.length() + strlen(hint);
			obuff = (char*)malloc(len_buff + 8);
			if (obuff != NULL) {
				sprintf_s(obuff, len_buff, hint, zErrMsg, sql);
				CQ_addLog(ac, CQLOG_ERROR, "数据库", obuff);
				sqlite3_free(zErrMsg);
			}
			else
				CQ_addLog(ac, CQLOG_ERROR, "运行环境", "内存分配失败");
			free(obuff);
			obuff = NULL;
		}
		free(obuff);
		obuff = NULL;

		CQ_sendPrivateMsg(ac, AdminQQ, cmd.c_str());
	}
	respons("on_PrivateMsg");

	return EVENT_IGNORE;
}


/*
* Type=2 群消息
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {

	GroupCount += 1;
	if (GroupCount > 50) {
		GroupCount = 0;
		respons("on_GroupMsgCounter");
	}

	string grpMsg = msg;

	if (Gflag == 233 && toGp == fromGroup) {
		pBuff = to_string(fromQQ) + ":\n" + grpMsg;
		CQ_sendPrivateMsg(ac, AdminQQ, pBuff.c_str());
	}

	if (grpMsg.find("[CQ:at,qq=") != string::npos) {
		if (grpMsg.find("qq=616471607") != string::npos || grpMsg.find("qq=2154055060") != string::npos || grpMsg.find("qq=2139223150") != string::npos) {  // 以后做成读配置的
			CQ_sendGroupMsg(ac, fromGroup, "喵~");

			// 在群里被艾特的内容都转发到主号并转发主号的应答
			// 测试阶段先直接转发
			char* obuff = NULL;
			char buff[256] = { '\0' };
			int len_buff = 0;

			char hint[] = { "来自%I64d在群(%I64d)的艾特:" };
			sprintf_s(buff, 255, hint, fromQQ, fromGroup);

			CQ_sendPrivateMsg(ac, AdminQQ, buff);

			// 然后存到数据库里让后端程序处理
			string sql;

			sql = "INSERT INTO `main`.`event` (`TYPE`, `LINK`, `CONT`, `NOTE`, `STATUS`)"\
				"VALUES(6002," + to_string(msgId) + ", '" + G2U(grpMsg.substr(0, 128).c_str()) + "', '" + G2U("来自群, 调试阶段统一人工转发") + "', 301); ";

			rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
			if (rc == SQLITE_OK) {
				sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
				char hint[] = { "\nSQL(\n%s\n)执行成功" };
				len_buff = sql.length() + strlen(hint);
				obuff = (char*)malloc(len_buff + 8);
				if (obuff != NULL) {
					sprintf_s(obuff, len_buff, hint, sql.c_str());
					CQ_addLog(ac, CQLOG_DEBUG, "数据库", obuff);
				}
				free(obuff);
				obuff = NULL;
			}
			else {
				char hint[] = { "因为%s,\nSQL(\n%s\n)执行失败" };
				len_buff = sql.length() + strlen(hint);
				obuff = (char*)malloc(len_buff + 8);
				if (obuff != NULL) {
					sprintf_s(obuff, len_buff, hint, zErrMsg, sql);
					CQ_addLog(ac, CQLOG_ERROR, "数据库", obuff);
					sqlite3_free(zErrMsg);
				}
				else
					CQ_addLog(ac, CQLOG_ERROR, "运行环境", "内存分配失败");
				free(obuff);
				obuff = NULL;
			}
			free(obuff);
			obuff = NULL;

			CQ_sendPrivateMsg(ac, AdminQQ, grpMsg.c_str());
			respons("on_GroupMsgAT");
		}
	}
	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=4 讨论组消息
*/
// 讨论组似乎已经被废弃了? 实际创建多人聊天发现也算在群消息里
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t msgId, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=101 群事件-管理员变动
* subType 子类型，1/被取消管理员 2/被设置管理员
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=102 群事件-群成员减少
* subType 子类型，1/群员离开 2/群员被踢 3/自己(即登录号)被踢
* fromQQ 操作者QQ(仅subType为2、3时存在)
* beingOperateQQ 被操作QQ
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=103 群事件-群成员增加
* subType 子类型，1/管理员已同意 2/管理员邀请
* fromQQ 操作者QQ(即管理员QQ)
* beingOperateQQ 被操作QQ(即加群的QQ)
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=201 好友事件-好友已添加
*/
CQEVENT(int32_t, __eventFriend_Add, 16)(int32_t subType, int32_t sendTime, int64_t fromQQ) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=301 请求-好友添加
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag) {

	//CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");
	
	pBuff = CQ_getStrangerInfo(ac, fromQQ, TRUE);
	CQ_addLog(ac, CQLOG_INFO, "好友请求", pBuff.c_str());
	CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");
	
	// 加好友就直接同意, 然后写数据库里让后端AI分析人际关系
	// level越小权限越高, amity越大越友好. 初始给个适中值, 由后端AI调整
	string sql = "INSERT INTO `main`.`relationship` (`QQ`, `nickname`, `level`, `amity`) VALUES(" + to_string(fromQQ) + ", '" + pBuff.c_str() +"', 250, 10);";

	int len_buff;
	char* obuff;

	rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
	if (rc == SQLITE_OK) {
		sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
		char hint[] = { "\nSQL(\n%s\n)执行成功" };
		len_buff = sql.length() + strlen(hint);
		obuff = (char*)malloc(len_buff + 8);
		if (obuff != NULL) {
			sprintf_s(obuff, len_buff, hint, sql.c_str());
			CQ_addLog(ac, CQLOG_DEBUG, "数据库", obuff);
		}
		free(obuff);
		obuff = NULL;
	}
	else {
		char hint[] = { "因为%s,\nSQL(\n%s\n)执行失败" };
		len_buff = sql.length() + strlen(hint);
		obuff = (char*)malloc(len_buff + 8);
		if (obuff != NULL) {
			sprintf_s(obuff, len_buff, hint, zErrMsg, sql);
			CQ_addLog(ac, CQLOG_ERROR, "数据库", obuff);
			sqlite3_free(zErrMsg);
		}
		else
			CQ_addLog(ac, CQLOG_ERROR, "运行环境", "内存分配失败");
		free(obuff);
		obuff = NULL;
	}
	free(obuff);
	obuff = NULL;

	pBuff += "(" + to_string(fromQQ) + ")请求添加好友, 附言`" + msg + "`. 已默认同意.";
	CQ_sendPrivateMsg(ac, AdminQQ, pBuff.c_str());
	respons("on_Request_AddFriend");

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=302 请求-群添加
* subType 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag) {

	//if (subType == 1) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPADD, REQUEST_ALLOW, "");
	//} else if (subType == 2) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
	//}

	if (subType == 2) {
		CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
		// 群邀请就直接加入了
		pBuff = CQ_getStrangerInfo(ac, fromQQ, TRUE);
		pBuff += "(" + to_string(fromQQ) + ")邀请加入群`" + to_string(fromGroup) + "`, 已默认同意";
		CQ_sendPrivateMsg(ac, AdminQQ, pBuff.c_str());
		respons("on_REQUEST_GROUPINVITE");
	}
		

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* 菜单，可在 .json 文件中设置菜单数目、函数名
* 如果不使用菜单，请在 .json 及此处删除无用菜单
*/
CQEVENT(int32_t, __menuA, 0)() {
	MessageBoxA(NULL, "这是menuA，在这里载入窗口，或者进行其他工作。", "" ,0);
	return 0;
}

CQEVENT(int32_t, __menuB, 0)() {
	MessageBoxA(NULL, "这是menuB，在这里载入窗口，或者进行其他工作。", "" ,0);
	return 0;
}

void respons(const char* eve) {
	time_t time_now = time(NULL);
	if (time_now - Time_lastTrigger > 10) {
		GtmpCounter += 1;
		char buff[128];
		int tmp = -1;
		sprintf_s(buff, "被动触发(%s)成功", eve);
		CQ_addLog(ac, CQLOG_DEBUG, "后台处理", buff);



		if (GtmpCounter < 10) { // 测试酷Q Pro版玩的, 顺手点个赞
			tmp += CQ_sendLike(ac, 616471607);
			tmp += CQ_sendLike(ac, AdminQQ);
			if (tmp != 0)
				GtmpCounter = 233;
			_ltoa_s(tmp, buff, 10);
			CQ_addLog(ac, CQLOG_DEBUG, "名片赞", buff);
		}
	}
}

void polling() {
	// 这里打算定时轮训数据库, 查找并处理后端程序的指令
}

void respTimer() {
	Timer timer;
	timer.start(60000, bind(respons, "on_Timer60s"));
	this_thread::sleep_for(chrono::hours(616471607));
	timer.stop();
}

/*
	这里的设计是做一个表, 三大类动作下有
*/

bool cmdExec(const char * strCmd) {
	CQcmd cmd;
	string command;
	command = strCmd;
	cmd = mainParse(command);

	if (cmd.status < 4 || cmd.status > 8) {
		CQ_sendPrivateMsg(ac, AdminQQ, "命令语法有误, 无法执行");
		return Err;
	}
		
	switch (cmd.cmdID)
	{
	case 0:
		if (cmd.toGrp != -1)
			CQ_sendGroupMsg(ac, cmd.toGrp, cmd.content);
		else if (cmd.toPri != -1) {
			if (cmd.content != "Err")
				CQ_sendPrivateMsg(ac, cmd.toPri, cmd.content);
			if (cmd.flag == 100)
				CQ_sendLike(ac, cmd.toPri);
		}
		break;
		
	case 1:
		switch (cmd.subCmdID)
		{
		case 0: 
			CQ_deleteMsg(ac, cmd.action);
			break;
		case 1:
			if (cmd.flag == 10)
				CQ_setGroupKick(ac, cmd.toGrp, cmd.toPri, FALSE);
			else if (cmd.flag == 233)
				CQ_setGroupLeave(ac, cmd.toGrp, FALSE);
			break;
		case 2:
			if (cmd.flag == 22)
				CQ_setGroupWholeBan(ac, cmd.toGrp, cmd.action);
			else if (cmd.flag < 22)
				CQ_setGroupBan(ac, cmd.toGrp, cmd.toPri, cmd.action);
			break;
		case 3:
			CQ_setGroupAnonymous(ac, cmd.toGrp, cmd.action);
			break;
		case 4:
			CQ_setGroupCard(ac, cmd.toGrp, cmd.toPri, cmd.content);
			break;
		case 5:
			CQ_setGroupSpecialTitle(ac, cmd.toGrp, cmd.toPri, cmd.content, cmd.action);
			break;
		case 6:
			// 加好友/加群邀请默认同意就先不做了
			break;
		case 7:
			CQ_addLog(ac, CQLOG_INFO, "后台处理", cmd.content);
			break;
		default:
			break;
		}

	case 3:
		switch (cmd.subCmdID)
		{
		case 8: 
			switch (cmd.flag) {
			case 81:
				pBuff = CQ_getGroupMemberInfoV2(ac, cmd.toGrp, cmd.toPri, TRUE);
				break;
			case 82:
				pBuff = CQ_getStrangerInfo(ac, cmd.toPri, TRUE);
				break;
			case 83:
				pBuff = to_string(CQ_getLoginQQ(ac));
				break;
			case 84:
				pBuff = CQ_getLoginNick(ac);
				break;
			}

		case 9:
			pBuff = CQ_getRecord(ac, cmd.content, "mp3");
			break;

		case 10:
			pBuff = CQ_getGroupMemberList(ac, cmd.toGrp);
			break;

		case 11:
			pBuff = CQ_getGroupList(ac);
			break;

		case 12:
			pBuff = CQ_getAppDirectory(ac);
			break;
				
		default:
			break;
		}
	default:
		break;
	}
	return OK;
}

// 下面这段抄自https://www.cnblogs.com/cyberarmy/p/10098649.html
//UTF-8到GB2312的转换
char* U2G(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}

//GB2312到UTF-8的转换
char* G2U(const char* gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}
// 上面这段抄自https://www.cnblogs.com/cyberarmy/p/10098649.html