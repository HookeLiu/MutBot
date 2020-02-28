/*
	# 简易Lex解析

	## 根据编译原理写的一个简单的命令此法分析器.

	设计的格式化语言大致形式:
	"响应类型 内容类型 操作关系 操作对象 对象ID 操作内容 [限定条件]"

	一次只接受一条语句, 即换行符为断句符, 变量及字符串用?包围。其中方括号是可选, 默认最大选择。

	By 佚之狗 Dec. 2019
*/

#include "stdafx.h"
#include "string"
#include <iostream>
std::int8_t stateTable[16][30] = // 根据编译原理做的有限自动机, 针对ASCII码给出29个状态(见https://raw.githubusercontent.com/HookeLiu/IC-Notebook/master/RichHighLight/lex.png)
{                        // 其实github.com/HookeLiu/IC-Notebook的Lex还是燕儿写的, 懒得再设计中文的词法解析了所以就直接抄来用吧...
	/*                   0    1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29 | -1是退出 */
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

std::string keywords[] =         // 预设命令中的关键词, 因为不打算再造轮子做中文解析了, 所以就只用酷Q API的关键词, 之后的源语句预处理时直接粗暴替换...
{
	"send", "set",  "get",
	"msg", "grp", "user", "env",
	"self", "grpmb",
	"delete", "kick", "ban", "anonymous", "card", "specialTitle", "log", "info", "record", "list", "image", "nick", "qq", "directory",
	"request", "admin", "like", "blackList"
};

const int Len_keywords = (int)(sizeof(keywords) / sizeof(std::string));

/*这一部分暂时先做了, 先实现功能吧...
std::string preproccess(std::string cmd) { // 懒得费劲了, 直接暴力替换吧
	int pos;
	std::string cmdWords[] = {
		"设置", "置", "获取", "取", "发送", "发", "消息", "群, "
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
	case'a':case'b':case'c':case'd':case'f': return 14; // 返回是按顺序的. 这一行不处理e,E, 因为return 5 的时候就处理了.
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
 对字符串进行匹配, 返回处理完成时的自动机节点位置和字符串
*/
strType strTypeMatch(std::string str) {
	std::int8_t typ, i;
	std::uint16_t len;
	std::string subStr = "";
	std::int8_t state = 0, curState = 0;
	strType stm;
	bool overLenthFlag = false;

	if (str.length() < 1000)
		len = str.length();
	else {
		len = 996;
		str = str.substr(0, 990);
		overLenthFlag = true;
	}
	for (i = 0; i < len; i += 1) {
		typ = getCharType(str.at(i));
		if (str.at(i) == '\\') {  // 跳过转义符
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
	if (overLenthFlag == true) {
		subStr += "...";
	}
	stm.subStr = subStr;
	stm.state = state;
	return stm;
}

/* stateTable对应的高亮类型
  0                     1               2                  3
 错误                   hex数字         一般数字           符号,自动机里的符号
  4                     5               6                  7
 其它内容,可能是符号    #注释(预处理)   // 注释            多行注释
  8                     9               10                 11
 #!内容                 //!内容         /*!内容            标识符
  12                    13              14
  '',字符               "" 字符串       关键字1
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
			if (keywords[i] == str) {				// 判断str是不是keylist里指定的关键词
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


extern CQcmd mainParse(std::string cmd) {
	strType result_cmd;
	strType highType;
	CQcmd code;
	int i = 0, len;
	len = cmd.length();

	while (1) {
		result_cmd = strTypeMatch(cmd);
		if (result_cmd.state == 0) {
			break;
		}
		i += result_cmd.subStr.length();
		cmd = cmd.substr(result_cmd.subStr.length(), len - i);
		highType = strHighlightType(result_cmd.state, result_cmd.subStr);

		if (highType.state == 14) {                     // 14是预设关键词    开始是想着按照`关键词 + [关键词] + 操作数 + 操作内容`的模式来, 但是...也许以后得重新设计一下...
			code.status += 2;
			switch (highType.pos) {
			case 0: code.cmdID = 0; break;              // send
			case 1: code.cmdID = 1; break;              // set    
			case 2: code.cmdID = 2; break;              // get      
			case 3:                                     // msg, msg有 `send`, `set`, `get` 三个动作
				if (code.cmdID == 0) {
					code.toGrp = -1;
					code.toPri = 233;
				}
				if (code.cmdID == 1) {
					code.action = 849424;               // msg被`设置`就只有`撤回`一种情况, 所以关键词`delete`和限定条件可有可无. (编号偷懒, `撤回`的九宫格输入数...)
					code.subCmdID = 0;
				}
				break;
			case 4:                                     // grp, grp有 `set`的限定: `ban`, `anonymous`, `request`, `admin`, `kick` ; `get`的限定: `info`, `list`
				if (code.cmdID == 0) {
					code.toGrp = 233;
					code.toPri = -1;
				}
				break;
			case 5:
				break;
				// 暂时先不做那么多了, 以后再慢慢填坑吧...

			case 25:
				code.flag = 100;
				if (code.cmdID != -1) {
					code.toGrp = -1;
					code.toPri = 233;
				}
				break;
			case 26: code.cmdID = 4; code.status += 4; break;
			default:
				break;
			}
		}

		if (highType.state == 2) {                  // 2是数字
			code.status += 3;
			if (code.cmdID == 0) {
				if (code.toPri == 233)
					code.toPri = std::stoll(result_cmd.subStr);
				if (code.toGrp == 233)
					code.toGrp = std::stoll(result_cmd.subStr);
			}
			else if (code.cmdID == 1) {
				if (code.action == 849424)
					code.action = std::stol(result_cmd.subStr);
			}
			else if (code.cmdID == 4) {
				code.action = std::stoll(result_cmd.subStr);
			}
		}

		if (highType.state == 13) {                 // 13是问号字符串
			code.status += 1;
			std::string cont = result_cmd.subStr.substr(1, result_cmd.subStr.length() - 2);
			char buff[4096];
			sprintf_s(buff, cont.c_str());
			code.content = buff;
		}
	}
	return code;
}