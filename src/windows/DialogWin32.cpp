#include "DialogWin32.h"
#include "base/log.h"
#include "scene2d/scene2d.h"
#include "base/EncodingManager.h"
#include "graphics/GraphicDeviceD2D.h"
#include "graphics/PainterD2D.h"
#include "theme.h"
#include "absl/strings/str_format.h"
#include "absl/strings/numbers.h"
#include "absl/functional/bind_front.h"
#include "absl/time/clock.h"
#include "graph2d/PaintContextInterface.h"
#include "api/kwui/ScriptEngine.h"
#include "api/kwui/Application.h"
#include "base/ResourceManager.h"
#include "graph2d/graph2d.h"
#ifdef _WIN32
#include "graphics/PaintSurfaceD2D.h"
#endif
#if WITH_SKIA
#include "xskia/PaintSurfaceX.h"
#endif

using base::EncodingManager;
using base::ResourceManager;

namespace windows {
typedef LRESULT(CALLBACK* WndProc)(HWND, UINT, WPARAM, LPARAM);
static ATOM RegisterWindowClass(HINSTANCE hInstance,
    const wchar_t* class_name,
    HICON icon,
    WndProc wnd_proc);

static constexpr UINT_PTR ANIMATION_TIMER_EVENT = 0xFFFF00A0;
static HCURSOR s_preloaded_cursors[NUM_CURSOR_TYPES] = {};

typedef HRESULT(WINAPI* GETDPIFORMONITOR)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);

typedef BOOL(WINAPI* ADJUSTWINDOWRECTEXFORDPI)(LPRECT, DWORD, BOOL, DWORD, UINT);

float DialogWin32::getMonitorDpiScale(HMONITOR monitor)
{
    static GETDPIFORMONITOR pGETDPIFORMONITOR = NULL;
    if (!pGETDPIFORMONITOR) {
        HMODULE hm = LoadLibraryW(L"Shcore.dll");
        if (hm) {
            pGETDPIFORMONITOR = (GETDPIFORMONITOR)GetProcAddress(hm, "GetDpiForMonitor");
        }
        if (!pGETDPIFORMONITOR) {
            pGETDPIFORMONITOR = [](HMONITOR, MONITOR_DPI_TYPE, UINT* dpi_x, UINT* dpi_y) -> HRESULT {
                float dpi_scale = graph2d::getInitialDesktopDpiScale();
                *dpi_x = *dpi_y = USER_DEFAULT_SCREEN_DPI * dpi_scale;
                return S_OK;
                };
        }
    }
    UINT dpi = 96;
    DEVICE_SCALE_FACTOR scale = SCALE_100_PERCENT;
    (*pGETDPIFORMONITOR)(monitor, MDT_EFFECTIVE_DPI, &dpi, &dpi);
    return (float)dpi / 96.0f;
}

static void PreloadCursor() {
    if (s_preloaded_cursors[0])
        return;
    s_preloaded_cursors[CURSOR_ARROW] = LoadCursor(NULL, IDC_ARROW);
    s_preloaded_cursors[CURSOR_CROSS] = LoadCursor(NULL, IDC_CROSS);
    s_preloaded_cursors[CURSOR_HAND] = LoadCursor(NULL, IDC_HAND);
    s_preloaded_cursors[CURSOR_IBEAM] = LoadCursor(NULL, IDC_IBEAM);
    s_preloaded_cursors[CURSOR_WAIT] = LoadCursor(NULL, IDC_WAIT);
}

static std::unordered_map<std::string, DialogWin32*> g_dialog_map;

static scene2d::VKey make_vkey(int vk) {
    static const struct {
        int      fVK;
        scene2d::VKey fKey;
    } gPair[] = {
        { VK_RETURN,  scene2d::VKey::Return   },
        { VK_ESCAPE,  scene2d::VKey::Escape   },
        { VK_BACK,    scene2d::VKey::Back     },
        { VK_DELETE,  scene2d::VKey::Delete   },
        { VK_UP,      scene2d::VKey::Up       },
        { VK_DOWN,    scene2d::VKey::Down     },
        { VK_LEFT,    scene2d::VKey::Left     },
        { VK_RIGHT,   scene2d::VKey::Right    },
        { VK_TAB,     scene2d::VKey::Tab      },
        { VK_PRIOR,   scene2d::VKey::PageUp   },
        { VK_NEXT,    scene2d::VKey::PageDown },
        { VK_HOME,    scene2d::VKey::Home     },
        { VK_END,     scene2d::VKey::End      },
        { VK_SPACE,   scene2d::VKey::Space    },
        // { VK_SHIFT,   scene2d::VKey::Shift    },
        // { VK_CONTROL, scene2d::VKey::Ctrl     },
        // { VK_MENU,    scene2d::VKey::Option   },
        { 'A',        scene2d::VKey::A        },
        { 'B',        scene2d::VKey::B        },
        { 'C',        scene2d::VKey::C        },
        { 'V',        scene2d::VKey::V        },
        { 'X',        scene2d::VKey::X        },
        { 'Y',        scene2d::VKey::Y        },
        { 'Z',        scene2d::VKey::Z        },
    };
    for (size_t i = 0; i < ABSL_ARRAYSIZE(gPair); i++) {
        if (gPair[i].fVK == vk) {
            return gPair[i].fKey;
        }
    }
    return scene2d::VKey::Invalid;
}

DialogWin32::DialogWin32(const WCHAR* wnd_class_name,
    HICON icon, int flags,
    absl::optional<PopupShadowData> popup_shadow,
    absl::optional<CreateData> create_data)
    : hwnd_parent_(NULL), hwnd_anchor_(NULL), hwnd_(NULL)
    , flags_(flags), create_data_(create_data)
    , visible_(false), first_show_window_(true)
    , mouse_event_tracking_(false), mouse_capture_(false)
    , popup_shadow_data_(popup_shadow)
    , on_window_pos_changed_(false)
    , dpi_scale_(1.0f)
    , himc_(NULL)
    , animation_timer_id_(0) {

    if (create_data.has_value()) {
        dpi_scale_ = create_data.value().dpi_scale;
    }

    mouse_position_ = scene2d::PointF(size_.width * 0.5f, size_.height * 0.5f);
    id_ = absl::StrFormat("%p", this);
    g_dialog_map[id_] = this;
    PreloadCursor();
    InitWindow(GetModuleHandle(NULL), wnd_class_name, icon);
}

