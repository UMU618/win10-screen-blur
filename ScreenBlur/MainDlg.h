/*
 *  MIT License
 *
 * Copyright 2020-present UMU618.com, UMUTech.com
 *
 */

#pragma once

extern CString g_app_id;

class CMainDlg : public CDialogImpl<CMainDlg>,
                 public CUpdateUI<CMainDlg>,
                 public CMessageFilter,
                 public CIdleHandler,
                 public umu::ShellNotifyIconImpl<CMainDlg> {
 public:
  enum { IDD = IDD_MAINDLG, WM_ACTIVATE_PREV_WINDOW = WM_USER + 101 };

  virtual BOOL PreTranslateMessage(MSG* pMsg) {
    return CWindow::IsDialogMessage(pMsg);
  }

  virtual BOOL OnIdle() {
    UIUpdateChildWindows();
    return FALSE;
  }

  BEGIN_UPDATE_UI_MAP(CMainDlg)
  END_UPDATE_UI_MAP()

  BEGIN_MSG_MAP(CMainDlg)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_KEYUP, Hide)
    MESSAGE_HANDLER(WM_LBUTTONUP, Hide)
    MESSAGE_RANGE_HANDLER(WM_MBUTTONDOWN, WM_MOUSELAST, Hide)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
    MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_ENDSESSION, OnEndSession)
    MESSAGE_HANDLER(WM_ACTIVATE_PREV_WINDOW, Show)

    COMMAND_ID_HANDLER(ID_POPUP_ACRYLIC_BLUR, OnPopupAcrylicBlur)
    COMMAND_ID_HANDLER(ID_POPUP_BLUR, OnPopupBlur)
    COMMAND_ID_HANDLER(ID_POPUP_PAUSE, OnPopupPause)
    COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
    COMMAND_ID_HANDLER(ID_APP_EXIT, OnAppExit)

    CHAIN_MSG_MAP(ShellNotifyIconImpl)
  END_MSG_MAP()

  LRESULT OnEraseBkgnd(UINT /*uMsg*/,
                       WPARAM /*wParam*/,
                       LPARAM /*lParam*/,
                       BOOL& /*bHandled*/) {
    return 1;
  }

  LRESULT OnTimer(UINT /*uMsg*/,
                  WPARAM wParam,
                  LPARAM /*lParam*/,
                  BOOL& /*bHandled*/) {
    if (IDR_MAINFRAME == wParam) {
      LASTINPUTINFO lii = {sizeof(LASTINPUTINFO)};
      if (GetLastInputInfo(&lii) &&
          GetTickCount() - lii.dwTime > wait_seconds_ * 1000) {
        SetWindowPos(nullptr, GetSystemMetrics(SM_XVIRTUALSCREEN),
                     GetSystemMetrics(SM_YVIRTUALSCREEN),
                     GetSystemMetrics(SM_CXVIRTUALSCREEN),
                     GetSystemMetrics(SM_CYVIRTUALSCREEN),
                     SWP_NOZORDER | SWP_SHOWWINDOW);
      }
    }
    return 0;
  }

  LRESULT Hide(UINT /*uMsg*/,
               WPARAM /*wParam*/,
               LPARAM /*lParam*/,
               BOOL& /*bHandled*/) {
    x_ = -1;
    y_ = -1;
    ShowWindow(SW_HIDE);
    return 0;
  }

  LRESULT Show(UINT /*uMsg*/,
               WPARAM /*wParam*/,
               LPARAM /*lParam*/,
               BOOL& /*bHandled*/) {
    SetWindowPos(nullptr, GetSystemMetrics(SM_XVIRTUALSCREEN),
                 GetSystemMetrics(SM_YVIRTUALSCREEN),
                 GetSystemMetrics(SM_CXVIRTUALSCREEN),
                 GetSystemMetrics(SM_CYVIRTUALSCREEN),
                 SWP_NOZORDER | SWP_SHOWWINDOW);
    return 0;
  }

  LRESULT OnMouseMove(UINT /*uMsg*/,
                      WPARAM /*wParam*/,
                      LPARAM lParam,
                      BOOL& /*bHandled*/) {
    if (-1 == x_ || -1 == y_) {
      x_ = GET_X_LPARAM(lParam);
      y_ = GET_Y_LPARAM(lParam);
    } else {
      UINT dx = std::abs(x_ - GET_X_LPARAM(lParam));
      UINT dy = std::abs(y_ - GET_Y_LPARAM(lParam));
      if (dx > 18 || dy > 16) {
        x_ = -1;
        y_ = -1;
        ShowWindow(SW_HIDE);
      }
    }
    return 0;
  }

  LRESULT OnContextMenu(UINT /*uMsg*/,
                        WPARAM /*wParam*/,
                        LPARAM lParam,
                        BOOL& /*bHandled*/) {
    CMenu menu;
    menu.LoadMenuW(IDR_MAINFRAME);
    CMenuHandle popup_menu = menu.GetSubMenu(0);
    PrepareMenu(popup_menu);
    popup_menu.SetMenuDefaultItem(ID_APP_ABOUT);
    popup_menu.TrackPopupMenuEx(TPM_RIGHTALIGN | TPM_BOTTOMALIGN,
                                GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
                                m_hWnd);
    return 0;
  }

  void PrepareMenu(HMENU hMenu) {
    CMenuHandle popup_menu(hMenu);
    popup_menu.CheckMenuRadioItem(ID_POPUP_ACRYLIC_BLUR, ID_POPUP_BLUR, accent_,
                                  MF_BYCOMMAND);
    popup_menu.CheckMenuItem(
        ID_POPUP_PAUSE, MF_BYCOMMAND | (paused_ ? MF_CHECKED : MF_UNCHECKED));
  }

  LRESULT OnSettingChange(UINT /*uMsg*/,
                          WPARAM /*wParam*/,
                          LPARAM /*lParam*/,
                          BOOL& /*bHandled*/) {
    MoveWindow(GetSystemMetrics(SM_XVIRTUALSCREEN),
               GetSystemMetrics(SM_YVIRTUALSCREEN),
               GetSystemMetrics(SM_CXVIRTUALSCREEN),
               GetSystemMetrics(SM_CYVIRTUALSCREEN));
    return 0;
  }

  LRESULT OnInitDialog(UINT /*uMsg*/,
                       WPARAM /*wParam*/,
                       LPARAM /*lParam*/,
                       BOOL& /*bHandled*/) {
    umu::SingletonWindow::SetSingleton(m_hWnd, g_app_id);

    // set icons
    HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR,
                                   ::GetSystemMetrics(SM_CXICON),
                                   ::GetSystemMetrics(SM_CYICON));
    SetIcon(hIcon, TRUE);
    HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR,
                                        ::GetSystemMetrics(SM_CXSMICON),
                                        ::GetSystemMetrics(SM_CYSMICON));
    SetIcon(hIconSmall, FALSE);

    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    UIAddChildWindowContainer(m_hWnd);

    CString tip;
    tip.LoadString(IDR_MAINFRAME);
    SetDefaultItem(ID_APP_ABOUT);
    AddNotifyIcon(tip, hIconSmall, IDR_MAINFRAME);

    MoveWindow(GetSystemMetrics(SM_XVIRTUALSCREEN),
               GetSystemMetrics(SM_YVIRTUALSCREEN),
               GetSystemMetrics(SM_CXVIRTUALSCREEN),
               GetSystemMetrics(SM_CYVIRTUALSCREEN));

    if (ID_POPUP_ACRYLIC_BLUR == accent_) {
      BlurApi::GetInstance().EnableBlur(m_hWnd, ACCENT_ENABLE_ACRYLICBLURBEHIND,
                                        0x00FFFFFF);
    } else if (ID_POPUP_BLUR == accent_) {
      BlurApi::GetInstance().EnableBlur(m_hWnd, ACCENT_ENABLE_BLURBEHIND);
    }

    SetTimer(IDR_MAINFRAME, 5000);

    return TRUE;
  }

  LRESULT OnDestroy(UINT /*uMsg*/,
                    WPARAM /*wParam*/,
                    LPARAM /*lParam*/,
                    BOOL& /*bHandled*/) {
    // unregister message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveMessageFilter(this);
    pLoop->RemoveIdleHandler(this);

    return 0;
  }

  LRESULT OnEndSession(UINT /*uMsg*/,
                       WPARAM /*wParam*/,
                       LPARAM lParam,
                       BOOL& /*bHandled*/) {
    CloseDialog(IDABORT);
    return 0;
  }

  LRESULT OnPopupAcrylicBlur(WORD /*wNotifyCode*/,
                             WORD wID,
                             HWND /*hWndCtl*/,
                             BOOL& /*bHandled*/) {
    if (ID_POPUP_ACRYLIC_BLUR != accent_) {
      accent_ = ID_POPUP_ACRYLIC_BLUR;
      BlurApi::GetInstance().EnableBlur(m_hWnd,
                                        ACCENT_ENABLE_TRANSPARENTGRADIENT);
      BlurApi::GetInstance().EnableBlur(m_hWnd, ACCENT_ENABLE_ACRYLICBLURBEHIND,
                                        0x00FFFFFF);
    }
    return 0;
  }

  LRESULT OnPopupBlur(WORD /*wNotifyCode*/,
                      WORD wID,
                      HWND /*hWndCtl*/,
                      BOOL& /*bHandled*/) {
    if (ID_POPUP_BLUR != accent_) {
      accent_ = ID_POPUP_BLUR;
      BlurApi::GetInstance().EnableBlur(m_hWnd, ACCENT_ENABLE_BLURBEHIND);
    }
    return 0;
  }

  LRESULT OnPopupPause(WORD /*wNotifyCode*/,
                       WORD /*wID*/,
                       HWND /*hWndCtl*/,
                       BOOL& /*bHandled*/) {
    paused_ = !paused_;
    if (paused_) {
      KillTimer(IDR_MAINFRAME);
    } else {
      SetTimer(IDR_MAINFRAME, 5000);
    }
    return 0;
  }

  LRESULT OnAppAbout(WORD /*wNotifyCode*/,
                     WORD /*wID*/,
                     HWND /*hWndCtl*/,
                     BOOL& /*bHandled*/) {
    CAboutDlg dlg;
    dlg.DoModal(m_hWnd);
    return 0;
  }

  LRESULT OnAppExit(WORD /*wNotifyCode*/,
                    WORD wID,
                    HWND /*hWndCtl*/,
                    BOOL& /*bHandled*/) {
    CloseDialog(wID);
    return 0;
  }

  void CloseDialog(int nVal) {
    DestroyWindow();
    ::PostQuitMessage(nVal);
  }

 private:
  bool paused_ = false;
  DWORD wait_seconds_ = 30;
  UINT accent_ = ID_POPUP_ACRYLIC_BLUR;
  int x_ = -1;
  int y_ = -1;
};
