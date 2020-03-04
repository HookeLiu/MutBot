/* 说明
*	17ds8多功能AI的QQ接口. 选用的是酷Q官方SDK的VC++版本, 因为这个版本的封装最少最接近底层, API和结构最清晰. 没有选用易语言的一个原因是难以做通用开发(关键原因是易语言要付费且界面累赘, 个人也不习惯易语言那种汉化和命名方式)
*	这个多功能AI项目的本体在于后端处理程序.
*	这个VC++工程基本只是在酷Q官方给的SDK上做一个简易的数据接口, 作为后端AI程序和QQ用户的桥梁.
	所以基本上就是clone过来加了点内容, 也没啥改动. 后端主要是用Python, 消息处理主体依靠神经网络.
*	(需要注意的是, 酷Q和这个工程用的都是GB2312, 数据库以及后端程序用的都是UTF-8, 
	所以这里用了个网上抄来的简易转码函数).
*	By 佚之狗 Dec. 2019
*/

/* TODO:
* 本程序导致酷Q退出时总是有残余线程不能立即退出, 不明原因, 待排查.
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

#define OK       1
#define Err      -1
#define Unknown  0
#define AppUpate 0x1000 // CQ更新数据后通知后端处理
#define AppExit  0x996
#define AppSync  0x300  // 后端处理完后让CQ端同步处理
#define HandSk   0x233
#define Breath   0x500  // 心跳包
#define CQreturn 0x8888
#define typ_db   101    // 记录到APP_DB
#define typ_dLog 102    // 记录到调试日志文件
#define typ_sLog 103    // 记录到状态文件

using namespace std;

/* 全局运行状态 */
sqlite3* db;
char*    zErrMsg = nullptr; // sqlite的错误提示消息
// rc 用于记录数据库请求状态; GroupCounter计次群消息, 打算用于状态监测以及条件触发之类; adminConverFlag用于判断是否处于命令模式; DEVflag保留测试用
int rc = -1, GroupCounter = 0, adminConverFlag = -1, DEVflag = -1;
static uint64_t GtmpCounter = 0;    // 全局触发计次, 保留测试用
static UINT8 Heartbeat = 5;         // 作为判断后端是否掉线的依据
// 配合命令模式使用, 发送给个人和发送给群互斥
uint64_t toQQ  = -1, toGp = -1;
// 考虑设计自检和容错
bool IsCorrectClosedLastTime = true; uint8_t ErrorCounter = 0;
string pBuff;						// 为了方便输出文字信息
time_t Time_lastTrigger;			// 主要用于日志, 也是配合自检和容错机制

/* 全局数据交换变量 */
long WPA = NULL;     // 消息参数1
long LPA = NULL;     // 消息参数2

/* 主要功能函数 */
void respons(const char* );					// 主功能, 应答以及触发处理
DWORD WINAPI polling(LPVOID p);				// 数据库轮询
char* acpToUTF8(const char* acp);   		// 辅助功能, Unicode转GB2312. (数据库以及后端是utf-8)
char* utf8ToACP(const char* utf8);   		// 辅助功能, GB2312转Unicode. (酷Q是GB2312, 为了方便传递和存储)
int cmdExec(const char* command);			// 主功能, 分析命令语句并调用相应酷Q API
// TODO: 得考虑做个像个样子的解释器... 现在(16Feb2020)只是傻傻地词法分析后用switch case选择酷Q API, 还不如用正则...
int fileLog(int logType, string msg);       // 记录到状态文件和调试文件
int dbCheck();                              // 数据库自检
int dbInit();                               // 新建数据库初始化	
DWORD WINAPI breath(LPVOID p);              // 发送心跳包
void escapeCharA(char** p);                 // 存数据库时特殊符号的转义(不转义会导致酷Q内部错误)
void oppositeEscapeCharA(char* r);          // 反转义

/* 应用间通信 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); // 窗口通信的回调函数
HWND CQwd = nullptr;						    // CQ端通信窗口的句柄 
HWND Pywd = nullptr;						    // 中间层通信窗口的句柄 

/* 全局配置 */
const int64_t AdminQQ = 2139223150; // 以后打算通过读配置文件之类的方式来动态设置
static string APPpath, dbPath;		// 相当于全局Setting了
int ac = -1;						// (AuthCode, 酷Q自己的认证机制, 调用CQ API都需要传入)
bool enabled = false;				// 似乎没啥乱用的开关状态
CRITICAL_SECTION g_csVar;			// 主要是担心数据锁引发错误
string blackList;                   // 黑名单, 以后做成读配置项的

/* 多线程 */
// 0-app主响应线程, 1-发送通信心跳信号
const int THREAD_NUM = 2;
HANDLE TdHandle[THREAD_NUM];
DWORD ThreadId[THREAD_NUM];


// ================酷Q接口=================