DialogWin32::~DialogWin32() {
    g_dialog_map.erase(id_);
}

DialogWin32* DialogWin32::findDialogById(const std::string& id)
{
    auto it = g_dialog_map.find(id);
    return (it == g_dialog_map.end()) ? nullptr : it->second;
}

void DialogWin32::SetPopupAnchor(DialogWin32* anchor)
{
    hwnd_anchor_ = anchor ? anchor->hwnd_ : NULL;
}

void DialogWin32::SetVisible(bool visible) {
    if (visible_ != visible) {
        visible_ = visible;
    }
    if (popup_shadow_) {
        popup_shadow_->SetVisible(visible);
    }
    if (visible_) {
        if (flags_ & DIALOG_FLAG_POPUP) {
            ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
        } else {
            ShowWindow(hwnd_, SW_SHOWNORMAL);
        }
    } else {
        ShowWindow(hwnd_, SW_HIDE);
    }
    if (hwnd_parent_) {
        EnableWindow(hwnd_parent_, !visible_);
    }
    if (visible_ && first_show_window_) {
        first_show_window_ = false;
        UpdateWindow(hwnd_);
    }
}

scene2d::RectF DialogWin32::adjustWindowRect(const scene2d::RectF& rect,
    float dpi_scale, bool customFrame, bool popup)
{
    static ADJUSTWINDOWRECTEXFORDPI pADJUSTWINDOWRECTEXFORDPI = NULL;
    if (!pADJUSTWINDOWRECTEXFORDPI) {
        HMODULE hm = LoadLibraryW(L"Shcore.dll");
        if (hm) {
            pADJUSTWINDOWRECTEXFORDPI = (ADJUSTWINDOWRECTEXFORDPI)GetProcAddress(hm, "AdjustWindowRectExForDpi");
        }
        if (!pADJUSTWINDOWRECTEXFORDPI) {
            pADJUSTWINDOWRECTEXFORDPI = [](LPRECT rect, DWORD style, BOOL menu, DWORD ex_style, UINT) -> BOOL {
                return AdjustWindowRectEx(rect, style, menu, ex_style);
                };
        }
    }
    UINT dpi = (UINT)((float)USER_DEFAULT_SCREEN_DPI * dpi_scale);
    RECT r = { (LONG)(rect.left), (LONG)rect.top, lroundf(rect.right), lroundf(rect.bottom) };
    DWORD style, ex_style;
    if (customFrame) {
        style = WS_POPUP;
        ex_style = 0;
    } else {
        if (popup) {
            style = WS_POPUPWINDOW;
            ex_style = WS_EX_PALETTEWINDOW;
        } else {
            style = WS_OVERLAPPEDWINDOW;
            ex_style = WS_EX_OVERLAPPEDWINDOW;
        }
    }
    pADJUSTWINDOWRECTEXFORDPI(&r, style, FALSE, ex_style, dpi);
    return scene2d::RectF::fromLTRB((float)r.left,
        (float)r.top, (float)r.right, (float)r.bottom);
}

void DialogWin32::Resize(float width, float height) {
    long w = lroundf(width * dpi_scale_);
    long h = lroundf(height * dpi_scale_);
    SetWindowPos(hwnd_, NULL,
        0, 0,
        (int)w, (int)h,
        SWP_NOMOVE | SWP_NOACTIVATE);
}

void DialogWin32::setWindowPos(const scene2d::RectF& rect)
{
    SetWindowPos(hwnd_, NULL,
        (int)rect.left,
        (int)rect.top,
        (int)rect.width(),
        (int)rect.height(),
        SWP_NOACTIVATE);
}

void DialogWin32::SetTitle(const std::string& text) {
    // _title_label->SetText(text);
    auto utf16_text = EncodingManager::UTF8ToWide(text);
    SetWindowTextW(hwnd_, utf16_text.c_str());
}

void DialogWin32::InitWindow(HINSTANCE hInstance, const WCHAR* wnd_class_name, HICON icon) {
    auto title = EncodingManager::UTF8ToWide("");
    RegisterWindowClass(hInstance, wnd_class_name, icon, &DialogWin32::WndProcMain);
    DWORD style, ex_style;
    if (flags_ & DIALOG_FLAG_MAIN) {
        if (popup_shadow_data_.has_value()) {
            style = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
            ex_style = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR
                | WS_EX_STATICEDGE | WS_EX_APPWINDOW;

        } else {
            style = WS_OVERLAPPEDWINDOW;
            ex_style = WS_EX_OVERLAPPEDWINDOW;
        }
    } else if (flags_ & DIALOG_FLAG_POPUP) {
        //style = WS_POPUP | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        //ex_style = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR
        //    | WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_WINDOWEDGE | WS_EX_STATICEDGE;
        style = WS_POPUPWINDOW;
        ex_style = WS_EX_NOACTIVATE | WS_EX_PALETTEWINDOW;
    } else {
        style = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        ex_style = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR
            | WS_EX_STATICEDGE | WS_EX_APPWINDOW;
    }
    int wnd_left = 360;
    int wnd_top = 240;
    if (create_data_.has_value()) {
        dpi_scale_ = create_data_->dpi_scale;
        pixel_size_ = (size_ * dpi_scale_).makeRound();
        MONITORINFO info = {};
        info.cbSize = sizeof(info);
        if (GetMonitorInfoW(create_data_->monitor, &info)) {
            int monitor_width = (int)info.rcWork.right;
            int monitor_height = (int)info.rcWork.bottom;
            wnd_left = (monitor_width - (int)pixel_size_.width) / 2;
            wnd_top = (monitor_height - (int)pixel_size_.height) / 2;
        }
    } else {
        dpi_scale_ = graph2d::getInitialDesktopDpiScale();
        pixel_size_ = (size_ * dpi_scale_).makeRound();
        RECT work_area;
        if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &work_area, 0)) {
            int monitor_width = (int)work_area.right;
            int monitor_height = (int)work_area.bottom;
            wnd_left = (monitor_width - (int)pixel_size_.width) / 2;
            wnd_top = (monitor_height - (int)pixel_size_.height) / 2;
        }
    }
    CreateWindowExW(ex_style, wnd_class_name, title.c_str(), style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, this);
}

