/*
	# ����Lex����

	## ���ݱ���ԭ��д��һ���򵥵�����˷�������.

	��Ƶĸ�ʽ�����Դ�����ʽ:
	"��Ӧ���� �������� ������ϵ �������� ����ID �������� [�޶�����]"

	һ��ֻ����һ�����, �����з�Ϊ�Ͼ��, �������ַ�����?��Χ�����з������ǿ�ѡ, Ĭ�����ѡ��

	By ��֮�� Dec. 2019
*/

#include "stdafx.h"
#include "string"
#include <iostream>
std::int8_t stateTable[16][30] = // ���ݱ���ԭ�����������Զ���, ���ASCII�����29��״̬(��https://raw.githubusercontent.com/HookeLiu/IC-Notebook/master/RichHighLight/lex.png)
{                        // ��ʵgithub.com/HookeLiu/IC-Notebook��Lex�������д��, ������������ĵĴʷ����������Ծ�ֱ�ӳ����ð�...
	/*                   0    1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29 | -1���˳� */
	/*  +, -        */ {25, -1, -1, -1, -1, -1, -1, -1, 9, -1, 10, -1, 12, -1, 29, -1, -1, 17, 19, 19, 19, -1, 22, 22, -1, -1, 28, 27, 28, 29} ,
	/* *            */{25, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1, 12, -1, 29, -1, 18, 17, 19, 20, 19, -1, 23, 22, -1, -1, 28, 27, 28, 29},
	/* /            */{16, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1, 12, -1, 29, -1, 14, 17, 19, 19, 21, -1, 22, 24, -1, -1, 28, 27, 28, 29},
	/* \n           */{25, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 29, -1, -1, -1, 19, 19, 19, -1, 22, 22, -1, -1, 28, -1, -1, -1},
	/* x,X          */{15, 2, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1, 12, -1, 29, 15, -1, 17, 19, 19, 19, -1, 22, 22, -1, -1, 28, 27, 28, 29},
	/* e,E          */{15, -1, 3, 3, -1, 8, -1, -1, -1, -1, 10, -1, 12, -1, 29, 15, -1, 17, 19, 19, 19, -1, 22, 22, -1, -1, 28, 27, 28, 29},
	/* .            */{4, -1, -1, -1, -1, 6, -1, -1, -1, -1, 10, -1, 12, -1, 29, -1, -1, 17, 19, 19, 19, -1, 22, 22, -1, -1, 28, 27, 28, 29},
	/* !            */{25, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1, 12, -1, 17, -1, -1, 17, 22, 19, 19, -1, 22, 22, -1, -1, 27, 27, 28, 29},
	/* "            */{10, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, -1, 12, -1, 29, -1, -1, 17, 19, 19, 19, -1, 22, 22, -1, -1, 28, 27, 28, 29},
	/* '            */{12, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1, 13, -1, 29, -1, -1, 17, 19, 19, 19, -1, 22, 22, -1, -1, 28, 27, 28, 29},
	/* ident(G->++) */{15, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1, 12, -1, 29, 15, -1, 17, 19, 19, 19, -1, 22, 22, -1, -1, 28, 27, 28, 29},
	/* 1 - 9        */{5, 5, 3, 3, 7, 5, 7, 7, 7, 7, 10, -1, 12, -1, 29, 15, -1, 17, 19, 19, 19, -1, 22, 22, -1, -1, 28, 27, 28, 29},
	/* #            */{26, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1, 12, -1, 29, -1, -1, 17, 19, 19, 19, -1, 22, 22, -1, -1, 28, 27, 28, 29},
	/* 0            */{1, 5, 3, 3, 7, 5, 7, 7, 7, 7, 10, -1, 12, -1, 29, 15, -1, 17, 19, 19, 19, -1, 22, 22, -1, -1, 28, 27, 28, 29},
	/* a-f          */{15, -1, 3, 3, -1, -1, -1, -1, -1, -1, 10, -1, 12, -1, 29, 15, -1, 17, 19, 19, 19, -1, 22, 22, -1, -1, 28, 27, 28, 29},
	/* other        */{25, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1, 12, -1, 29, -1, -1, 17, 19, 19, 19, -1, 22, 22, -1, 25, 28, 27, 28, 29}
};

std::string keywords[] =         // Ԥ�������еĹؼ���, ��Ϊ�������������������Ľ�����, ���Ծ�ֻ�ÿ�Q API�Ĺؼ���, ֮���Դ���Ԥ����ʱֱ�Ӵֱ��滻...
{
	"send", "set",  "get",
	"msg", "grp", "user", "env",
	"self", "grpmb",
	"delete", "kick", "ban", "anonymous", "card", "specialTitle", "log", "info", "record", "list", "image", "nick", "qq", "directory",
	"request", "admin",    "like",
};

const int Len_keywords = (int)(sizeof(keywords) / sizeof(std::string));

/*��һ������ʱ������, ��ʵ�ֹ��ܰ�...
std::string preproccess(std::string cmd) { // ���÷Ѿ���, ֱ�ӱ����滻��
	int pos;
	std::string cmdWords[] = {
		"����", "��", "��ȡ", "ȡ", "����", "��", "��Ϣ", "Ⱥ, "
	};
}
*/