/*
* 返回应用的ApiVer、Appid，打包后将不会调用
*/
CQEVENT(const char*, AppInfo, 0)() { return CQAPPINFO; }

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
	/*
	* 初始化一些全局参数比如path等.
	* 一些简单的日志以便调试.
	*/

	CQ_addLog(ac, CQLOG_INFO, "酷Q启动", "--------------");
	double time_start = clock(); // 测试用
	APPpath = CQ_getAppDirectory(ac);
	dbPath = APPpath + "app.db";

	char* _tmpBuff = (char*)malloc(APPpath.length() * 2);
	_tmpBuff = acpToUTF8(dbPath.c_str());                  // sqlite3不支持GB2312, 如果不转成utf-8则不能处理非ASCII的目录名
	if (dbPath.find(_tmpBuff) == string::npos) {
		CQ_addLog(ac, CQLOG_WARNING, "运行环境", "检测到运行在非英文目录, 这是不建议的.");
		dbPath = _tmpBuff;
	}
	dbPath = _tmpBuff;
	freeString(_tmpBuff);

	FILE* flog;
	char buff[256];
	time_t curTime = time(NULL);

	pBuff = APPpath + "status.log";
	try {
		fopen_s(&flog, pBuff.c_str(), "r+");
	}
	catch (const std::exception&) {
	}
	if (flog) {
		fseek(flog, 0, SEEK_END);
		int len_file = (int)ftell(flog);
		rewind(flog);
		if (len_file < 11 || len_file > 12) {
			pBuff = "酷Q启动记录文件长度为" + to_string(len_file) + ", 可能存在问题也可能是第一次使用";
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

			if (sta[0] != '0') {    // 这说明上次未按期望的方式退出程序, 可能是酷Q重新加载应用, 为了防止上次残余的线程影响本次运行, 应该先做一些相关处理.
				IsCorrectClosedLastTime = false;
				CQ_addLog(ac, CQLOG_WARNING, "运行环境", "检测到上次未按期望的方式退出程序, 这可能会造成一些异常. 虽然程序设计了有限的容错机制, 但仍建议退出酷Q并等待几秒后再重新启动酷Q");
			}
		}
		fseek(flog, 0, SEEK_SET);
		sprintf_s(buff, "1 %I64d", curTime);
		fwrite(buff, 1, strlen(buff), flog);
		fclose(flog);
	}
	else {
		CQ_addLog(ac, CQLOG_WARNING, "运行环境", "状态文件打开失败, 尝试创建文件");
		pBuff = APPpath + "status.log";
		try {
			fopen_s(&flog, pBuff.c_str(), "w");
		}
		catch (const std::exception&) {
			CQ_addLog(ac, CQLOG_WARNING, "运行环境", "无法创建状态文件, 请检查权限和目录");
		}
		if (flog) {
			CQ_addLog(ac, CQLOG_INFOSUCCESS, "首次启动", "新建成功. 欢迎使用~");
		}
		else {
			CQ_addLog(ac, CQLOG_ERROR, "运行环境", "尝试再次创建仍然失败, 请检查目录名以及权限.");
		}
		pBuff = "当前APP目录→" + APPpath;
		CQ_addLog(ac, CQLOG_INFO, "运行环境", pBuff.c_str());
	}
	if (flog)
		fclose(flog);

	if (db) {
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		db = nullptr;
	}

	double time_end = clock(); // 测试用
	double dur_startup = (double)((time_end - time_start) / CLOCKS_PER_SEC);
	sprintf_s(buff, "启动耗时→%.3f秒", dur_startup);
	CQ_addLog(ac, CQLOG_DEBUG, "运行时长", buff);

	//debug MessageBoxA(NULL, "CQEVENT(int32_t, __eventStartup, 0)", "debug", 0);

	return 0;
}


