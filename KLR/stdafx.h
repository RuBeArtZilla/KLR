// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

/*#define WINVER         0x0600
#define _WIN32_WINNT   0x0600
#define _WIN32_IE      0x0700*/

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <CommCtrl.h>
#include <iterator>
#include <vector>
#include <string>
#include <sstream>