ATOM RegisterWindowClass(HINSTANCE hInstance, const wchar_t* class_name, HICON icon, WndProc wnd_proc) {
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    //wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;// | CS_DBLCLKS;
    wcex.style = CS_DBLCLKS; /*| CS_OWNDC*/;
    wcex.lpfnWndProc = wnd_proc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = icon;
    wcex.hIconSm = icon;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr; // MAKEINTRESOURCE(IDC_TRYWIN32);
    wcex.lpszClassName = class_name;

    return RegisterClassExW(&wcex);
}

static int MakeButtonMask(WPARAM wParam) {
    int mask = kwui::ButtonState::NO_BUTTON;
    if (wParam & MK_LBUTTON) mask |= kwui::ButtonState::LEFT_BUTTON;
    if (wParam & MK_MBUTTON) mask |= kwui::ButtonState::MIDDLE_BUTTON;
    if (wParam & MK_RBUTTON) mask |= kwui::ButtonState::RIGHT_BUTTON;
    return mask;
}

static int GetModifiersState() {
    int state = scene2d::NO_MODIFILER;
    if (GetKeyState(VK_LCONTROL) & 0x8000) state |= scene2d::LCTRL_MODIFIER;
    if (GetKeyState(VK_RCONTROL) & 0x8000) state |= scene2d::RCTRL_MODIFIER;
    if (GetKeyState(VK_LSHIFT) & 0x8000) state |= scene2d::LSHIFT_MODIFIER;
    if (GetKeyState(VK_RSHIFT) & 0x8000) state |= scene2d::RSHIFT_MODIFIER;
    if (GetKeyState(VK_LMENU) & 0x8000) state |= scene2d::LALT_MODIFIER;
    if (GetKeyState(VK_RMENU) & 0x8000) state |= scene2d::RALT_MODIFIER;
    return state;
}

static WPARAM MapLeftRightKeys(WPARAM vk, LPARAM lParam) {
    WPARAM new_vk = vk;
    UINT scancode = (lParam & 0x00ff0000) >> 16;
    int extended = (lParam & 0x01000000) != 0;

    switch (vk) {
    case VK_SHIFT:
        new_vk = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
        break;
    case VK_CONTROL:
        new_vk = extended ? VK_RCONTROL : VK_LCONTROL;
        break;
    case VK_MENU:
        new_vk = extended ? VK_RMENU : VK_LMENU;
        break;
    default:
        // not a key we map from generic to left/right specialized
        //  just return it.
        new_vk = vk;
        break;
    }

    return new_vk;
}
static std::wstring IME_GetCompositionString(HIMC himc, DWORD str) {
    LONG length = ImmGetCompositionStringW(himc, str, NULL, 0);
    if (length < sizeof(WCHAR))
        return std::wstring();

    length /= sizeof(WCHAR);
    std::wstring text(length, L'\0');
    ImmGetCompositionStringW(himc, str, &text[0], length * sizeof(WCHAR));
    return text;
}
static int IME_GetCaretPos(HIMC himc) {
    return (int)ImmGetCompositionStringW(himc, GCS_CURSORPOS, NULL, 0);
}

