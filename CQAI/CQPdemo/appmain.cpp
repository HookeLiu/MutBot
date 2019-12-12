/*
*	17ds8�๦��AI��QQ�ӿ�. ѡ�õ��ǿ�Q�ٷ�SDK��VC++�汾, ��Ϊ����汾�ķ�װ������ӽ��ײ�, API�ͽṹ������. û��ѡ�������Ե�һ��ԭ����������ͨ�ÿ���(�ؼ�ԭ����������Ҫ�����ҽ�����׸, ����Ҳ��ϰ�����������ֺ�����������ʽ)
*	����๦��AI��Ŀ�ı������ں�˴������,
*	���VC++���̻���ֻ���ڿ�Q�ٷ�����SDK����һ�����׵����ݽӿ�, ��Ϊ���AI�����QQ�û�������.
*	���Ի����Ͼ���clone�������˵�����, Ҳûɶ�Ķ�. �����Ҫ����Python, ��Ϣ������������������.
*	(��Ҫע�����, ��Q����������õĶ���GB2312, ���ݿ��Լ���˳����õĶ���UTF-8, 
*	�����������˸����ϳ����ļ���ת�뺯��).
*	By ��֮�� Dec. 2019
*/
/*
* CoolQ Demo for VC++ 
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/

#include "stdafx.h"
#include "string"
#include "cqp.h"
#include "appmain.h" //Ӧ��AppID����Ϣ������ȷ��д�������Q�����޷�����

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

const int64_t AdminQQ = 2139223150; // �Ժ����ͨ���������ļ�֮��ķ�ʽ����̬����

int ac = -1; //AuthCode ���ÿ�Q�ķ���ʱ��Ҫ�õ�
bool enabled = false;


/* 
* ����Ӧ�õ�ApiVer��Appid������󽫲������
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/* 
* ����Ӧ��AuthCode����Q��ȡӦ����Ϣ��������ܸ�Ӧ�ã���������������������AuthCode��
* ��Ҫ�ڱ��������������κδ��룬���ⷢ���쳣���������ִ�г�ʼ����������Startup�¼���ִ�У�Type=1001����
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}


/*
* Type=1001 ��Q����
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q������ִ��һ�Σ���������ִ��Ӧ�ó�ʼ�����롣
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventStartup, 0)() {

	APPpath = CQ_getAppDirectory(ac);
	pBuff = "��ǰӦ��Ŀ¼��" + APPpath;
	CQ_addLog(ac, CQLOG_DEBUG, "���л���", pBuff.c_str());

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
			pBuff = "��Q������¼�ļ�����Ϊ" + to_string(len_file) + ", ���ܴ�������";
			CQ_addLog(ac, CQLOG_WARNING, "���л���", pBuff.c_str());
		}
		else {
			char sta[2], tim[11];
			fseek(flog, 0, SEEK_SET);
			fread(&sta, 1, 1, flog);
			sta[1] = '\0';
			fread(&tim, 1, 10, flog);
			tim[10] = '\0';
			sprintf_s(buff, "��Q�����ɹ�, �ϴμ�¼״̬��%s, �ϴμ�¼ʱ���%s", sta, tim);
			CQ_addLog(ac, CQLOG_DEBUG, "���л���", buff);
		}
		fseek(flog, 0, SEEK_SET);
		sprintf_s(buff, "1 %I64d", curTime);
		fwrite(buff, 1, strlen(buff), flog);
		fclose(flog);
	}
	else {
			CQ_addLog(ac, CQLOG_ERROR, "���л���", "״̬�ļ���ʧ��, ����Ŀ¼�Լ�Ȩ��");
			pBuff = "��ǰAPPĿ¼��" + APPpath;
			CQ_addLog(ac, CQLOG_INFO, "���л���", pBuff.c_str());
	}
	if(flog != NULL)
		fclose(flog);

	return 0;
}


/*
* Type=1002 ��Q�˳�
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q�˳�ǰִ��һ�Σ���������ִ�в���رմ��롣
* ������������Ϻ󣬿�Q���ܿ�رգ��벻Ҫ��ͨ���̵߳ȷ�ʽִ���������롣
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
* Type=1003 Ӧ���ѱ�����
* ��Ӧ�ñ����ú󣬽��յ����¼���
* �����Q����ʱӦ���ѱ����ã�����_eventStartup(Type=1001,��Q����)�����ú󣬱�����Ҳ��������һ�Ρ�
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventEnable, 0)() {
	enabled = true;

	char *outbuff;

	pBuff = CQ_getAppDirectory(ac) ;
	pBuff += "app.db";
	rc = sqlite3_open(pBuff.c_str(), &db);
	pBuff = "���ݿ�(" + pBuff + ")����״̬��" + to_string(rc);
	CQ_addLog(ac, CQLOG_DEBUG, "���ݿ�", pBuff.c_str());

	if ( rc == SQLITE_OK ) {
		// ���ȼ��һ�����ݿ��ǲ�����ȷ��, ���ǵĻ����п����ǵ�һ��ʹ��, �ͻ��ó�ʼ��һ��
		bool dbIscorrect = FALSE;
		// ��������ǳ�����
		char strFindTable[128];
		sprintf_s(strFindTable, 127, "SELECT * FROM `sqlite_master` where type = 'table' and name = 'event'");
		sqlite3_stmt* stmt0 = NULL;
		if (sqlite3_prepare_v2(db, strFindTable, strlen(strFindTable), &stmt0, NULL) != SQLITE_OK) {
			if (stmt0) {
				sqlite3_finalize(stmt0);
				sqlite3_close(db);
				CQ_addLog(ac, CQLOG_ERROR, "���л���", "���ݿ��ѯ��ʼ������");
			}
		}
		int r = sqlite3_step(stmt0);
		//�жϱ���ڣ�������Ƿ��ѵ�ĩβ
		//ͨ��sqlite3_step����ִ�д��������䡣����DDL��DML�����ԣ� sqlite3_stepִ����ȷ�ķ���ֵ
		//ֻ��SQLITE_DONE������SELECT��ѯ���ԣ���������ݷ���SQLITE_ROW������������ĩβʱ�򷵻�SQLITE_DONE��
		if (r == SQLITE_DONE) {
			dbIscorrect = FALSE;
			CQ_addLog(ac, CQLOG_WARNING, "���ݿ�", "û���ҵ����ݱ�, ׼����ʼ�����ݿ�...");
		}
		else if (r == SQLITE_ROW) 
			dbIscorrect = TRUE;
		// ��������ǳ�����

		// ��һ����app.db�Ľṹ
		if (dbIscorrect != TRUE) {
			rc = sqlite3_exec(db, "CREATE TABLE  `event` (`EID` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,`TYPE` INTEGER NOT NULL,`TIME` NUMERIC NOT NULL DEFAULT CURRENT_TIMESTAMP,`LINK` INTEGER,`CONT` TEXT,`NOTE` TEXT,`STATUS` INTEGER);", NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				pBuff = "���ݿ��ʼ��ʧ��(" + to_string(rc) + "):" + zErrMsg;
				CQ_addLog(ac, CQLOG_ERROR, "���л���", pBuff.c_str());
			}
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			rc = sqlite3_exec(db, "CREATE TABLE `relationship` (`QQ` INTEGER NOT NULL UNIQUE,`Nickname` TEXT NOT NULL,`level` INTEGER NOT NULL,`amity` INTEGER NOT NULL,`from` TEXT,`note` TEXT,`lastActiv` NUMERIC DEFAULT CURRENT_TIMESTAMP,PRIMARY KEY(`QQ`));", NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				pBuff = "���ݿ��ʼ��ʧ��(" + to_string(rc) + "):" + zErrMsg;
				CQ_addLog(ac, CQLOG_ERROR, "���л���", pBuff.c_str());
			}
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			rc = sqlite3_exec(db, "CREATE INDEX `Activ` ON `relationship` (`lastActiv` DESC);", NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				pBuff = "���ݿ��ʼ��ʧ��(" + to_string(rc) + "):" + zErrMsg;
				CQ_addLog(ac, CQLOG_ERROR, "���л���", pBuff.c_str());
			}
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			rc = sqlite3_exec(db, "CREATE INDEX `power` ON `relationship` (`level` ASC);", NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				pBuff = "���ݿ��ʼ��ʧ��(" + to_string(rc) + "):" + zErrMsg;
				CQ_addLog(ac, CQLOG_ERROR, "���л���", pBuff.c_str());
			}
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			rc = sqlite3_exec(db, "CREATE INDEX `priority` ON `event` (`STATUS` DESC);", NULL, NULL, &zErrMsg);
			if (rc != SQLITE_OK) {
				pBuff = "���ݿ��ʼ��ʧ��(" + to_string(rc) + "):" + zErrMsg;
				CQ_addLog(ac, CQLOG_ERROR, "���л���", pBuff.c_str());
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
			char hint[] = { "SQL(\n%s\n)ִ�гɹ�" };
			int len_buff = strlen(sql) + strlen(hint);
			outbuff = (char*)malloc(len_buff + 8);
			if (outbuff != NULL) {
				sprintf_s(outbuff, len_buff, hint, sql);
				CQ_addLog(ac, CQLOG_DEBUG, "���ݿ�", outbuff);
			}
			free(outbuff);
			outbuff = NULL;
		}
		else {
			char hint[] = { "��Ϊ%s,\nSQL(\n%s\n)ִ��ʧ��" };
			int len_buff = strlen(sql) + strlen(hint);
			outbuff = (char*)malloc(len_buff + 8);
			if (outbuff != NULL) {
				sprintf_s(outbuff, len_buff, hint, zErrMsg, sql);
				CQ_addLog(ac, CQLOG_ERROR, "���ݿ�", outbuff);
				sqlite3_free(zErrMsg);
			}
			free(outbuff);
			outbuff = NULL;
		}
	}
	else {
		pBuff = "���ݿ�����ʧ��(" + to_string(rc) + ")";
		CQ_addLog(ac, CQLOG_ERROR, "���л���", pBuff.c_str());
	}

	respons("on_Enable");

	thread Timer60s(respTimer);
	Timer60s.detach();

	return 0;
}


/*
* Type=1004 Ӧ�ý���ͣ��
* ��Ӧ�ñ�ͣ��ǰ�����յ����¼���
* �����Q����ʱӦ���ѱ�ͣ�ã��򱾺���*����*�����á�
* ���۱�Ӧ���Ƿ����ã���Q�ر�ǰ��������*����*�����á�
*/
CQEVENT(int32_t, __eventDisable, 0)() {
	enabled = false;
	return 0;
}


