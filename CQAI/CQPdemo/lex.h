#pragma once

struct CQcmd {
	std::int8_t		cmdID = -1;
	std::int8_t		subCmdID = -1;
	std::int8_t		flag = -1;
	std::int64_t	toPri = -1;
	std::int64_t	toGrp = -1;
	std::int32_t    action = -1;
	const char* content = "Err";
};
extern CQcmd mainParse(std::string cmd);