LRESULT DialogWin32::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        OnCreate();
        break;
    case WM_DESTROY:
        OnDestroy();
        break;
    case WM_DPICHANGED:
        OnDpiChanged(LOWORD(wParam), (const RECT*)lParam);
        return 0;
    case WM_SIZE:
        pixel_size_ = scene2d::DimensionF((float)LOWORD(lParam), (float)HIWORD(lParam));
        size_ = scene2d::DimensionF(pixel_size_.width / dpi_scale_, pixel_size_.height / dpi_scale_);
        OnResize();
        break;
    case WM_NCCALCSIZE: {
        if (!popup_shadow_data_.has_value())
            return DefWindowProcW(hWnd, message, wParam, lParam);
        WINDOWPLACEMENT placement = {};
        if (GetWindowPlacement(hWnd, &placement)) {
            if (placement.showCmd == SW_SHOWMINIMIZED) {
                return DefWindowProcW(hWnd, message, wParam, lParam);
            }
        }
        LPNCCALCSIZE_PARAMS ncParams = (LPNCCALCSIZE_PARAMS)lParam;
        ncParams->rgrc[0].top -= 0;
        ncParams->rgrc[0].left -= 0;
        ncParams->rgrc[0].bottom += 0;
        ncParams->rgrc[0].right += 0;
        return 0;
    }
    case WM_NCHITTEST: {
        if (!popup_shadow_data_.has_value())
            return DefWindowProcW(hWnd, message, wParam, lParam);
        RECT rect;
        GetWindowRect(hWnd, &rect);
        float x = (float)(GET_X_LPARAM(lParam) - rect.left) / dpi_scale_;
        float y = (float)(GET_Y_LPARAM(lParam) - rect.top) / dpi_scale_;
        if (scene_->pickNode(scene2d::PointF(x, y), scene2d::NODE_FLAG_CLICKABLE, NULL)) {
            return HTCLIENT;
        } else {
            return HTCAPTION;
        }
    }
                     //case WM_NCACTIVATE:
                         //c2_log("DialogWin32 HWND %p NCACTIVE wParam %d\n", hWnd, wParam);
                         //OnActivate(wParam);
                         //return FALSE;
    case WM_DISPLAYCHANGE:
        //c2_log("recv WM_DISPLAYCHANGE\n");
        DiscardDeviceResources();
        OnPaint();
        break;
    case WM_PAINT:
        OnPaint();
        break;
    case WM_WINDOWPOSCHANGING:
        return DefWindowProcW(hwnd_, message, wParam, lParam);
    case WM_WINDOWPOSCHANGED: {
        OnWindowPosChanged((WINDOWPOS*)lParam);
        return DefWindowProcW(hwnd_, message, wParam, lParam);
    }
    case WM_ERASEBKGND: // don't want flicker
        return true;
    case WM_KEYDOWN:
        //c2_log("WM_KEYDOWN: wParam=0x%08x, mapped=0x%08x\n", wParam, MapLeftRightKeys(wParam, lParam));
        OnKeyDown(MapLeftRightKeys(wParam, lParam), GetModifiersState(), (lParam & (1 << 30)) != 0);
        break;
    case WM_KEYUP:
        OnKeyUp(MapLeftRightKeys(wParam, lParam), GetModifiersState());
        break;
    case WM_CHAR:
        //c2_log("WM_CHAR 0x%04x\n", wParam);
        switch (wParam) {
        case 0x08: // backspace
        case 0x0A: // linefeed
        case 0x1B: // escape
        case 0x09: // tab
        case 0x0D: // carriage return
            break;
        default:
            if (wParam >= 0x20)
                OnCharacter(wParam);
        }
        break;
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
        OnMouseDown(kwui::ButtonState::LEFT_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_LBUTTONUP:
        OnMouseUp(kwui::ButtonState::LEFT_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
        OnMouseDown(kwui::ButtonState::MIDDLE_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_MBUTTONUP:
        OnMouseUp(kwui::ButtonState::MIDDLE_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
        OnMouseDown(kwui::ButtonState::RIGHT_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_RBUTTONUP:
        OnMouseUp(kwui::ButtonState::RIGHT_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_MOUSEMOVE:
        mouse_position_ = scene2d::PointF((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam))
            / dpi_scale_;
        OnMouseMove(MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_NCMOUSEMOVE:
        if (!popup_shadow_data_.has_value())
            return DefWindowProcW(hWnd, message, wParam, lParam);
        mouse_position_ = scene2d::PointF((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam))
            / dpi_scale_;
        OnMouseMove(MakeButtonMask(wParam), GetModifiersState());
        return DefWindowProcW(hWnd, message, wParam, lParam);
    case WM_MOUSEWHEEL:
        OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam), MakeButtonMask(LOWORD(wParam)), GetModifiersState());
        break;
    case WM_MOUSEHWHEEL:
        OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam), MakeButtonMask(LOWORD(wParam)), GetModifiersState(), true);
        break;
    case WM_MOUSELEAVE:
        //c2_log("mouse leave\n");
        mouse_event_tracking_ = false;
        RequestUpdate();
        return 0;
    case WM_MOUSEACTIVATE:
        if (flags_ & DIALOG_FLAG_POPUP) {
            return MA_NOACTIVATE;
        } else {
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }
        break;
    case WM_ACTIVATE: {
        LRESULT ret = DefWindowProcW(hWnd, message, wParam, lParam);
        if (GET_WM_ACTIVATE_STATE(wParam, lParam) == WA_INACTIVE) {
            OnActivate(false);
        } else {
            OnActivate(true);
        }
        return ret;
    }
    case WM_IME_STARTCOMPOSITION:
        //c2_log("WM_IME_STARTCOMPOSITION\n");
        OnImeStartComposition();
        break;
    case WM_IME_COMPOSITION:
        //c2_log("WM_IME_COMPOSITION lParam=0x%08X\n", lParam);
        himc_ = ImmGetContext(hWnd);
        if (himc_) {
            if (lParam & GCS_COMPSTR) {
                std::wstring utf16_text = IME_GetCompositionString(himc_, GCS_COMPSTR);
                //c2_log("  GCS_COMPSTR=[%s]\n",
                //       EncodingManager::Instance()->WideToUTF8(utf16_text).GetCString());
                // Send Editing Text
                if (lParam & GCS_CURSORPOS) {
                    int caret_pos = IME_GetCaretPos(himc_);
                    //c2_log("  GCS_CURSORPOS=[%d]\n", caret_pos);
                    OnImeComposition(utf16_text, absl::make_optional(caret_pos));
                } else {
                    OnImeComposition(utf16_text, absl::nullopt);
                }
            }
            if (lParam & GCS_RESULTSTR) {
                std::wstring utf16_text = IME_GetCompositionString(himc_, GCS_RESULTSTR);
                //c2_log("  GCS_RESULTSTR=[%s]\n",
                //       EncodingManager::Instance()->WideToUTF8(utf16_text).GetCString());
                // Send Input Text
                OnImeCommit(utf16_text);
            }
            ImmReleaseContext(hWnd, himc_);
            himc_ = NULL;
        }
        break;
    case WM_IME_ENDCOMPOSITION:
        OnImeEndComposition();
        //c2_log("WM_IME_ENDCOMPOSITION\n");
        break;
    case WM_TIMER:
        if (wParam == ANIMATION_TIMER_EVENT)
            OnAnimationTimerEvent();
        break;
    case WM_SETFOCUS:
        if (auto node = focused_node_.upgrade()) {
            node->state_ |= scene2d::NODE_STATE_FOCUSED;
            scene2d::FocusEvent focus_in(node.get(), scene2d::FOCUS_IN);
            scene_->dispatchEvent(node.get(), focus_in, true);
        }
        return 0;
    case WM_KILLFOCUS:
        if (auto node = focused_node_.upgrade()) {
            node->state_ &= ~scene2d::NODE_STATE_FOCUSED;
            scene2d::FocusEvent focus_out(node.get(), scene2d::FOCUS_OUT);
            scene_->dispatchEvent(node.get(), focus_out, true);
        }
        return 0;
    case WM_SETCURSOR:
        if (auto node = hovered_node_.upgrade()) {
            style::CursorType cursor = node->computedStyle().cursor;
            if (cursor == style::CursorType::None) SetCursor(NULL);
            else if (cursor == style::CursorType::Auto) SetCursor(s_preloaded_cursors[CURSOR_ARROW]);
            else if (cursor == style::CursorType::Default) SetCursor(s_preloaded_cursors[CURSOR_ARROW]);
            else if (cursor == style::CursorType::Crosshair) SetCursor(s_preloaded_cursors[CURSOR_CROSS]);
            else if (cursor == style::CursorType::Pointer) SetCursor(s_preloaded_cursors[CURSOR_HAND]);
            else if (cursor == style::CursorType::Text) SetCursor(s_preloaded_cursors[CURSOR_IBEAM]);
            else if (cursor == style::CursorType::Wait) SetCursor(s_preloaded_cursors[CURSOR_WAIT]);
            else SetCursor(s_preloaded_cursors[CURSOR_ARROW]);
        } else {
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }
        return TRUE;
    case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE) {
            OnCloseSysCommand(*this);
            return 0;
        }
        return DefWindowProcW(hWnd, message, wParam, lParam);
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}
LRESULT CALLBACK DialogWin32::WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    DialogWin32* me;
    if (message == WM_NCCREATE) {
        me = reinterpret_cast<DialogWin32*>(((LPCREATESTRUCTW)lParam)->lpCreateParams);
        me->hwnd_ = hWnd;
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)me);
    } else {
        me = reinterpret_cast<DialogWin32*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }
    if (me)
        return me->WindowProc(hWnd, message, wParam, lParam);
    else
        return DefWindowProcW(hWnd, message, wParam, lParam);
}
void DialogWin32::OnCreate() {
    scene_ = std::make_unique<scene2d::Scene>(*this);
    /*
    _root = make_shared<Node2D>();

    _title_label = make_shared<LabelNode>();
    _title_label->SetFontSize(14);
    _title_label->SetColor(theme::H3_TEXT_COLOR);
    _title_label->SetOrigin(scene2d::PointF(16, 8));
    _root->AddChild(_title_label);

    _close_button = make_shared<ImageButtonNode>();
    _close_button->SetImageName("close_button.png", "close_button_hover.png");
    _close_button->SetBackgroundColor(NO_COLOR, theme::CLOSE_BUTTON_HOVER_COLOR);
    _close_button->SetSize({ 40, 32 });
    _close_button->SetOrigin({ _size.x - 40, 0 });
    _root->AddChild(_close_button);
    _close_button->SetClickedCallback(MakeCallback(this, &DialogWin32::OnCloseSysCommand));
    */
    if (popup_shadow_data_.has_value()) {
        float px = (float)popup_shadow_data_->padding;
        float py = (float)popup_shadow_data_->padding;
        popup_shadow_data_->monitor = create_data_.has_value()
            ? create_data_->monitor
            : NULL;
        popup_shadow_data_->dpi_scale = dpi_scale_;
        popup_shadow_ = std::make_unique<PopupShadow>(
            size_.width + px * 2,
            size_.height + py * 2,
            hwnd_, L"kwui::PopupShadow", *popup_shadow_data_);
    }
    SetupHook();
}
void DialogWin32::OnPaint() {
    if (!surface_) {
        LOG(WARNING) << "OnPaint: No surface.";
        return;
    }

    scene_->resolveStyle();
    scene_->computeLayout(size_.width, size_.height);

    bool ok = false;
    int tries = 0;
    while (tries < 2) {
        PAINTSTRUCT ps;
        BeginPaint(hwnd_, &ps);
        auto pi = surface_->beginPaint();

        pi->clear(theme::BACKGROUND_COLOR);
        scene_->paint(pi.get());

        ok = surface_->endPaint();
        if (!ok) {
            EndPaint(hwnd_, &ps);

            DiscardDeviceResources();
            recreateSurface();
            ++tries;

            continue;
        }
        if (!ok)
            LOG(INFO) << "OnPaint failed";
        EndPaint(hwnd_, &ps);
        break;
    }
    if (ok)
        surface_->swapBuffers();

    scene_->runPostRenderTasks();
}
void DialogWin32::OnResize() {
    LOG(INFO) << "OnResize pixelSize " << pixel_size_.width << "x" << pixel_size_.height << "px";
    ClosePopups();
    UpdateBorderAndRenderTarget();
    RequestPaint();
}
void DialogWin32::Close() {
    if (animation_timer_id_) {
        KillTimer(hwnd_, animation_timer_id_);
        animation_timer_id_ = 0;
        animating_nodes_.clear();
    }
    if (popup_shadow_) {
        popup_shadow_->Close();
        popup_shadow_ = nullptr;
    }
    SendMessageW(hwnd_, WM_CLOSE, 0, 0);
}
void DialogWin32::RequestPaint() {
    InvalidateRect(hwnd_, NULL, FALSE);
}
void DialogWin32::RequestUpdate() {
    scene_->resolveStyle();
    scene_->computeLayout(size_.width, size_.height);

    POINT point;
    if (GetCursorPos(&point) && ScreenToClient(hwnd_, &point)) {
        mouse_position_ = scene2d::PointF((float)point.x, (float)point.y);
        mouse_position_ /= dpi_scale_;
    } else {
        mouse_position_ = scene2d::PointF::fromAll(-1);
    }
    UpdateHoveredNode();
    UpdateFocusedNode();
}
void DialogWin32::RequestAnimationFrame(scene2d::Node* node) {
    auto it = find_if(animating_nodes_.begin(), animating_nodes_.end(),
        [&](const auto& link) {
            return link.get() == node;
        });
    if (it == animating_nodes_.end())
        animating_nodes_.push_back(node->weakProxy());
    if (!animating_nodes_.empty() && !animation_timer_id_) {
        animation_timer_id_ = SetTimer(hwnd_, ANIMATION_TIMER_EVENT, USER_TIMER_MINIMUM, NULL);
    }
}
void DialogWin32::UpdateCaretRect(const scene2d::PointF& origin, const scene2d::DimensionF& size) {
    scene2d::PointF top_left = (origin * dpi_scale_).makeRound();
    scene2d::PointF bottom_right = ((origin + scene2d::PointF(size.width, size.height)) * dpi_scale_).makeRound();

    COMPOSITIONFORM cof;
    cof.dwStyle = CFS_RECT;
    cof.ptCurrentPos.x = (LONG)top_left.x;
    cof.ptCurrentPos.y = (LONG)top_left.y;
    cof.rcArea.left = (LONG)top_left.x;
    cof.rcArea.top = (LONG)top_left.y;
    cof.rcArea.right = (LONG)bottom_right.x;
    cof.rcArea.bottom = (LONG)bottom_right.y;

    CANDIDATEFORM caf;
    caf.dwIndex = 0;
    caf.dwStyle = CFS_EXCLUDE;
    caf.ptCurrentPos.x = (LONG)top_left.x;
    caf.ptCurrentPos.y = (LONG)top_left.y;
    caf.rcArea.left = (LONG)top_left.x;
    caf.rcArea.top = (LONG)top_left.y;
    caf.rcArea.right = (LONG)bottom_right.x;
    caf.rcArea.bottom = (LONG)bottom_right.y;

    if (himc_) {
        ImmSetCompositionWindow(himc_, &cof);
        ImmSetCandidateWindow(himc_, &caf);
    } else {
        himc_ = ImmGetContext(hwnd_);
        if (himc_) {
            ImmSetCompositionWindow(himc_, &cof);
            ImmSetCandidateWindow(himc_, &caf);
            ImmReleaseContext(hwnd_, himc_);
        }
    }
}
void DialogWin32::OnKeyDown(int key, int modifiers, bool prev_down) {
    if (key == VK_RETURN)
        return OnEnterKeyDown(*this);
    if (key == VK_ESCAPE)
        return OnEscapeKeyDown(*this);
    if (kwui::Application::scriptReloadEnabled()) {
        if (key == VK_F5 && modifiers == 0)
            OnF5Down(*this);
    }

    base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
    if (node) {
        scene2d::KeyEvent key_down(node.get(), scene2d::KEY_DOWN, make_vkey(key), modifiers);
        scene_->dispatchEvent(node.get(), key_down, true);
    }
}
void DialogWin32::OnKeyUp(int key, int modifiers) {
    if (key == VK_RETURN)
        return OnEnterKeyUp(*this);
    if (key == VK_ESCAPE)
        return OnEscapeKeyUp(*this);

    base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
    if (node) {
        scene2d::KeyEvent key_up(node.get(), scene2d::KEY_UP, make_vkey(key), modifiers);
        scene_->dispatchEvent(node.get(), key_up, true);
    }
}
void DialogWin32::OnCharacter(wchar_t ch) {
    base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
    if (node) {
        scene2d::ImeEvent chars(node.get(), scene2d::CHARS, std::wstring(1, ch));
        scene_->dispatchEvent(node.get(), chars, true);
    }
}
void DialogWin32::OnImeComposition(const std::wstring& text, absl::optional<int> caret_pos) {
    base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
    if (node) {
        scene2d::ImeEvent composing(node.get(), scene2d::COMPOSING, text, caret_pos);
        scene_->dispatchEvent(node.get(), composing, true);
    }
}
void DialogWin32::OnImeCommit(const std::wstring& text) {
    base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
    if (node) {
        scene2d::ImeEvent commit(node.get(), scene2d::COMMIT, text);
        scene_->dispatchEvent(node.get(), commit, true);
    }
}
void DialogWin32::OnImeStartComposition() {
    // Node2DRef node = _focused_node.lock();
    // if (node) {
    //     node->OnImeStartComposition(*this);
    //     scene2d::PointF origin, size;
    //     if (node->QueryImeCaretRect(origin, size))
    //         UpdateCaretRect(node->MapPointToRoot(origin), size);
    // }
    base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
    if (node) {
        scene2d::ImeEvent start_compose(node.get(), scene2d::START_COMPOSE);
        scene_->dispatchEvent(node.get(), start_compose, true);
        if (start_compose.caret_rect_) {
            UpdateCaretRect(
                scene_->mapPointToScene(
                    node.get(),
                    start_compose.caret_rect_->origin(),
                    true),
                start_compose.caret_rect_->size());
        }
    }
}
void DialogWin32::OnImeEndComposition() {
    base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
    if (node) {
        scene2d::ImeEvent end_compose(node.get(), scene2d::END_COMPOSE);
        scene_->dispatchEvent(node.get(), end_compose, true);
    }
}
void DialogWin32::OnMouseDown(scene2d::ButtonState button, int buttons, int modifiers) {
    if (button == buttons && !mouse_capture_) {
        //c2_log("capture mouse\n");
        mouse_capture_ = true;
        SetCapture(hwnd_);
    }

    scene2d::Node* node;
    scene2d::PointF local_pos;
    node = scene_->pickNode(mouse_position_, scene2d::NODE_FLAG_CLICKABLE, &local_pos);
    if (node) {
        //LOG(INFO) << "mouse down " << local_pos;
        active_node_ = node->weaken();
        node->state_ |= scene2d::NODE_STATE_ACTIVE;
        scene2d::MouseEvent mouse_down(node, scene2d::MOUSE_DOWN, mouse_position_, local_pos, button, buttons, modifiers);
        scene_->dispatchEvent(node, mouse_down, true);
    }

    node = scene_->pickNode(mouse_position_, scene2d::NODE_FLAG_FOCUSABLE);
    base::object_refptr<scene2d::Node> old_focused = focused_node_.upgrade();
    if (node) {
        if (node != old_focused.get()) {
            if (old_focused) {
                old_focused->state_ &= ~scene2d::NODE_STATE_FOCUSED;
                scene2d::FocusEvent focus_out(old_focused.get(), scene2d::FOCUS_OUT);
                scene_->dispatchEvent(old_focused.get(), focus_out, true);
            }
        }
        focused_node_ = node->weaken();
        node->state_ |= scene2d::NODE_STATE_FOCUSED;
        scene2d::FocusEvent focus_in(node, scene2d::FOCUS_IN);
        scene_->dispatchEvent(focused_node_.get(), focus_in, true);
    }
}
void DialogWin32::OnMouseUp(scene2d::ButtonState button, int buttons, int modifiers) {
    if (buttons == 0 && mouse_capture_) {
        mouse_capture_ = false;
        //c2_log("release mouse\n");
        ReleaseCapture();
    }

    base::object_refptr<scene2d::Node> node = active_node_.upgrade();
    if (node) {
        active_node_ = nullptr;
        node->state_ &= ~scene2d::NODE_STATE_ACTIVE;
        scene2d::PointF local_pos = mouse_position_ - scene_->mapPointToScene(node.get(), scene2d::PointF(), true);
        scene2d::MouseEvent mouse_up(node.get(), scene2d::MOUSE_UP, mouse_position_, local_pos, button, buttons, modifiers);
        scene_->dispatchEvent(node.get(), mouse_up, true);
    }
}
void DialogWin32::OnMouseMove(int buttons, int modifiers) {
    //c2_log("OnMouseMove %.0f, %.0f\n", _mouse_position.x, _mouse_position.y);
    UpdateMouseTracking();
    UpdateHoveredNode();
    base::object_refptr<scene2d::Node> node = active_node_ ? active_node_.upgrade() : hovered_node_.upgrade();
    if (node) {
        scene2d::PointF local_pos = mouse_position_ - scene_->mapPointToScene(node.get(), scene2d::PointF(), true);
        scene2d::MouseEvent mouse_move(node.get(), scene2d::MOUSE_MOVE, mouse_position_, local_pos, kwui::ButtonState::NO_BUTTON, buttons, modifiers);
        scene_->dispatchEvent(node.get(), mouse_move, true);
        RequestPaint();
    }
}
void DialogWin32::OnMouseWheel(int delta, int buttons, int modifiers, bool hwheel)
{
    scene2d::Node* node;
    scene2d::PointF local_pos;
    node = scene_->pickNode(mouse_position_, scene2d::NODE_FLAG_SCROLLABLE, &local_pos);
    if (node) {
        float wheel_delta = float(delta) / WHEEL_DELTA;
        //LOG(INFO) << "mouse wheel " << local_pos << ", delta=" << wheel_delta << ", hwheel=" << hwheel;
        scene2d::MouseCommand cmd = hwheel ? scene2d::MOUSE_HWHEEL : scene2d::MOUSE_WHEEL;
        scene2d::MouseEvent mouse_wheel(node, cmd, mouse_position_, local_pos, wheel_delta, buttons, modifiers);
        scene_->dispatchEvent(node, mouse_wheel, true);
    }
}
void DialogWin32::UpdateHoveredNode() {
    scene2d::PointF local_pos;
    scene2d::Node* node = scene_->pickNode(mouse_position_, scene2d::NODE_FLAG_HOVERABLE, &local_pos);
    base::object_refptr<scene2d::Node> old_hovered = hovered_node_.upgrade();
    if (node != old_hovered.get()) {
        if (old_hovered) {
            old_hovered->state_ &= ~scene2d::NODE_STATE_HOVER;
            scene2d::MouseEvent hover_leave(old_hovered.get(), scene2d::MOUSE_OUT, mouse_position_, scene2d::PointF());
            scene_->dispatchEvent(old_hovered.get(), hover_leave, true);
        }
        hovered_node_ = node ? node->weaken() : base::object_weakptr<scene2d::Node>();
        if (node) {
            node->state_ |= scene2d::NODE_STATE_HOVER;
            scene2d::MouseEvent hover_enter(node, scene2d::MOUSE_OVER, mouse_position_, local_pos);
            scene_->dispatchEvent(node, hover_enter, true);
        }
        RequestPaint();
    }
}
void DialogWin32::UpdateFocusedNode() {
    // Focused node become invisible
    if (auto node = focused_node_.upgrade()) {
        if (!node->visibleInHierarchy()) {
            node->state_ &= ~scene2d::NODE_STATE_FOCUSED;
            scene2d::FocusEvent focus_out(node.get(), scene2d::FOCUS_OUT);
            scene_->dispatchEvent(node.get(), focus_out, true);
            focused_node_ = nullptr;
        }
    }
}
void DialogWin32::recreateSurface() {
#if WITH_SKIA
    xskia::PaintSurfaceX::Configuration config;
    config.hwnd = hwnd_;
    config.pixel_size = pixel_size_;
    config.dpi_scale = dpi_scale_;
    surface_ = xskia::PaintSurfaceX::create(config);
    LOG(INFO) << "DialogWin32::recreateSurface() hwnd=" << std::hex << hwnd_
        << " size=(" << size_.width << "x" << size_.height << ")"
        << " dpi_scale=" << dpi_scale_
        << " surface=" << std::hex << surface_.get();
#else
    graphics::PaintSurfaceWindowD2D::Configuration config;
    config.hwnd = hwnd_;
    config.size = size_;
    config.dpi_scale = dpi_scale_;
    surface_ = graphics::PaintSurfaceWindowD2D::create(config);
    LOG(INFO) << "DialogWin32::recreateSurface() hwnd=" << std::hex << hwnd_
        << " size=(" << size_.width << "x" << size_.height << ")"
        << " dpi_scale=" << dpi_scale_
        << " surface=" << std::hex << surface_.get();
#endif
}
void DialogWin32::OnDpiChanged(UINT dpi, const RECT* rect) {
    LOG(INFO) << "DialogWin32 dpi changed to " << dpi
        << "size " << rect->right - rect->left << "x" << rect->bottom - rect->top << "px";
    float new_dpi_scale = (float)dpi / USER_DEFAULT_SCREEN_DPI;
    if (new_dpi_scale == dpi_scale_)
        return;

    dpi_scale_ = new_dpi_scale;
    if (popup_shadow_) {
        popup_shadow_->_render_dpi_scale = dpi_scale_;
        float padding_px = roundf(popup_shadow_data_->padding * dpi_scale_);
        float double_padding_px = roundf(2.0f * popup_shadow_data_->padding * dpi_scale_);
        SetWindowPos(popup_shadow_->GetHwnd(), hwnd_,
            (LONG)(rect->left - padding_px),
            (LONG)(rect->top - padding_px),
            (LONG)(rect->right - rect->left + double_padding_px),
            (LONG)(rect->bottom - rect->top + double_padding_px),
            SWP_NOACTIVATE);
        /*
        c2_log("...Adjust PopupShadow geometry to pos=(%lu, %lu) px, size=(%lu, %lu) px\n",
            (LONG)(rect->left - padding_px),
            (LONG)(rect->top - padding_px),
            (LONG)(rect->right - rect->left + double_padding_px),
            (LONG)(rect->bottom - rect->top + double_padding_px));
        */
    }
    SetWindowPos(hwnd_, NULL,
        rect->left,
        rect->top,
        rect->right - rect->left,
        rect->bottom - rect->top,
        SWP_NOZORDER | SWP_NOACTIVATE);

    DiscardDeviceResources();
    UpdateBorderAndRenderTarget();
    RequestUpdate();
}
void DialogWin32::OnCloseSysCommand(EventContext& ctx) {
    auto ret = kwui::ScriptEngine::get()->sendEvent("dialog:request-close", id_);
    bool any_true = false;
    ret.visitArray([&](int, const kwui::ScriptValue& v) {
        if (v.toBool())
            any_true = true;
        });
    if (any_true)
        return;
    Close();
}
void DialogWin32::UpdateMouseTracking() {
    if (!mouse_event_tracking_) {
        mouse_event_tracking_ = true;
        TRACKMOUSEEVENT tme = {};
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = hwnd_;
        tme.dwFlags = TME_LEAVE;
        TrackMouseEvent(&tme);
    }
}
void DialogWin32::OnDestroy() {
    ReleaseHook();
    ClosePopups();
    scene_ = nullptr;
    // _title_label = nullptr;
    // _close_button = nullptr;
    surface_ = nullptr;
    if ((flags_ & DIALOG_FLAG_MAIN) && g_dialog_map.size() == 1)
        PostQuitMessage(0);
    if (hwnd_parent_ && visible_)
        EnableWindow(hwnd_parent_, true);
}