/*
* Type=21 ˽����Ϣ
* subType �����ͣ�11/���Ժ��� 1/��������״̬ 2/����Ⱥ 3/����������
*/

// ��Ҫ��Ӧ��ظ����ɺ�����ɵ�, ����ֻ���˽�ĺ�Ⱥ�������������ȼ����¼���һЩת����ָ���.

CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char *msg, int32_t font) {

	//���Ҫ�ظ���Ϣ������ÿ�Q�������ͣ��������� return EVENT_BLOCK - �ضϱ�����Ϣ�����ټ�������  ע�⣺Ӧ�����ȼ�����Ϊ"���"(10000)ʱ������ʹ�ñ�����ֵ
	//������ظ���Ϣ������֮���Ӧ��/�������������� return EVENT_IGNORE - ���Ա�����Ϣ

	string cmd = msg;
	int32_t id = msgId;
	

	if (fromQQ == AdminQQ) {

		if (cmd.find("����ģʽ") != string::npos) {
			if (DEVflag == 233) {
				if (cmd.find("�˳�") != string::npos) {
					DEVflag = -1;
					CQ_sendPrivateMsg(ac, fromQQ, "���˳�����ģʽ");
					return EVENT_BLOCK;
				}
				else
					CQ_sendPrivateMsg(ac, fromQQ, "�Ѵ�������ģʽ, �����˳����˳�");
			}
			if (cmd.find("����") != string::npos && DEVflag == -1) {
				DEVflag = 233;
				CQ_sendPrivateMsg(ac, fromQQ, "�ѽ�������ģʽ");
				return EVENT_BLOCK;
			}
		}

		if (DEVflag == 233) {
			bool isExeced;
			isExeced = cmdExec(msg);
			if (isExeced == OK)
				CQ_sendPrivateMsg(ac, fromQQ, "������ִ��");
			else
				CQ_sendPrivateMsg(ac, fromQQ, "����ִ��ʧ��");
			return EVENT_BLOCK;
		}
			
		regex parm1("\\d{6,12}");
		smatch result;
		char buff[2048];

		if (cmd.find("����") < 16 && cmd.find(':') != string::npos) {
			unsigned int pos;
			pos = cmd.find(':');
	
			sprintf_s(buff, 2047,"����Ա����˽��ģʽ:%s��%d�ַ�, ���ݡ�\n%s", cmd.c_str(), cmd.length(), cmd.substr(pos).c_str());
			CQ_addLog(ac, CQLOG_INFO, "�������", buff);

			if (regex_search(cmd, result, parm1)) 
				toQQ = stoll(result.str(), 0, 10);
			if (toQQ != -1 && (cmd.size() > pos)) 
				CQ_sendPrivateMsg(ac, toQQ, cmd.substr(pos + 1).c_str());
			else {
				CQ_addLog(ac, CQLOG_ERROR, "���л���", "ָ�����ʧ��");
				pBuff = "����(\n" + cmd.substr(pos + 1) + "\n)����ʧ��";
				CQ_sendPrivateMsg(ac, AdminQQ, pBuff.c_str());
			}	
		}

		if ((cmd.find("����") < 6 || cmd.find("�˳�") < 6) && Gflag != -1) {
			if (Gflag == 233) {
				toQQ = -1;
				toGp = -1;
				Gflag = -1;
				CQ_sendPrivateMsg(ac, AdminQQ, "���˳��Ի�ģʽ");
			}
		}

		if (Gflag == 233 && toQQ != -1) 
			CQ_sendPrivateMsg(ac, toQQ, cmd.c_str());
		if (Gflag == 233 && toGp != -1)
			CQ_sendGroupMsg(ac, toGp, cmd.c_str());

		if (cmd.find("�Ի�") < 16 && Gflag == -1) {
			regex parm1("\\d{6,12}");
			smatch result;
			
			if (regex_search(cmd, result, parm1)) {
				if (cmd.find("Ⱥ") < 16 && toGp == -1) {
					toQQ = -1;
					toGp = stoll(result.str(), 0, 10);
					sprintf_s(buff, 2047, "\n����Ա�� %I64d ��Ⱥ�ĶԻ���ʼ", toGp);
					CQ_addLog(ac, CQLOG_INFO, "�������", "����Ⱥ�ĶԻ�ģʽ");
					CQ_sendPrivateMsg(ac, AdminQQ, "�ѽ���Ⱥ�ĶԻ�ģʽ");
				}
				else if (toQQ == -1) {
					toGp = -1;
					toQQ = stoll(result.str(), 0, 10);
					sprintf_s(buff, 2047, "\n����Ա�� %I64d ��˽�ĶԻ���ʼ", toQQ);
					CQ_addLog(ac, CQLOG_INFO, "�������", "����˽�ĶԻ�ģʽ");
					CQ_sendPrivateMsg(ac, AdminQQ, "�ѽ���˽�ĶԻ�ģʽ");
				}
				Gflag = 233;
			}
			else {
				CQ_addLog(ac, CQLOG_ERROR, "���л���", "ָ�����ʧ��");
				CQ_sendPrivateMsg(ac, AdminQQ, "ָ�����ʧ��");
				Gflag = -1;
			}
		}
	}
	else {// ����˽�ĵ����ݶ�ת�������Ų�ת�����ŵ�Ӧ��
		// ���Խ׶���ֱ��ת��
		char* obuff = NULL;
		char buff[256] = {'\0'};
		int len_buff = 0;

		char hint[] = { "����%I64d��˽����Ϣ:" };
		sprintf_s(buff, 255, hint, fromQQ);

		CQ_sendPrivateMsg(ac, AdminQQ, buff);

		// Ȼ��浽���ݿ����ú�˳�����
		string sql;

		sql = "INSERT INTO `main`.`event` (`TYPE`, `LINK`, `CONT`, `NOTE`, `STATUS`)"\
			"VALUES(6001," + to_string(id) + ", '" + G2U( cmd.substr(0, 128).c_str() ) + "', '" + G2U("����˽��, ���Խ׶�ͳһ�˹�ת��") + "', 301); ";

		rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
		if (rc == SQLITE_OK) {
			sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
			char hint[] = { "\nSQL(\n%s\n)ִ�гɹ�" };
			len_buff = sql.length() + strlen(hint);
			obuff = (char*)malloc(len_buff + 8);
			if (obuff != NULL) {
				sprintf_s(obuff, len_buff, hint, sql.c_str());
				CQ_addLog(ac, CQLOG_DEBUG, "���ݿ�", obuff);
			}
			free(obuff);
			obuff = NULL;
		}
		else {
			char hint[] = { "��Ϊ%s,\nSQL(\n%s\n)ִ��ʧ��" };
			len_buff = sql.length() + strlen(hint);
			obuff = (char*)malloc(len_buff + 8);
			if (obuff != NULL) {
				sprintf_s(obuff, len_buff, hint, zErrMsg, sql);
				CQ_addLog(ac, CQLOG_ERROR, "���ݿ�", obuff);
				sqlite3_free(zErrMsg);
			}
			else
				CQ_addLog(ac, CQLOG_ERROR, "���л���", "�ڴ����ʧ��");
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
* Type=2 Ⱥ��Ϣ
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
		if (grpMsg.find("qq=616471607") != string::npos || grpMsg.find("qq=2154055060") != string::npos || grpMsg.find("qq=2139223150") != string::npos) {  // �Ժ����ɶ����õ�
			CQ_sendGroupMsg(ac, fromGroup, "��~");

			// ��Ⱥ�ﱻ���ص����ݶ�ת�������Ų�ת�����ŵ�Ӧ��
			// ���Խ׶���ֱ��ת��
			char* obuff = NULL;
			char buff[256] = { '\0' };
			int len_buff = 0;

			char hint[] = { "����%I64d��Ⱥ(%I64d)�İ���:" };
			sprintf_s(buff, 255, hint, fromQQ, fromGroup);

			CQ_sendPrivateMsg(ac, AdminQQ, buff);

			// Ȼ��浽���ݿ����ú�˳�����
			string sql;

			sql = "INSERT INTO `main`.`event` (`TYPE`, `LINK`, `CONT`, `NOTE`, `STATUS`)"\
				"VALUES(6002," + to_string(msgId) + ", '" + G2U(grpMsg.substr(0, 128).c_str()) + "', '" + G2U("����Ⱥ, ���Խ׶�ͳһ�˹�ת��") + "', 301); ";

			rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
			if (rc == SQLITE_OK) {
				sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
				char hint[] = { "\nSQL(\n%s\n)ִ�гɹ�" };
				len_buff = sql.length() + strlen(hint);
				obuff = (char*)malloc(len_buff + 8);
				if (obuff != NULL) {
					sprintf_s(obuff, len_buff, hint, sql.c_str());
					CQ_addLog(ac, CQLOG_DEBUG, "���ݿ�", obuff);
				}
				free(obuff);
				obuff = NULL;
			}
			else {
				char hint[] = { "��Ϊ%s,\nSQL(\n%s\n)ִ��ʧ��" };
				len_buff = sql.length() + strlen(hint);
				obuff = (char*)malloc(len_buff + 8);
				if (obuff != NULL) {
					sprintf_s(obuff, len_buff, hint, zErrMsg, sql);
					CQ_addLog(ac, CQLOG_ERROR, "���ݿ�", obuff);
					sqlite3_free(zErrMsg);
				}
				else
					CQ_addLog(ac, CQLOG_ERROR, "���л���", "�ڴ����ʧ��");
				free(obuff);
				obuff = NULL;
			}
			free(obuff);
			obuff = NULL;

			CQ_sendPrivateMsg(ac, AdminQQ, grpMsg.c_str());
			respons("on_GroupMsgAT");
		}
	}
	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=4 ��������Ϣ
*/
// �������ƺ��Ѿ���������? ʵ�ʴ����������췢��Ҳ����Ⱥ��Ϣ��
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t msgId, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=101 Ⱥ�¼�-����Ա�䶯
* subType �����ͣ�1/��ȡ������Ա 2/�����ù���Ա
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=102 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/ȺԱ�뿪 2/ȺԱ���� 3/�Լ�(����¼��)����
* fromQQ ������QQ(��subTypeΪ2��3ʱ����)
* beingOperateQQ ������QQ
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=103 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/����Ա��ͬ�� 2/����Ա����
* fromQQ ������QQ(������ԱQQ)
* beingOperateQQ ������QQ(����Ⱥ��QQ)
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=201 �����¼�-���������
*/
CQEVENT(int32_t, __eventFriend_Add, 16)(int32_t subType, int32_t sendTime, int64_t fromQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=301 ����-�������
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag) {

	//CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");
	
	pBuff = CQ_getStrangerInfo(ac, fromQQ, TRUE);
	CQ_addLog(ac, CQLOG_INFO, "��������", pBuff.c_str());
	CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");
	
	// �Ӻ��Ѿ�ֱ��ͬ��, Ȼ��д���ݿ����ú��AI�����˼ʹ�ϵ
	// levelԽСȨ��Խ��, amityԽ��Խ�Ѻ�. ��ʼ��������ֵ, �ɺ��AI����
	string sql = "INSERT INTO `main`.`relationship` (`QQ`, `nickname`, `level`, `amity`) VALUES(" + to_string(fromQQ) + ", '" + pBuff.c_str() +"', 250, 10);";

	int len_buff;
	char* obuff;

	rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
	if (rc == SQLITE_OK) {
		sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
		char hint[] = { "\nSQL(\n%s\n)ִ�гɹ�" };
		len_buff = sql.length() + strlen(hint);
		obuff = (char*)malloc(len_buff + 8);
		if (obuff != NULL) {
			sprintf_s(obuff, len_buff, hint, sql.c_str());
			CQ_addLog(ac, CQLOG_DEBUG, "���ݿ�", obuff);
		}
		free(obuff);
		obuff = NULL;
	}
	else {
		char hint[] = { "��Ϊ%s,\nSQL(\n%s\n)ִ��ʧ��" };
		len_buff = sql.length() + strlen(hint);
		obuff = (char*)malloc(len_buff + 8);
		if (obuff != NULL) {
			sprintf_s(obuff, len_buff, hint, zErrMsg, sql);
			CQ_addLog(ac, CQLOG_ERROR, "���ݿ�", obuff);
			sqlite3_free(zErrMsg);
		}
		else
			CQ_addLog(ac, CQLOG_ERROR, "���л���", "�ڴ����ʧ��");
		free(obuff);
		obuff = NULL;
	}
	free(obuff);
	obuff = NULL;

	pBuff += "(" + to_string(fromQQ) + ")������Ӻ���, ����`" + msg + "`. ��Ĭ��ͬ��.";
	CQ_sendPrivateMsg(ac, AdminQQ, pBuff.c_str());
	respons("on_Request_AddFriend");

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=302 ����-Ⱥ���
* subType �����ͣ�1/����������Ⱥ 2/�Լ�(����¼��)������Ⱥ
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag) {

	//if (subType == 1) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPADD, REQUEST_ALLOW, "");
	//} else if (subType == 2) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
	//}

	if (subType == 2) {
		CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
		// Ⱥ�����ֱ�Ӽ�����
		pBuff = CQ_getStrangerInfo(ac, fromQQ, TRUE);
		pBuff += "(" + to_string(fromQQ) + ")�������Ⱥ`" + to_string(fromGroup) + "`, ��Ĭ��ͬ��";
		CQ_sendPrivateMsg(ac, AdminQQ, pBuff.c_str());
		respons("on_REQUEST_GROUPINVITE");
	}
		

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* �˵������� .json �ļ������ò˵���Ŀ��������
* �����ʹ�ò˵������� .json ���˴�ɾ�����ò˵�
*/
CQEVENT(int32_t, __menuA, 0)() {
	MessageBoxA(NULL, "����menuA�����������봰�ڣ����߽�������������", "" ,0);
	return 0;
}

CQEVENT(int32_t, __menuB, 0)() {
	MessageBoxA(NULL, "����menuB�����������봰�ڣ����߽�������������", "" ,0);
	return 0;
}

void respons(const char* eve) {
	time_t time_now = time(NULL);
	if (time_now - Time_lastTrigger > 10) {
		GtmpCounter += 1;
		char buff[128];
		int tmp = -1;
		sprintf_s(buff, "��������(%s)�ɹ�", eve);
		CQ_addLog(ac, CQLOG_DEBUG, "��̨����", buff);



		if (GtmpCounter < 10) { // ���Կ�Q Pro�����, ˳�ֵ����
			tmp += CQ_sendLike(ac, 616471607);
			tmp += CQ_sendLike(ac, AdminQQ);
			if (tmp != 0)
				GtmpCounter = 233;
			_ltoa_s(tmp, buff, 10);
			CQ_addLog(ac, CQLOG_DEBUG, "��Ƭ��", buff);
		}
	}
}

void polling() {
	// ������㶨ʱ��ѵ���ݿ�, ���Ҳ������˳����ָ��
}

void respTimer() {
	Timer timer;
	timer.start(60000, bind(respons, "on_Timer60s"));
	this_thread::sleep_for(chrono::hours(616471607));
	timer.stop();
}

/*
	������������һ����, �����ද������
*/

bool cmdExec(const char * strCmd) {
	CQcmd cmd;
	string command;
	command = strCmd;
	cmd = mainParse(command);

	if (cmd.status < 4 || cmd.status > 8) {
		CQ_sendPrivateMsg(ac, AdminQQ, "�����﷨����, �޷�ִ��");
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
			// �Ӻ���/��Ⱥ����Ĭ��ͬ����Ȳ�����
			break;
		case 7:
			CQ_addLog(ac, CQLOG_INFO, "��̨����", cmd.content);
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

// ������γ���https://www.cnblogs.com/cyberarmy/p/10098649.html
//UTF-8��GB2312��ת��
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

//GB2312��UTF-8��ת��
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
// ������γ���https://www.cnblogs.com/cyberarmy/p/10098649.html