/*
 *  MIT License
 *
 * Copyright 2020-present UMU618.com, UMUTech.com
 *
 */

#include "stdafx.h"

#include <atlctrls.h>
#include <atldlgs.h>
#include <atlframe.h>

#include "umu/shell_notify_icon.hpp"
#include "umu/singleton_window.hpp"

#include "resource.h"

#include "aboutdlg.h"
#include "blur.hpp"

#include "MainDlg.h"

#if defined _M_IX86
#pragma comment( \
    linker,      \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment( \
    linker,      \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment( \
    linker,      \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment( \
    linker,      \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

CAppModule _Module;
CString g_app_id(_T("{29F65821-ABAE-450D-82A0-DBAB27DAD259}"));

int Run(LPTSTR /*lpstrCmdLine*/ = nullptr, int nCmdShow = SW_SHOWDEFAULT) {
  CMessageLoop theLoop;
  _Module.AddMessageLoop(&theLoop);

  CMainDlg dlgMain;

  if (dlgMain.Create(nullptr) == nullptr) {
    ATLTRACE(_T("Main dialog creation failed!\n"));
    return 0;
  }

  dlgMain.ShowWindow(nCmdShow);

  int nRet = theLoop.Run();

  _Module.RemoveMessageLoop();
  return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance,
                     HINSTANCE /*hPrevInstance*/,
                     LPTSTR lpstrCmdLine,
                     int nCmdShow) {
  HWND prev = umu::SingletonWindow::FindPrevInstanceWindow(g_app_id);
  if (nullptr != prev) {
    PostMessage(prev, CMainDlg::WM_ACTIVATE_PREV_WINDOW, 0, 0);
    return 0;
  }

  if (!BlurApi::GetInstance().InitBlur()) {
    return -1;
  }

  HRESULT hRes = ::CoInitialize(nullptr);
  ATLASSERT(SUCCEEDED(hRes));

  AtlInitCommonControls(ICC_BAR_CLASSES | ICC_HOTKEY_CLASS);

  hRes = _Module.Init(nullptr, hInstance);
  ATLASSERT(SUCCEEDED(hRes));

  int nRet = Run(lpstrCmdLine, nCmdShow);

  _Module.Term();
  ::CoUninitialize();

  return nRet;
}