void DialogWin32::OnActivate(bool active) {
    //c2_log("%p OnActive %d\n", _hwnd, active);
    if (active && popup_shadow_) {
        SetWindowPos(popup_shadow_->GetHwnd(),
            hwnd_,
            0, 0,
            0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOCOPYBITS);
    }
    if (!active) {
        ClosePopups();
    }
}
void DialogWin32::OnWindowPosChanged(WINDOWPOS* wnd_pos) {
    if (on_window_pos_changed_)
        return;
    if (!popup_shadow_)
        return;
    on_window_pos_changed_ = true;
    //c2_log("OnWindowPosChanged, flags = 0x%04X\n", wnd_pos->flags);
    if (!(wnd_pos->flags & SWP_NOMOVE)
        || !(wnd_pos->flags & SWP_NOSIZE)
        || !(wnd_pos->flags & SWP_NOACTIVATE)) {
        BOOL success;
        auto hdwp = BeginDeferWindowPos(1);
        UINT flags = SWP_NOACTIVATE;
        if (wnd_pos->flags & SWP_NOMOVE)
            flags |= SWP_NOMOVE;
        if (wnd_pos->flags & SWP_NOSIZE)
            flags |= SWP_NOSIZE;
        float padding_px = roundf(popup_shadow_data_->padding * dpi_scale_);
        float double_padding_px = roundf(2.0f * popup_shadow_data_->padding * dpi_scale_);
        //c2_log("BeginDeferWindowPos flags=%08x padding_px=%.0f\n", wnd_pos->flags, padding_px);
        //c2_log("  popup DeferWindowPos cx=%d, cy=%d\n",
        //       wnd_pos->cx + (int)double_padding_px,
        //       wnd_pos->cy + (int)double_padding_px);
        hdwp = DeferWindowPos(hdwp, popup_shadow_->GetHwnd(), hwnd_,
            wnd_pos->x - (int)padding_px,
            wnd_pos->y - (int)padding_px,
            wnd_pos->cx + (int)double_padding_px,
            wnd_pos->cy + (int)double_padding_px,
            flags);
        success = EndDeferWindowPos(hdwp);
        //c2_log("EndDeferWindowPos success=%d\n", success);
    }
    on_window_pos_changed_ = false;
}
void DialogWin32::DiscardDeviceResources() {
    // DiscardNodeDeviceResources(_root.get());
    scene_->discardDeviceResources();
    if (surface_) surface_->discardDeviceResources();
}
void DialogWin32::OnEnterKeyDown(EventContext& ctx)
{
    kwui::ScriptEngine::get()->postEvent("dialog:enter-key-down", id_);
}
void DialogWin32::OnEnterKeyUp(EventContext& ctx)
{
    kwui::ScriptEngine::get()->postEvent("dialog:enter-key-up", id_);
}
void DialogWin32::OnEscapeKeyDown(EventContext& ctx)
{
    kwui::ScriptEngine::get()->postEvent("dialog:escape-key-down", id_);
}
void DialogWin32::OnEscapeKeyUp(EventContext& ctx)
{
    kwui::ScriptEngine::get()->postEvent("dialog:escape-key-up", id_);
}
void DialogWin32::OnF5Down(EventContext& ctx)
{
    if (kwui::Application::scriptReloadEnabled()) {
        ResourceManager::instance()->clearCache();
        scene_->reloadScriptModule();
        OnPaint();
    }
}
void DialogWin32::DiscardNodeDeviceResources(scene2d::Node* node) {
    // node->DiscardDeviceResources();
    // for (auto& child : node->GetChildren())
    //     DiscardNodeDeviceResources(child.get());
}

