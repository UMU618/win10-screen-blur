// Implementation of the CNotifyIconData class and the CTrayIconImpl template.
#pragma once

namespace umu {
// Wrapper class for the Win32 NOTIFYICONDATA structure
class NotifyIconData : public NOTIFYICONDATA {
 public:
  NotifyIconData() {
    ZeroMemory(this, sizeof(NOTIFYICONDATA));
    cbSize = sizeof(NOTIFYICONDATA);
  }
};

// Template used to support adding an icon to the taskbar.
// This class will maintain a taskbar icon and associated context menu.
template <class T>
class ShellNotifyIconImpl {
 public:
  ShellNotifyIconImpl() : added_(false), menu_default_item_(0) {
    WM_TASKBAR_CREATED_ = RegisterWindowMessage(_T("TaskbarCreated"));
  }

  ~ShellNotifyIconImpl() {
    // Remove the icon
    DeleteNotifyIcon();
  }

  // Add a notify icon
  //  lpszToolTip - The tooltip to display
  //  hIcon       - The icon to display
  //  nID         - The resource ID of the context menu
  // returns true on success
  bool AddNotifyIcon(LPCTSTR lpszToolTip, HICON hIcon, UINT nID) {
    T* pT = static_cast<T*>(this);
    // Fill in the data
    notify_icon_data_.hWnd = *pT;
    notify_icon_data_.uID = nID;
    notify_icon_data_.hIcon = hIcon;
    notify_icon_data_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    notify_icon_data_.uCallbackMessage = WM_SHELL_NOTIFY_ICON;
    _tcscpy_s(notify_icon_data_.szTip, _countof(notify_icon_data_.szTip),
              lpszToolTip);
    added_ = Shell_NotifyIcon(NIM_ADD, &notify_icon_data_) ? true : false;
    // Done
    return added_;
  }

  bool ModifyNotifyIcon(HICON hIcon) {
    // Fill in the data
    notify_icon_data_.hIcon = hIcon;
    notify_icon_data_.uFlags = NIF_ICON;
    added_ = Shell_NotifyIcon(NIM_MODIFY, &notify_icon_data_) ? true : false;
    // Done
    return added_;
  }

  // Delete taskbar icon
  // returns true on success
  bool DeleteNotifyIcon() {
    if (!added_)
      return false;
    // Remove
    notify_icon_data_.uFlags = 0;
    if (Shell_NotifyIcon(NIM_DELETE, &notify_icon_data_)) {
      added_ = false;
      return true;
    }
    return false;
  }

  // Set the icon tooltip text
  // returns true on success
  bool SetTooltipText(LPCTSTR pszTooltipText) {
    if (pszTooltipText == NULL)
      return FALSE;
    // Fill the structure
    notify_icon_data_.uFlags = NIF_TIP;
    _tcscpy_s(notify_icon_data_.szTip, _countof(notify_icon_data_.szTip),
              pszTooltipText);
    // Set
    return Shell_NotifyIcon(NIM_MODIFY, &notify_icon_data_) ? true : false;
  }

  bool SetBalloonTooltipText(LPCTSTR info_text,
                             LPCTSTR info_title = NULL,
                             UINT timeout = 1000) {
    if (NULL == info_text) {
      return false;
    }
    // Fill the structure
    notify_icon_data_.uFlags = NIF_INFO;
    notify_icon_data_.dwInfoFlags = NIIF_INFO;
    notify_icon_data_.uTimeout = timeout;
    _tcscpy_s(notify_icon_data_.szInfo, _countof(notify_icon_data_.szInfo),
              info_text);
    if (NULL != info_title) {
      _tcscpy_s(notify_icon_data_.szInfoTitle,
                _countof(notify_icon_data_.szInfoTitle), info_title);
    }
    // Set
    return Shell_NotifyIcon(NIM_MODIFY, &notify_icon_data_) ? true : false;
  }

  // Set the default menu item ID
  inline void SetDefaultItem(UINT nID) { menu_default_item_ = nID; }

  BEGIN_MSG_MAP(ShellNotifyIconImpl)
    MESSAGE_HANDLER(WM_SHELL_NOTIFY_ICON, OnShellNotifyIcon)
    MESSAGE_HANDLER(WM_TASKBAR_CREATED_, OnTaskbarCreated)
  END_MSG_MAP()

  LRESULT OnShellNotifyIcon(UINT /*uMsg*/,
                            WPARAM wParam,
                            LPARAM lParam,
                            BOOL& /*bHandled*/) {
    // Is this the ID we want?
    if (wParam != notify_icon_data_.uID)
      return 0;
    T* pT = static_cast<T*>(this);
    // Was the right-button clicked?
    if (LOWORD(lParam) == WM_RBUTTONUP) {
      // Load the menu
      CMenu menu;
      if (!menu.LoadMenu(notify_icon_data_.uID))
        return 0;
      // Get the sub-menu
      CMenuHandle popup_menu(menu.GetSubMenu(0));
      // Prepare
      pT->PrepareMenu(popup_menu);
      // Get the menu position
      CPoint pos;
      GetCursorPos(&pos);
      // Make app the foreground
      ::SetForegroundWindow(*pT);
      // Set the default menu item
      if (menu_default_item_ == 0) {
        popup_menu.SetMenuDefaultItem(0, TRUE);
      } else {
        popup_menu.SetMenuDefaultItem(menu_default_item_);
      }
      // Track
      int cmd = popup_menu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RETURNCMD,
                                            pos.x, pos.y, *pT);
      pT->PostMessage(WM_COMMAND, cmd, reinterpret_cast<LPARAM>(pT->m_hWnd));
      // BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
      // pT->PostMessage(WM_NULL);
      // Done
      menu.DestroyMenu();
    } else if (LOWORD(lParam) == WM_LBUTTONDBLCLK) {
      // Make app the foreground
      SetForegroundWindow(*pT);
      // Load the menu
      CMenu menu;
      if (!menu.LoadMenu(notify_icon_data_.uID))
        return 0;
      // Get the sub-menu
      CMenuHandle popup_menu(menu.GetSubMenu(0));
      // Get the item
      if (menu_default_item_) {
        // Send
        pT->SendMessage(WM_COMMAND, menu_default_item_, 0);
      } else {
        UINT item = popup_menu.GetMenuItemID(0);
        // Send
        pT->SendMessage(WM_COMMAND, item, 0);
      }
      // Done
      menu.DestroyMenu();
    }

    return 0;
  }

  LRESULT OnTaskbarCreated(UINT /*uMsg*/,
                           WPARAM /*wParam*/,
                           LPARAM /*lParam*/,
                           BOOL& /*bHandled*/) {
    DeleteNotifyIcon();
    notify_icon_data_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    added_ = Shell_NotifyIcon(NIM_ADD, &notify_icon_data_) ? true : false;
    return 0;
  }

  // Allow the menu items to be enabled/checked/etc.
  virtual void PrepareMenu(HMENU /*hMenu*/) {
    // Stub
  }

 private:
  static const UINT WM_SHELL_NOTIFY_ICON = (WM_USER + 100);

  NotifyIconData notify_icon_data_;
  bool added_;
  UINT menu_default_item_;
  UINT WM_TASKBAR_CREATED_;
};

}  // namespace umu