/*
* Type=1002 酷Q退出
* 无论本应用是否被启用，本函数都会在酷Q退出前执行一次，请在这里执行插件关闭代码。
* 本函数调用完毕后，酷Q将很快关闭，请不要再通过线程等方式执行其他代码。
*/
CQEVENT(int32_t, __eventExit, 0)() {
	double time_start = clock(); // 测试用

	enabled = false;

	string t;
	t = "INSERT INTO `main`.`event` (`TYPE`, `CONT`, `NOTE`, `STATUS`) VALUES(1002, '本次运行共触发了 " + to_string(GtmpCounter) + " 次', 'CQ_on_EXIT', 200); ";
	fileLog(typ_db, t);
	t = "0 %I64d";
	fileLog(typ_sLog, t);

	if (db != nullptr) {
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		db = nullptr;
	}

	if (Pywd != nullptr)
		PostMessage(Pywd, WM_USER, AppExit, 0x1024);

	if (&g_csVar) DeleteCriticalSection(&g_csVar);

	double time_end = clock(); // 测试用
	double dur_exit = (double)((time_end - time_start) / CLOCKS_PER_SEC);
	char buff[128];
	sprintf_s(buff, "退出(酷Q)耗时→%.3f秒", dur_exit);
	CQ_addLog(ac, CQLOG_DEBUG, "运行时长", buff);

	freeString(buff);

	if (CQwd != nullptr) {
		PostMessage(CQwd, WM_USER, AppExit, 0x1024);
		PostMessage(CQwd, WM_DESTROY, AppExit, 0x1024);
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
	/*
	* 启用之后初始化通信和数据库, 准备自检和容错.
	*/
	double time_start = clock(); // 测试用
	enabled = true;

	InitializeCriticalSection(&g_csVar);

	int dbIscorrect = dbCheck();
	if (dbIscorrect != OK) dbInit();

	pBuff = "3 %I64d";
	fileLog(typ_sLog, pBuff);
	CQ_addLog(ac, CQLOG_INFO, "程序流程", "初始化开始. 正在创建通信窗口...");

	CQwd = createMainWindow(WndProc);
	TCHAR title[64];
	GetWindowText(CQwd, title, 60);
	char buff[256];
	sprintf_s(buff, "窗口创建成功:  窗口句柄 ==> 0x%p; 窗口标题 == > %ws", CQwd, title);
	CQ_addLog(ac, CQLOG_INFOSUCCESS, "窗口程序", buff);

	if (((_access(".\\MutBot\\PyDeamon\\PyDeamon.py", 0)) != -1) && ((_access("C:\\Windows\\py.exe", 0)) != -1)) {  // 尝试启动中间层服务程序
		CQ_addLog(ac, CQLOG_INFO, "运行环境", "发现中间层程序, 尝试启动...");
		char buf1[256];
		char para[512];
		char debug[512];
		strcpy_s(para, ".\\MutBot\\PyDeamon\\PyDeamon.py \"");
		if (_getcwd(buf1, sizeof(buf1)) == NULL);
		strcat_s(para, buf1);
		strcat_s(para, "\\data\"");
		
		wchar_t* cmd = (wchar_t*)malloc(strlen(para) * 2);
		cmd = acpToUTF16(para);

		wchar_t* path = (wchar_t*)malloc(strlen(buf1) * 2);
		path = acpToUTF16(buf1);

		sprintf_s(debug, "尝试调用`ShellExecute(0x%p, 'open', 'C:\\Windows\\py.exe', '%s', '%s', SW_SHOWMINNOACTIVE);`", CQwd, para, buf1);
		CQ_addLog(ac, CQLOG_DEBUG, "程序流程", debug);

		HINSTANCE hNewExe = ShellExecute(
			CQwd,  // 父窗口句柄或出错时显示错误父窗口的句柄
			L"open",
			L"C:\\Windows\\py.exe",
			cmd,                 // 可执行程序的参数, 没有可设为NULL
			path,                // 默认目录
			SW_SHOWMINIMIZED
		);

		char buff[512];
		if ((DWORD)hNewExe <= 32) {
			sprintf_s(buff, "中间层程序启动失败: %d", GetLastError());
			CQ_addLog(ac, CQLOG_WARNING, "运行环境", buff);
		}
		else {
			sprintf_s(buff, "中间层启动成功( %ld ). 等待窗口创建(先睡3秒)...", hNewExe);
			CQ_addLog(ac, CQLOG_DEBUG, "运行环境", buff);
			Sleep(3000);
		}
		freeString(cmd);
		freeString(path);
	}

	CQ_addLog(ac, CQLOG_DEBUG, "程序流程", "应用已启动, 准备寻找后端窗口...");

	Pywd = FindWindow(NULL, PyTitle);

	if (Pywd != nullptr) {
		CQ_addLog(ac, CQLOG_DEBUG, "程序流程", "成功找到后端窗口. 尝试握手...");
		PostMessage(Pywd, WM_USER, HandSk, 0x01);
	}
	else {
		ErrorCounter += 1;
		CQ_addLog(ac, CQLOG_WARNING, "程序流程", "没有找到后端应用, 请先运行后端应用.");
	}

	TdHandle[1] = CreateThread(NULL, 0, breath, NULL, NULL, &ThreadId[1]);
	if(TdHandle[1]) CloseHandle(TdHandle[1]);
	sprintf_s(buff, "`breath`: Handle ==> 0x%p (%ld); Id ==> 0x%ld (%ld)", TdHandle[1], (ULONG)TdHandle[1], ThreadId[1], ThreadId[1]);
	CQ_addLog(ac, CQLOG_DEBUG, "线程创建", buff);

	string sql = "INSERT INTO `main`.`event` (`TYPE`, `NOTE`, `STATUS`) VALUES(1003, 'app_on_Enable', 200); ";
	fileLog(typ_db, sql);

	CQ_addLog(ac, CQLOG_INFO, "程序流程", "初始化程序完成");

	double time_end = clock(); // 测试用
	double dur_init = (double)((time_end - time_start) / CLOCKS_PER_SEC);
	sprintf_s(buff, "初始化耗时→%.3f秒. ac码: %d", dur_init, ac);
	CQ_addLog(ac, CQLOG_DEBUG, "运行时长", buff);
	CQ_addLog(ac, CQLOG_INFO, "======", "==============");

	//debug MessageBoxA(NULL, "CQEVENT(int32_t, __eventEnable, 0)", "debug", 0);
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

	double time_start = clock(); // 测试用

	string t;
	t = "INSERT INTO `main`.`event` (`TYPE`, `CONT`, `NOTE`, `STATUS`) VALUES(1002, '本次运行共触发了 " + to_string(GtmpCounter) + " 次', 'CQ_on_disable', 200); ";
	fileLog(typ_db, t);
	t = "0 %I64d";
	fileLog(typ_sLog, t);

	if (db != nullptr) {
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		db = nullptr;
	}

	if (Pywd != nullptr)
		PostMessage(Pywd, WM_USER, AppExit, 0x1024);

	if (&g_csVar) DeleteCriticalSection(&g_csVar);


	double time_end = clock(); // 测试用
	double dur_disable = (double)((time_end - time_start) / CLOCKS_PER_SEC);
	char buff[128];
	sprintf_s(buff, "停用耗时→%.3f秒", dur_disable);
	CQ_addLog(ac, CQLOG_DEBUG, "运行时长", buff);
	CQ_addLog(ac, CQLOG_INFO, "******", "**************");

	if (CQwd != nullptr) {
		PostMessage(CQwd, WM_USER, AppExit, 0x1024);
	}

	//pBuff = "函数`__eventDisable`(handle" + to_string(__threadhandle()) + " -- threadid" + to_string(__threadid()) + ") 结束";
	//MessageBoxA(NULL, pBuff.c_str(), "debug", 0);  // debug
	return 0;
}


/*
* Type=21 私聊消息
* subType 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char* msg, int32_t font) {
	//如果要回复消息，请调用酷Q方法发送，并且这里 return EVENT_BLOCK - 截断本条消息，不再继续处理  注意：应用优先级设置为"最高"(10000)时，不得使用本返回值
	//如果不回复消息，交由之后的应用/过滤器处理，这里 return EVENT_IGNORE - 忽略本条消息

		// 主要的应答回复是由后端生成的, 这里只针对私聊和群艾特这样高优先级的事件做一些转发和指令处理.
	if (enabled) {
		if (blackList.find(to_string(fromQQ)) != string::npos) return EVENT_BLOCK;
		double time_start = clock(); // 测试用
		string cmd = msg;

		if (fromQQ == AdminQQ) {
			if (cmd.find("命令模式") != string::npos) {
				if (DEVflag == 233) {
					if (cmd.find("退出") != string::npos) {
						DEVflag = -1;
						CQ_sendPrivateMsg(ac, fromQQ, "已退出命令模式");
						return EVENT_BLOCK;
					}
					else {
						CQ_sendPrivateMsg(ac, fromQQ, "已处于命令模式, 发送退出来退出");
						return EVENT_BLOCK;
					}
				}
				if (cmd.find("进入") != string::npos && DEVflag == -1) {
					DEVflag = 233;
					CQ_sendPrivateMsg(ac, fromQQ, "已进入命令模式");
					return EVENT_BLOCK;
				}
				return EVENT_BLOCK;
			}

			if (DEVflag == 233) {
				int status;
				char buff[128];
				status = cmdExec(msg);
				if (status >= OK) {
					sprintf_s(buff, "命令执行成功(%d)", status);
					CQ_sendPrivateMsg(ac, fromQQ, buff);
				}
				else if (status >= Unknown) {
					sprintf_s(buff, "命令已执行, 但不确定结果(%d)", status);
					CQ_sendPrivateMsg(ac, fromQQ, buff);
				}
				else {
					CQ_sendPrivateMsg(ac, fromQQ, "命令无法解析");
				}
				return EVENT_BLOCK;
			}

			regex parm1("\\d{6,12}");
			smatch result;
			char buff[2048];

			if ((cmd.find("结束") < 6 || cmd.find("退出") < 6) && adminConverFlag != -1) {
				if (adminConverFlag == 233) {
					toQQ = -1;
					toGp = -1;
					adminConverFlag = -1;
					CQ_sendPrivateMsg(ac, AdminQQ, "已退出对话模式");
				}
			}

			if (adminConverFlag == 233 && toQQ != -1) {
				CQ_sendPrivateMsg(ac, toQQ, cmd.c_str());
				return EVENT_BLOCK;
			}
			if (adminConverFlag == 233 && toGp != -1) {
				CQ_sendGroupMsg(ac, toGp, cmd.c_str());
				return EVENT_BLOCK;
			}

			if (cmd.find("对话") < 16 && adminConverFlag == -1) {
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
					adminConverFlag = 233;
				}
				else {
					CQ_addLog(ac, CQLOG_ERROR, "运行环境", "指令分析失败");
					CQ_sendPrivateMsg(ac, AdminQQ, "指令分析失败");
					adminConverFlag = -1;
				}
			}
		}
		else {// 别人私聊的内容都转发到主号并转发主号的应答
			// 测试阶段先直接转发
			char buff[128];
			sprintf_s(buff, "来自%I64d的私聊消息(%d):", fromQQ, msgId);
			CQ_sendPrivateMsg(ac, AdminQQ, buff);

			// 然后存到数据库里让后端程序处理
			string sql;
			char* tmp_cov = (char*)malloc(sizeof(char) * 128 + 8);
			strcpy_s(tmp_cov, 128, cmd.substr(0, 127).c_str());
			escapeCharA(&tmp_cov);
			sql = "INSERT INTO `main`.`event` (`TYPE`, `LINK`, `CONT`, `NOTE`, `STATUS`)"\
				"VALUES(6001," + to_string(msgId) + ", '" + tmp_cov + "', '" + "来自私聊, 调试阶段统一人工转发" + "', 301); ";
			fileLog(typ_db, sql);
			freeString(tmp_cov);

			CQ_sendPrivateMsg(ac, AdminQQ, cmd.c_str()); // 放在后面一是增加消息间隔减小被封的可能性二是能辅助观察数据库插入是否成功
		}
		respons("on_PrivateMsg");

		double time_end = clock(); // 测试用
		double dur_msgproc = (double)((time_end - time_start) / CLOCKS_PER_SEC);
		char buff[128];
		sprintf_s(buff, "私聊消息处理耗时→%.3f秒", dur_msgproc);
		CQ_addLog(ac, CQLOG_DEBUG, "运行时长", buff);
	}

	//debug MessageBoxA(NULL, "CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char* msg, int32_t font)", "debug", 0);
	return EVENT_IGNORE;
}


/*
* Type=2 群消息
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char* fromAnonymous, const char* msg, int32_t font) {
	if (enabled) {
		double time_start = clock(); // 测试用

		GroupCounter += 1;

		string grpMsg = msg;

		if (adminConverFlag == 233 && toGp == fromGroup) {
			pBuff = to_string(fromQQ) + "(" + to_string(msgId) + "):\n" + grpMsg;
			CQ_sendPrivateMsg(ac, AdminQQ, pBuff.c_str());
			return EVENT_IGNORE;
		}

		if (fromGroup == 962362386) { // debug, 后端做了
			return EVENT_IGNORE;
		}

		if (grpMsg.find("[CQ:at,qq=") != string::npos) {
			if (grpMsg.find("qq=616471607") != string::npos || grpMsg.find("qq=2154055060") != string::npos || grpMsg.find("qq=2139223150") != string::npos) {  // 以后做成读配置的

				// 在群里被艾特的内容都转发到主号并转发主号的应答
				// 测试阶段先直接转发

				char buff[128];
				sprintf_s(buff, "来自%I64d在群(%I64d)的艾特(%d):", fromQQ, fromGroup, msgId);
				CQ_sendPrivateMsg(ac, AdminQQ, buff);

				// 然后存到数据库里让后端程序处理
				string sql;
				char* tmp_cov = (char *)malloc(sizeof(char) * 128 + 8);
				strcpy_s(tmp_cov, 128, grpMsg.substr(0, 127).c_str());
				escapeCharA(&tmp_cov);
				sql = "INSERT INTO `main`.`event` (`TYPE`, `LINK`, `CONT`, `NOTE`, `STATUS`)"\
					"VALUES(6002," + to_string(msgId) + ", '" +tmp_cov + "', '" + "来自群艾特, 调试阶段统一人工转发" + "', 300); ";
				fileLog(typ_db, sql);
				freeString(tmp_cov);

				CQ_sendPrivateMsg(ac, AdminQQ, grpMsg.c_str());
				respons("on_GroupMsgAT");
			}
		}

		double time_end = clock(); // 测试用
		double dur_grpproc = (double)((time_end - time_start) / CLOCKS_PER_SEC);
		char buff[128];
		sprintf_s(buff, "群消息处理耗时→%.3f秒", dur_grpproc);
		CQ_addLog(ac, CQLOG_DEBUG, "运行时长", buff);
	}

	//debug MessageBoxA(NULL, "CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char* fromAnonymous, const char* msg, int32_t font)", "debug", 0);
	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=4 讨论组消息
*/
// 讨论组似乎已经被废弃了? 实际创建多人聊天发现也算在群消息里
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t msgId, int64_t fromDiscuss, int64_t fromQQ, const char* msg, int32_t font) {

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
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char* msg, const char* responseFlag) {

	pBuff += "(" + to_string(fromQQ) + ")请求添加好友, 附言`" + msg + "`. 已默认同意.";
	CQ_sendPrivateMsg(ac, AdminQQ, pBuff.c_str());
	CQ_addLog(ac, CQLOG_INFO, "好友请求", pBuff.c_str());
	CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");

	// 加好友就直接同意, 然后写数据库里让后端AI分析人际关系
	// level越小权限越高, amity越大越友好. 初始给个适中值, 由后端AI调整
	pBuff = CQ_getStrangerInfo(ac, fromQQ, TRUE);
	string sql = "INSERT INTO `main`.`relationship` (`QQ`, `nickname`, `level`, `amity`) VALUES(" + to_string(fromQQ) + ", '" + pBuff + "', 250, 10);";
	fileLog(typ_db, sql);

	respons("on_Request_AddFriend");

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=302 请求-群添加
* subType 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char* msg, const char* responseFlag) {

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
CQEVENT(int32_t, __reFindBG, 0)() {
	HWND nPywd;
	char buff[512];

	CQ_addLog(ac, CQLOG_DEBUG, "用户操作", "重新查找后端窗口...");

	nPywd = FindWindow(NULL, PyTitle);

	if (nPywd) {
		if (nPywd != Pywd) {
			sprintf_s(buff, "后端窗口句柄发生了变化:  新窗口句柄 ==> 0x%p; 之前的记录 == > 0x%p\n更新记录...", Pywd, nPywd);
			Pywd = nPywd;
			PostMessage(Pywd, WM_USER, HandSk, 0x01);
			CQ_addLog(ac, CQLOG_INFO, "运行环境", buff);
			MessageBoxA(NULL, buff, "查找结果", 0);
		}
		else {
			TCHAR title[64];
			GetWindowText(CQwd, title, 60);
			sprintf_s(buff, "后端窗口句柄没有变化:  窗口句柄 ==> 0x%p; 窗口标题 == > %ws", CQwd, title);
			MessageBoxA(NULL, buff, "查找结果", 0);
		}
	}
	else {
		ErrorCounter += 1;
		CQ_addLog(ac, CQLOG_ERROR, "运行环境", "后端通信窗口不见了?! ");
		MessageBoxA(NULL, "后端通信窗口不见了?! ", "查找结果", 0);
	}
	
	return 0;
}

CQEVENT(int32_t, __cheakWindow, 0)() {
	if (CQwd == nullptr) CQwd = createMainWindow(WndProc);
	else {
		char buff[512];
		TCHAR title[64];
		GetWindowText(CQwd, title, 60);
		sprintf_s(buff, "窗口已经存在:  窗口句柄 ==> 0x%p; 窗口标题 == > %ws", CQwd, title);
		CQ_addLog(ac, CQLOG_INFO, "窗口程序", buff);
		return 200;
	}
	if (!CQwd) {
		CQ_addLog(ac, CQLOG_ERROR, "运行环境", "因未知原因, 窗口创建失败. 可考虑重启.");
		return -233;
	}
	else {
		char buff[512];
		TCHAR title[64];
		GetWindowText(CQwd, title, 60);
		sprintf_s(buff, "窗口创建成功:  窗口句柄 ==> 0x%p; 窗口标题 == > %ws", CQwd, title);
		CQ_addLog(ac, CQLOG_INFOSUCCESS, "窗口程序", buff);
		return 302;
	}
	return 0;
}

CQEVENT(int32_t, __manualSync, 0)() {
	ErrorCounter = 0;
	respons("on_ManualTrigger");
	return 0;
}

// ---------------------------------------------
// ================主要功能==================
void respons(const char* eve) {
	double time_start = clock(); // 测试用
	if (time(NULL) - Time_lastTrigger > 0) {
		GtmpCounter += 1; Heartbeat -= 1;

		char buff[128];
		int tmp = -1;
		int param = 0;

		if (eve != "onBackend") {
			PostMessage(Pywd, WM_USER, AppUpate, 0x0002);
			sprintf_s(buff, "被动触发(%s)成功", eve);
			CQ_addLog(ac, CQLOG_DEBUG, "后台处理", buff);
			param = AppUpate;
		}
		else if (eve == "onBackend") {
			CQ_addLog(ac, CQLOG_DEBUG, "后台处理", "被后端通信唤醒...");
			param = AppSync;
		}
		TdHandle[0] = CreateThread(NULL, 0, polling, &param, NULL, &ThreadId[0]);
		sprintf_s(buff, "`polling`: Handle ==> 0x%p (%ld); Id ==> 0x%ld (%ld)", TdHandle[0], (ULONG)TdHandle[0], ThreadId[0], ThreadId[0]);
		CQ_addLog(ac, CQLOG_DEBUG, "线程创建", buff);
	}
	else {
		ErrorCounter += 1;
		CQ_addLog(ac, CQLOG_WARNING, "触发频率过高", "检测到不正常的触发间隔,不过也有可能是巧合.");
	}
	if (ErrorCounter > 5) {
		CQ_addLog(ac, CQLOG_FATAL, "异常触发", "触发频率异常高");
		CQ_setFatal(ac, "触发频率异常高");
	}
	if (GtmpCounter > 11 && GtmpCounter % 22 == 0) { // 正常了那么多次那就说明之前是巧合错误, 清零计数
		ErrorCounter = 0;
	}
	Time_lastTrigger = time(NULL);

	double time_end = clock(); // 测试用
	double dur_respons = (double)((time_end - time_start) / CLOCKS_PER_SEC);
	char buff[128];
	sprintf_s(buff, "应答程序耗时→%.3f秒", dur_respons);
	CQ_addLog(ac, CQLOG_DEBUG, "运行时长", buff);
}



DWORD WINAPI polling(LPVOID p) {
	// 这里打算定时轮训数据库, 查找并处理后端程序的指令
	// 查询数据库里后端程序给的指令(状态码233)并解释执行, 如果成功则把状态码改为0, 否则改为500

	double time_start = clock(); // 测试用

	int *from = (int *)p;

	EnterCriticalSection(&g_csVar);

	if (db == nullptr) {
		rc = sqlite3_open(dbPath.c_str(), &db);
	}

	const char* sql = "SELECT `EID`, `CONT` from main.event WHERE STATUS = 233;";
	sqlite3_stmt* stmt = nullptr;
	int status = -1;
	char buff[256];
	int res = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	int affectedRows = 0;

	if (res != SQLITE_OK) {
		if (stmt)
			sqlite3_finalize(stmt);
		sqlite3_close(db);
		pBuff = "读数据库错误(" + to_string(rc) + "):";
		CQ_addLog(ac, CQLOG_ERROR, "运行环境", pBuff.c_str());
		LeaveCriticalSection(&g_csVar);
		return -1;
	}

	CQ_addLog(ac, CQLOG_DEBUG, "数据库调试", "开始select操作(SELECT `EID`, `CONT` from main.event WHERE STATUS = 233;)");

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		affectedRows += 1;
		CQ_addLog(ac, CQLOG_DEBUG, "命令处理", "找到后端命令, 开始处理");
		char* _tmpStr = utf8ToACP((char*)sqlite3_column_text(stmt, 1));
		oppositeEscapeCharA(_tmpStr);
		status = cmdExec(_tmpStr);
		delete _tmpStr;
		if (status > OK) {
			sprintf_s(buff, "UPDATE `event` SET `STATUS` = 0 , `note` = 'exec successed(%d): %s' WHERE `EID` = %d", status, (char*)sqlite3_column_text(stmt, 1), sqlite3_column_int(stmt, 0));
			rc = sqlite3_exec(db, buff, NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				sprintf_s(buff, "后端命令执行成功, 但数据库更新失败(%d):%s", rc, zErrMsg);
				CQ_addLog(ac, CQLOG_WARNING, "数据库(命令状态)", buff);
			}
			else {
				CQ_addLog(ac, CQLOG_INFOSUCCESS, "数据库(命令状态)", "后端命令执行成功");
			}

		}
		else {
			sprintf_s(buff, "UPDATE `event` SET `STATUS` = 500 , `note` = 'exec failed(%d): %s' WHERE `EID` = %d", status, (char*)sqlite3_column_text(stmt, 1), sqlite3_column_int(stmt, 0));
			rc = sqlite3_exec(db, buff, NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				sprintf_s(buff, "后端命令执行失败, 且据库更新失败(%d):%s", rc, zErrMsg);
				CQ_addLog(ac, CQLOG_ERROR, "数据库(命令状态)", buff);
			}
			else {
				CQ_addLog(ac, CQLOG_WARNING, "指令执行", "后端指令解析执行成功, 但执行未得到期望的返回值, 不确定执行情况");
			}
		}
		if (*from == AppSync) {  // 如果是后端让查的, 那就也给后端返回一下
			PostMessage(Pywd, WM_USER, CQreturn, status);
		}
		Sleep(500);  // 多条命令间适当延时, 以免高频操作造成异常
	}

	LeaveCriticalSection(&g_csVar);

	if (affectedRows) {
		sprintf_s(buff, "本次触发执行了 %d 条命令, 本次轮询处理线程(0x%p)退出. 开始睡觉...", affectedRows, TdHandle[0]);
		CQ_addLog(ac, CQLOG_DEBUG, "后台处理", buff);
	}
	else {
		sprintf_s(buff, "未找到后端命令, 本次轮询处理线程(0x%p)退出. 继续睡觉...", TdHandle[0]);
		CQ_addLog(ac, CQLOG_DEBUG, "后台处理", buff);
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	db = nullptr;
	CloseHandle(TdHandle[0]);
	TdHandle[0] = nullptr;

	double time_end = clock(); // 测试用
	double dur_polling = (double)((time_end - time_start) / CLOCKS_PER_SEC);
	sprintf_s(buff, "处理耗时→%.3f秒", dur_polling);
	CQ_addLog(ac, CQLOG_DEBUG, "运行时长", buff);

	return 0;
}

/*
	根据词法分析把语句执行相应的酷Q API
*/

int cmdExec(const char* strCmd) {

	double time_start = clock(); // 测试用

	CQcmd cmd;
	string command, sql;
	command = strCmd;
	cmd = mainParse(command);
	char* cmdContent = (char*)malloc(strlen(strCmd));

	CQ_addLog(ac, CQLOG_DEBUG, ":命令解析:", strCmd);

//	if (cmd.status < 2 || cmd.status > 8) { // 成分权重和范围
//		CQ_addLog(ac, CQLOG_DEBUG, "命令解析", "命令语法有误, 无法执行");
//		return Err;
//	}

	if (cmd.content != "Err") {
		regex parm1(" \\?([\\S ]+)\\?");
		smatch result;
		regex_search(command, result, parm1);
		strcpy_s(cmdContent, command.length(), result.str(1).c_str());
		//debug MessageBoxA(NULL, result.str(1).c_str(), "正则匹配结果", 0);  // debug
	}
	else {
		strcpy_s(cmdContent, command.length(), "Err");
	}
	

	pBuff = "";

	switch (cmd.cmdID)             // cmdID对应lex.cpp中keywords的索引
	{
	case 0:
		if (cmd.toGrp != -1)
			return CQ_sendGroupMsg(ac, cmd.toGrp, cmdContent);
		else if (cmd.toPri != -1) {
			if (cmdContent != "Err")
				return CQ_sendPrivateMsg(ac, cmd.toPri, cmdContent);
			if (cmd.flag == 100)
				return CQ_sendLike(ac, cmd.toPri);
		}
		break;

	case 1:
		switch (cmd.subCmdID)     // subCmdID对应MAIN.md里的示例顺序
		{
		case 0:
			return CQ_deleteMsg(ac, cmd.action);
			break;
		case 1:
			if (cmd.flag == 10)
				return CQ_setGroupKick(ac, cmd.toGrp, cmd.toPri, FALSE);
			else if (cmd.flag == 233)
				return CQ_setGroupLeave(ac, cmd.toGrp, FALSE);
			break;
		case 2:
			if (cmd.flag == 22)
				return CQ_setGroupWholeBan(ac, cmd.toGrp, cmd.action);
			else if (cmd.flag < 22)
				return CQ_setGroupBan(ac, cmd.toGrp, cmd.toPri, cmd.action);
			break;
		case 3:
			return CQ_setGroupAnonymous(ac, cmd.toGrp, cmd.action);
			break;
		case 4:
			return CQ_setGroupCard(ac, cmd.toGrp, cmd.toPri, cmdContent);
			break;
		case 5:
			return CQ_setGroupSpecialTitle(ac, cmd.toGrp, cmd.toPri, cmdContent, cmd.action);
			break;
		case 6:
			// 加好友/加群邀请默认同意就先不做了
			// TODO: 得先把后端和lex做好
			break;
		case 7:
			return CQ_addLog(ac, CQLOG_INFO, "后台处理", cmdContent);
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
			break;
		case 9:
			pBuff = CQ_getRecord(ac, cmdContent, "mp3");
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
			pBuff = "";
			break;
		}
		if (pBuff != "") {
			sql = "INSERT INTO `main`.`event` (`TYPE`, `CONT`, `NOTE`, `STATUS`)"\
				"VALUES(1080, '" + pBuff + "', 'GetInfo', 288); ";
			fileLog(typ_db, pBuff);
		}
		break;
	case 4:
		if (cmd.action > 10000) {
			char buff[256];
			pBuff = to_string(cmd.action);
			blackList = pBuff + ", ";
			sprintf_s(buff, "已将`%s`添加到黑名单", to_string(cmd.action).c_str());
			CQ_sendPrivateMsg(ac, AdminQQ, buff);
		}
		else if (cmd.action == -1) {
			char buff[512];
			if (blackList.length() < 256)
				sprintf_s(buff, "当前黑名单列表:\n%s", blackList.c_str());
			else
				sprintf_s(buff, "当前黑名单列表:\n%s......", blackList.substr(0, 256).c_str());
			CQ_sendPrivateMsg(ac, AdminQQ, buff);
		}
		return OK;
	default:
		break;
	}

	freeString(cmdContent);

	double time_end = clock(); // 测试用
	double dur_parase = (double)((time_end - time_start) / CLOCKS_PER_SEC);
	char buff[128];
	sprintf_s(buff, "命令解析耗时→%.3f秒", dur_parase);
	CQ_addLog(ac, CQLOG_DEBUG, "运行时长", buff);

	return Unknown;
}

int fileLog(int logType, string msg) {
	FILE* flog;
	char buff[1024];
	time_t curTime = time(NULL);

	switch (logType) {
	case typ_sLog:
		pBuff = APPpath + "status.log";
		fopen_s(&flog, pBuff.c_str(), "w");
		if (flog) {
			sprintf_s(buff, msg.c_str(), curTime);
			fwrite(buff, 1, strlen(buff), flog);
			fclose(flog);
			return OK;
		}
		return Err;
	case typ_dLog:
		fopen_s(&flog, "debug.log", "a");
		if (flog) {
			pBuff = "[debug] %I64d |" + msg;
			sprintf_s(buff, pBuff.c_str(), time(NULL));
			fwrite(buff, 1, strlen(buff), flog);
			fclose(flog);
			return OK;
		}
		return Err;
	case typ_db:
	{
		EnterCriticalSection(&g_csVar);
		rc = sqlite3_open(dbPath.c_str(), &db);
		pBuff = "数据库(" + dbPath + ")连接状态→" + to_string(rc);
		CQ_addLog(ac, CQLOG_DEBUG, "数据库", pBuff.c_str());

		char* _tmpStr;
		_tmpStr = acpToUTF8(msg.c_str());
//		escapeCharA(&_tmpStr);
		rc = sqlite3_exec(db, _tmpStr, NULL, NULL, &zErrMsg);
		freeString(_tmpStr);
		if (rc != SQLITE_OK) {
			sprintf_s(buff, "`%s`,\nSQL(\n%s\n)执行失败", zErrMsg, msg.c_str());
			CQ_addLog(ac, CQLOG_ERROR, "SQL执行", buff);
			sqlite3_free(zErrMsg);
		}
		else if (rc == SQLITE_OK) {
			sprintf_s(buff, "SQL(\n%s\n)执行成功", msg.c_str());
			CQ_addLog(ac, CQLOG_DEBUG, "SQL执行", buff);
		}
		if (db) {
			sqlite3_close(db);
			db = nullptr;
		}
		LeaveCriticalSection(&g_csVar);
		return rc;
	}
	default:
		break;
	}
	return Unknown;
}

/* 初始化时的数据库自检 */
int dbCheck() {
	EnterCriticalSection(&g_csVar);
	rc = sqlite3_open(dbPath.c_str(), &db);
	pBuff = "数据库(" + dbPath + ")连接状态→" + to_string(rc);
	CQ_addLog(ac, CQLOG_DEBUG, "数据库自检", pBuff.c_str());

	if (rc == SQLITE_OK) {
		// 得先检查一下数据库是不是正确的, 不是的话就有可能是第一次使用, 就还得初始化一下
		bool dbIscorrect = FALSE;
		// 下面这段是抄来的
		char strFindTable[128];
		const char* sqlIni = "SELECT * FROM `sqlite_master` where type = 'table' and name = 'event'";
		sprintf_s(strFindTable, 127, sqlIni);
		sqlite3_stmt* stmt0 = nullptr;
		if (sqlite3_prepare_v2(db, strFindTable, -1, &stmt0, NULL) != SQLITE_OK) {
			if (stmt0)
				sqlite3_finalize(stmt0);
			sqlite3_close(db);
			int errCode = sqlite3_errcode(db);
			char buff[512];
			sprintf_s(buff, "数据库(%s)已连接(状态%d), 但执行初始化查询(%s)失败(%d)→ %s", dbPath.c_str(), rc, sqlIni, errCode, sqlite3_errmsg(db));
			CQ_addLog(ac, CQLOG_FATAL, "运行环境", buff);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		int r = sqlite3_step(stmt0);
		//判断表存在，结果集是否已到末尾
		//通过sqlite3_step命令执行创建表的语句。对于DDL和DML语句而言， sqlite3_step执行正确的返回值
		//只有SQLITE_DONE，对于SELECT查询而言，如果有数据返回SQLITE_ROW，当到达结果集末尾时则返回SQLITE_DONE。
		if (r == SQLITE_DONE) {
			dbIscorrect = FALSE;
			CQ_addLog(ac, CQLOG_WARNING, "数据库", "没有找到数据表, 准备初始化数据库...");
		}
		else if (r == SQLITE_ROW) {
			string dbCont = (char*)sqlite3_column_text(stmt0, 4);
			pBuff = "读到数据库表记录→ " + dbCont;
			CQ_addLog(ac, CQLOG_DEBUG, "数据库", pBuff.c_str());
			string orgSQL = SQL_init_eventTable;
			if (orgSQL.find(dbCont) != string::npos) {
				dbIscorrect = TRUE;
				CQ_addLog(ac, CQLOG_DEBUG, "数据库", "主数据库自检通过");
			}
		}

		sqlite3_finalize(stmt0);
		stmt0 = nullptr;
		if (db) {
			sqlite3_close(db);
			db = nullptr;
		}
		if (dbIscorrect) {
			LeaveCriticalSection(&g_csVar);
			return OK;
		}
	}
	LeaveCriticalSection(&g_csVar);
	return Err;
}

/* 第一次使用时创建数据库 */
int dbInit() {
	EnterCriticalSection(&g_csVar);
	rc = sqlite3_open(dbPath.c_str(), &db);
	pBuff = "数据库(" + dbPath + ")连接状态→" + to_string(rc);
	CQ_addLog(ac, CQLOG_DEBUG, "数据库", pBuff.c_str());

	if (rc == SQLITE_OK) {
		CQ_addLog(ac, CQLOG_INFO, "数据库初始化", "开始建表...");
		sqlite3_stmt* stmt1 = nullptr;
		// 开启一个用于初始化的事务
		rc = sqlite3_prepare_v2(db, SQL_begin, -1, &stmt1, NULL);
		if (rc != SQLITE_OK) {
			if (stmt1) sqlite3_finalize(stmt1);
			sqlite3_close(db);
			CQ_addLog(ac, CQLOG_ERROR, "数据库初始化", "初始化事务开启失败");
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		if (sqlite3_step(stmt1) != SQLITE_DONE) {
			sqlite3_finalize(stmt1);
			sqlite3_close(db);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		sqlite3_finalize(stmt1);
		// 事务开启成功

		sqlite3_stmt* stmt2 = nullptr;
		// 创建主事件表
		if (sqlite3_prepare_v2(db, SQL_init_eventTable, -1, &stmt2, 0)) {
			CQ_addLog(ac, CQLOG_ERROR, "数据库初始化", "创建事件表失败");
			if (stmt2) sqlite3_finalize(stmt2);
			sqlite3_close(db);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		if (sqlite3_step(stmt2) != SQLITE_DONE) {
			sqlite3_finalize(stmt2);
			sqlite3_close(db);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		sqlite3_finalize(stmt2);
		// 事件表创建成功

		// 创建人际关系表
		if (sqlite3_prepare_v2(db, SQL_init_relatTable, -1, &stmt2, 0)) {
			CQ_addLog(ac, CQLOG_ERROR, "数据库初始化", "创建人际关系表失败");
			if (stmt2) sqlite3_finalize(stmt2);
			sqlite3_close(db);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		if (sqlite3_step(stmt2) != SQLITE_DONE) {
			sqlite3_finalize(stmt2);
			sqlite3_close(db);
			LeaveCriticalSection(&g_csVar);
			return -1;
		}
		sqlite3_finalize(stmt2);
		// 关系表创建成功

		// 创建事件权重索引
		if (sqlite3_prepare_v2(db, SQL_init_priorIndex, -1, &stmt2, 0)) {
			CQ_addLog(ac, CQLOG_ERROR, "数据库初始化", "创建事件权重索引失败");
			if (stmt2) sqlite3_finalize(stmt2);
			sqlite3_close(db);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		if (sqlite3_step(stmt2) != SQLITE_DONE) {
			sqlite3_finalize(stmt2);
			sqlite3_close(db);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		sqlite3_finalize(stmt2);
		// 事件权重索引创建成功

		// 创建用户索引
		if (sqlite3_prepare_v2(db, SQL_init_powerIndex, -1, &stmt2, 0)) {
			CQ_addLog(ac, CQLOG_ERROR, "数据库初始化", "创建用户权限索引失败");
			if (stmt2) sqlite3_finalize(stmt2);
			sqlite3_close(db);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		if (sqlite3_step(stmt2) != SQLITE_DONE) {
			sqlite3_finalize(stmt2);
			sqlite3_close(db);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		sqlite3_finalize(stmt2);
		// 用户索引创建成功

		// 创建时间索引
		if (sqlite3_prepare_v2(db, SQL_init_activeIndex, -1, &stmt2, 0)) {
			CQ_addLog(ac, CQLOG_ERROR, "数据库初始化", "创建事件时间索引失败");
			if (stmt2) sqlite3_finalize(stmt2);
			sqlite3_close(db);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		if (sqlite3_step(stmt2) != SQLITE_DONE) {
			sqlite3_finalize(stmt2);
			sqlite3_close(db);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		sqlite3_finalize(stmt2);
		// 时间索引创建成功

		sqlite3_stmt* stmt3 = nullptr;
		// 提交事务
		if (sqlite3_prepare_v2(db, SQL_commit, -1, &stmt3, NULL)) {
			if (stmt3) sqlite3_finalize(stmt3);
			sqlite3_close(db);
			CQ_addLog(ac, CQLOG_ERROR, "数据库初始化", "初始化事务提交失败");
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
		sqlite3_finalize(stmt3);
		// 事务提交成功

		rc = sqlite3_exec(db, "COMMIT", NULL, NULL, NULL); // 保存数据库更改
		if (rc == SQLITE_OK) {
			CQ_addLog(ac, CQLOG_INFOSUCCESS, "数据库(初始化)", "数据库初始化提交成功");
			LeaveCriticalSection(&g_csVar);
			return rc;
		}
		else {
			char buff[512];
			sprintf_s(buff, "因为%s,数据库初始化提交失败(%d)", zErrMsg, rc);
			CQ_addLog(ac, CQLOG_FATAL, "数据库(初始化)", buff);
			sqlite3_free(zErrMsg);
			LeaveCriticalSection(&g_csVar);
			return Err;
		}
	}
	LeaveCriticalSection(&g_csVar);
	return Unknown;
}

/*
* 窗口消息回调函数. 这里用来接收来自中间层的窗口消息
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_USER:
		WPA = wParam;
		LPA = lParam;
		Heartbeat = 5;
		if (WPA != Breath) {
			char buff[128];
			sprintf_s(buff, "| 0x%p | typ: 0x%x; wParam: 0x%x; lParam: 0x%x \n", hWnd, msg, wParam, lParam);
			CQ_addLog(ac, CQLOG_INFO, "通信消息", buff);
		}
		switch (wParam) {
		case AppSync:
			respons("onBackend");
			if (ErrorCounter > 0) {
				ErrorCounter -= 1;
			}
			break;
		case 0x200:
			if (Pywd != nullptr) {
				if (LPA == 0x11) {
					CQ_addLog(ac, CQLOG_INFOSUCCESS, "运行环境", "后端应用握手成功");
				}
			}
			break;
		case AppExit:
			SendMessage(CQwd, WM_CLOSE, 0, 0);
			break;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		DestroyWindow(CQwd);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;

	return (LRESULT)0;
}

DWORD WINAPI breath(LPVOID p) {
	while (enabled) {
		PostMessage(Pywd, WM_USER, Breath, 0x200);
		Sleep(1000);
		if (Heartbeat < 1) {
			CQ_addLog(ac, CQLOG_WARNING, "运行环境", "后端通信服务似乎是掉线了... 请检查.");
			Heartbeat = 5;
		}
	}
	CloseHandle(TdHandle[1]);
	TdHandle[1] = nullptr;
	
	//	pBuff = "线程`breath`(handle" + to_string(__threadhandle()) + " -- threadid" + to_string(__threadid()) + ") 退出";
	//  MessageBoxA(NULL, pBuff.c_str(), "debug", 0);  // debug
	return 0;
}
// ========================================