void DialogWin32::UpdateBorderAndRenderTarget() {
    if (popup_shadow_data_.has_value()) {
        float border_radius = roundf(theme::DIALOG_BORDER_RADIUS * dpi_scale_);
        HRGN rgn = CreateRoundRectRgn(
            0, 0,
            (int)pixel_size_.width + 1, (int)pixel_size_.height + 1,
            (int)border_radius, (int)border_radius);
        SetWindowRgn(hwnd_, rgn, FALSE);
    }

    if (!surface_) {
        recreateSurface();
    } else {
        //LOG(INFO) << "RT resize " << (UINT32)_pixel_size.width << "x" << (UINT32)_pixel_size.height << "px";
        surface_->resize((int)pixel_size_.width, (int)pixel_size_.height, dpi_scale_);
    }
}
void DialogWin32::OnAnimationTimerEvent() {
    absl::Time timestamp = absl::Now();
    auto nodes = move(animating_nodes_);
    animating_nodes_.clear();
    for (auto& link : nodes) {
        auto node = link.upgrade();
        if (node)
            node->onAnimationFrame(timestamp);
    }
    if (animating_nodes_.empty() && animation_timer_id_) {
        KillTimer(hwnd_, animation_timer_id_);
        animation_timer_id_ = 0;
    }
}

