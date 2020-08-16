#pragma once

typedef enum _WINDOWCOMPOSITIONATTRIB {
  WCA_UNDEFINED = 0,
  WCA_NCRENDERING_ENABLED = 1,
  WCA_NCRENDERING_POLICY = 2,
  WCA_TRANSITIONS_FORCEDISABLED = 3,
  WCA_ALLOW_NCPAINT = 4,
  WCA_CAPTION_BUTTON_BOUNDS = 5,
  WCA_NONCLIENT_RTL_LAYOUT = 6,
  WCA_FORCE_ICONIC_REPRESENTATION = 7,
  WCA_EXTENDED_FRAME_BOUNDS = 8,
  WCA_HAS_ICONIC_BITMAP = 9,
  WCA_THEME_ATTRIBUTES = 10,
  WCA_NCRENDERING_EXILED = 11,
  WCA_NCADORNMENTINFO = 12,
  WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
  WCA_VIDEO_OVERLAY_ACTIVE = 14,
  WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
  WCA_DISALLOW_PEEK = 16,
  WCA_CLOAK = 17,
  WCA_CLOAKED = 18,
  WCA_ACCENT_POLICY = 19,
  WCA_FREEZE_REPRESENTATION = 20,
  WCA_EVER_UNCLOAKED = 21,
  WCA_VISUAL_OWNER = 22,
  WCA_LAST = 23
} WINDOWCOMPOSITIONATTRIB;

typedef struct _WINDOWCOMPOSITIONATTRIBDATA {
  WINDOWCOMPOSITIONATTRIB Attrib;
  PVOID pvData;
  SIZE_T cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

typedef enum _ACCENT_STATE {
  ACCENT_DISABLED = 0,
  ACCENT_ENABLE_GRADIENT = 1,
  ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
  ACCENT_ENABLE_BLURBEHIND = 3,
  ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
  ACCENT_INVALID_STATE = 5
} ACCENT_STATE;

typedef struct _ACCENT_POLICY {
  ACCENT_STATE AccentState;
  DWORD AccentFlags;
  DWORD GradientColor;
  DWORD AnimationId;
} ACCENT_POLICY;

typedef BOOL(WINAPI* pfnSetWindowCompositionAttribute)(
    HWND,
    WINDOWCOMPOSITIONATTRIBDATA*);


class BlurApi {
 public:
  static BlurApi& GetInstance() {
    static BlurApi instance;
    return instance;
  }

 public:
  bool InitBlur() {
    HMODULE user32 = GetModuleHandle(L"user32.dll");
    if (nullptr != user32) {
      SetWindowCompositionAttribute_ =
          (pfnSetWindowCompositionAttribute)GetProcAddress(
              user32, "SetWindowCompositionAttribute");
      if (nullptr != SetWindowCompositionAttribute_) {
        return true;
      }
    }
    return false;
  }

  bool EnableBlur(HWND hWnd,
                  ACCENT_STATE state,
                  DWORD gradient_color = 0) {
    ATLASSERT(nullptr != SetWindowCompositionAttribute_);
    ACCENT_POLICY accent = {state, 0, gradient_color, 0};
    WINDOWCOMPOSITIONATTRIBDATA data = {};
    data.Attrib = WCA_ACCENT_POLICY;
    data.pvData = &accent;
    data.cbData = sizeof(accent);
    return SetWindowCompositionAttribute_(hWnd, &data) != FALSE;
  }

 private:
  pfnSetWindowCompositionAttribute SetWindowCompositionAttribute_ = nullptr;
};
