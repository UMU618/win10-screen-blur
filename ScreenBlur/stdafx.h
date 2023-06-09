#pragma once

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#define _WIN32_IE 0x0700
#define _RICHEDIT_VER 0x0500

// ATL
#include <atlbase.h>
#include <atlstr.h>
#include <atltypes.h>
#include <atlwin.h>

// WTL
#include <atlapp.h>

extern CAppModule _Module;