LRESULT CALLBACK DialogWin32::hookProc(int code, WPARAM wParam, LPARAM lParam) {
    MSG* msg = (MSG*)lParam;
    if (wParam == PM_REMOVE) {
        if (msg->message == WM_LBUTTONDOWN || msg->message == WM_RBUTTONDOWN
            || msg->message == WM_NCLBUTTONDOWN || msg->message == WM_NCRBUTTONDOWN
            || msg->message == WM_SYSCHAR) {
            DialogWin32* dlg = nullptr;
            for (auto it = g_dialog_map.begin(); it != g_dialog_map.end(); ++it) {
                if (it->second->GetHwnd() == msg->hwnd) {
                    dlg = it->second;
                    break;
                }
            }
            if (dlg) {
                dlg->ClosePopups();
            }
        }
    }
    return ::CallNextHookEx(NULL, code, wParam, lParam);
}
void DialogWin32::ClosePopups()
{
    bool found;
    do {
        found = false;
        for (auto it = g_dialog_map.begin(); it != g_dialog_map.end(); ++it) {
            if (it->second->hwnd_anchor_ == hwnd_) {
                auto dlg = it->second;
                g_dialog_map.erase(it);
                dlg->Close();
                found = true;
                break;
            }
        }
    } while (found);
}
void DialogWin32::SetupHook()
{
    hook_ = ::SetWindowsHookExW(WH_GETMESSAGE, &DialogWin32::hookProc, NULL, GetCurrentThreadId());
}

void DialogWin32::ReleaseHook()
{
    if (hook_) {
        ::UnhookWindowsHookEx(hook_);
        hook_ = NULL;
    }
}

} // namespace windows