#pragma once


namespace umu {
class SingletonWindow {
 public:
  static HWND FindPrevInstanceWindow(LPCWSTR trait) {
    WindowTrait window_trait = {trait, nullptr};
    ::EnumWindows(EnumWindowsProc, (LPARAM)&window_trait);
    return window_trait.hwnd;
  }

  static bool SetSingleton(HWND hwnd, LPCWSTR trait) {
    return !!::SetProp(hwnd, trait, hwnd);
  }

  static bool CheckWindowProp(HWND hwnd, LPCWSTR trait) {
    HANDLE handle = ::GetProp(hwnd, trait);
    if (nullptr != handle && handle == hwnd) {
      return true;
    }
    return false;
  }

 private:
  static BOOL CALLBACK EnumWindowsProc(__in HWND hwnd, __in LPARAM lParam) {
    if (nullptr != hwnd) {
      WindowTrait* trait = (WindowTrait*)lParam;
      if (CheckWindowProp(hwnd, trait->trait)) {
        trait->hwnd = hwnd;
        return FALSE;
      }
    }
    return TRUE;
  }

 private:
  struct WindowTrait {
    LPCWSTR trait;
    HWND hwnd;
  };
};
}  // namespace umu
