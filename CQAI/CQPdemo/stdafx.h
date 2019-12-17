// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



// TODO: reference additional headers your program requires here
#include "stdint.h"
#include "string"

#include <iostream>
#include <ctime>
#include "sqlite3.h"
#include <regex>
#include <thread>
#include "timer.hpp"
#include "lex.h"
#include "dbStruct.h"