std::int8_t getCharType(const char c) {
	switch (c) {
	case '+': case '-':                 return 0;
	case '*':                           return 1;
	case '/':                           return 2;
	case '\n':                          return 3;
	case 'x': case 'X':                 return 4;
	case 'e': case 'E':					return 5;
	case '.':							return 6;
	case '!':							return 7;
	case '?':							return 8;
	case '\'':							return 9;
	case '_': case'g':case'h':case'i':case'j':case'k': \
	case'l':case'm':case'n':case'o':case'p':case'q': \
	case'r':case's':case't':case'u':case'v':case'w': \
	case'y':case'z':case'G':case'H':case'I':case'J':case'K': \
	case'L':case'M':case'N':case'O':case'P':case'Q': \
	case'R':case'S':case'T':case'U':case'V':case'W': \
	case'Y':case'Z':					return 10;
	case'1':case'2':case'3':case'4':case'5':case'6':case'7':\
	case'8':case'9':					return 11;
	case'#':							return 12;
	case'0':							return 13;
	case'A':case'B':case'C':case'D':case'F': \
	case'a':case'b':case'c':case'd':case'f': return 14; // �����ǰ�˳���. ��һ�в�����e,E, ��Ϊreturn 5 ��ʱ��ʹ�����.
	default:							return 15;
	}
}

struct strType
{
	std::string subStr = "";
	std::int8_t	state = -1;
	int pos = -1;
};

/*
 ���ַ�������ƥ��, ���ش������ʱ���Զ����ڵ�λ�ú��ַ���
*/
strType strTypeMatch(std::string str) {
	std::int8_t typ, i, len;
	std::string subStr = "";
	std::int8_t state = 0, curState = 0;
	strType stm;

	len = str.length();
	for (i = 0; i < len; i += 1) {
		typ = getCharType(str.at(i));
		if (str.at(i) == '\\') {
			i += 1;
			subStr += '\\';
			typ = getCharType(str.at(i));
		}
		curState = stateTable[typ][state];
		if (curState == -1)
			break;
		subStr += str.at(i);
		state = curState;
	}
	stm.subStr = subStr;
	stm.state = state;
	return stm;
}

/* stateTable��Ӧ�ĸ�������
  0                     1               2                  3
 ����                   hex����         һ������           ����,�Զ�����ķ���
  4                     5               6                  7
 ��������,�����Ƿ���    #ע��(Ԥ����)   // ע��            ����ע��
  8                     9               10                 11
 #!����                 //!����         /*!����            ��ʶ��
  12                    13              14
  '',�ַ�               "" �ַ���       �ؼ���1
*/
strType strHighlightType(std::int8_t state, std::string str) {
	int typ = 0;

	strType keywd;

	switch (state) {
	case 3:  typ = 1;  break;                       // [a~f],[A~F],[0~9]    
	case 1:                                         // 0
	case 5:                                         // [0~9]
	case 7:  typ = 2;  break;                       // [0~9]
	case 4:                                         // .                
	case 16: typ = 3;  break;                       // /                   
	case 25: typ = 4;  break;                       // [symbol]                           
	case 28: typ = 5;  break;                       // [!\n],else                                    
	case 14:                                        // /                
	case 29: typ = 6;  break;                       // [!\n],else                        
	case 21: typ = 7;  break;                       // /                        
	case 27: typ = 8;  break;                       // [!\n],!                
	case 17: typ = 9;  break;                       // [!\n],!                            
	case 24: typ = 10; break;                       // /                        
	case 15:                                        // ident,[![0~9]ident]  
		for (int i = 0; i < Len_keywords; i += 1) {
			if (keywords[i] == str) {				// �ж�str�ǲ���keylist��ָ���Ĺؼ���
				typ = 14;
				keywd.subStr = keywords[i];
				keywd.pos = i;
				break;
			}
			else typ = 11;
		}
		break;
	case 13: typ = 12; break;                       // '
	case 11: typ = 13; break;                       // "
	}
	keywd.state = typ;
	return keywd;
}

struct CQcmd
{
	std::int8_t		cmdID = -1;
	std::int8_t		subCmdID = -1;
	std::int8_t		flag = -1;
	std::int64_t	toPri = -1;
	std::int64_t	toGrp = -1;
	std::int32_t    action = -1;
	const char* content = "";
};

CQcmd mainParse(std::string cmd) {
	strType result_cmd;
	strType highType;
	static CQcmd code;
	int i = 0, len;

	len = cmd.length();
	while (1) {
		result_cmd = strTypeMatch(cmd);
		if (result_cmd.state == 0)
			break;
		i += result_cmd.subStr.length();

		cmd = cmd.substr(result_cmd.subStr.length(), len - i);

		highType = strHighlightType(result_cmd.state, result_cmd.subStr);

		if (highType.state == 14) {
			switch (highType.pos) {
			case 0: code.cmdID = 0; break;
			case 1: code.cmdID = 1; break;
			case 2: code.cmdID = 2; break;
			case 3:
				if (code.cmdID != -1) {
					code.toGrp = -1;
					code.toPri = 233;
				}
				break;
			case 4:
				if (code.cmdID != -1) {
					code.toGrp = -1;
					code.toPri = 233;
				}
				break;
			case 5:
				code.flag = 100;
				if (code.cmdID == 1) {

				}
				if (code.cmdID == 2) {

				}

			default:
				break;
			}
		}

		if (highType.state == 2) {
			if (result_cmd.subStr.length() > 5) {
				if (code.toPri == 233)
					code.toPri = std::stoll(result_cmd.subStr);
				if (code.toGrp == 233)
					code.toGrp = std::stoll(result_cmd.subStr);
			}
			else
				code.action = std::stoll(result_cmd.subStr);
		}

		if (highType.state == 13) {
			std::string cont = result_cmd.subStr.substr(1, result_cmd.subStr.length() - 2);
			char buff[8192];
			sprintf_s(buff, cont.c_str());
			code.content = buff;
		}
	}
	return code;
}