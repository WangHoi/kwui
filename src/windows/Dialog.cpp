#include "Dialog.h"
#include "base/log.h"
#include "scene2d/scene2d.h"
#include "EncodingManager.h"
#include "graphics/GraphicDevice.h"
#include "graphics/Painter.h"
#include "theme.h"
#include "absl/strings/str_format.h"
#include "absl/strings/numbers.h"
#include "absl/functional/bind_front.h"
#include "absl/time/clock.h"
#include "graph2d/Painter.h"
#include "api/kwui/ScriptEngine.h"
#include "api/kwui/Application.h"

namespace windows {
typedef LRESULT(CALLBACK* WndProc)(HWND, UINT, WPARAM, LPARAM);
static ATOM RegisterWindowClass(HINSTANCE hInstance,
    const wchar_t* class_name,
    HICON icon,
    WndProc wnd_proc);

static constexpr UINT_PTR ANIMATION_TIMER_EVENT = 0xFFFF00A0;
static HCURSOR s_preloaded_cursors[NUM_CURSOR_TYPES] = {};

typedef HRESULT (WINAPI* GETDPIFORMONITOR)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);

typedef BOOL(WINAPI* ADJUSTWINDOWRECTEXFORDPI)(LPRECT, DWORD, BOOL, DWORD, UINT);

float Dialog::getMonitorDpiScale(HMONITOR monitor)
{
    static GETDPIFORMONITOR pGETDPIFORMONITOR = NULL;
    if (!pGETDPIFORMONITOR) {
        HMODULE hm = LoadLibraryW(L"Shcore.dll");
        if (hm) {
            pGETDPIFORMONITOR = (GETDPIFORMONITOR)GetProcAddress(hm, "GetDpiForMonitor");
        }
        if (!pGETDPIFORMONITOR) {
            pGETDPIFORMONITOR = [](HMONITOR, MONITOR_DPI_TYPE, UINT* dpi_x, UINT* dpi_y) -> HRESULT {
                float dpi_scale = windows::graphics::GraphicDevice::instance()->GetInitialDesktopDpiScale();
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

static std::unordered_map<std::string, Dialog*> g_dialog_map;

Dialog::Dialog(const WCHAR* wnd_class_name,
    HICON icon, int flags,
    absl::optional<PopupShadowData> popup_shadow,
    absl::optional<CreateData> create_data)
    : _hwnd_parent(NULL), _hwnd_anchor(NULL), _hwnd(NULL)
    , _flags(flags), _create_data(create_data)
    , _visible(false), _first_show_window(true)
    , _mouse_event_tracking(false), _mouse_capture(false)
    , _popup_shadow_data(popup_shadow)
    , _on_window_pos_changed(false)
    , _dpi_scale(1.0f)
    , _himc(NULL)
    , _animation_timer_id(0) {

    if (create_data.has_value()) {
        _dpi_scale = create_data.value().dpi_scale;
    }

    _mouse_position = scene2d::PointF(_size.width * 0.5f, _size.height * 0.5f);
    id_ = absl::StrFormat("%p", this);
    g_dialog_map[id_] = this;
    PreloadCursor();
    InitWindow(GetModuleHandle(NULL), wnd_class_name, icon);
}

Dialog::~Dialog() {
    g_dialog_map.erase(id_);
}

Dialog* Dialog::findDialogById(const std::string& id)
{
    auto it = g_dialog_map.find(id);
    return (it == g_dialog_map.end()) ? nullptr : it->second;
}

void Dialog::SetPopupAnchor(Dialog* anchor)
{
    _hwnd_anchor = anchor ? anchor->_hwnd : NULL;
}

void Dialog::SetVisible(bool visible) {
    if (_visible != visible) {
        _visible = visible;
    }
    if (_popup_shadow) {
        _popup_shadow->SetVisible(visible);
    }
    if (_visible) {
        if (_flags & DIALOG_FLAG_POPUP) {
            ShowWindow(_hwnd, SW_SHOWNOACTIVATE);
        } else {
            ShowWindow(_hwnd, SW_SHOWNORMAL);
        }
    } else {
        ShowWindow(_hwnd, SW_HIDE);
    }
    if (_hwnd_parent) {
        EnableWindow(_hwnd_parent, !_visible);
    }
    if (_visible && _first_show_window) {
        _first_show_window = false;
        UpdateWindow(_hwnd);
    }
}

scene2d::RectF Dialog::adjustWindowRect(const scene2d::RectF& rect,
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

void Dialog::Resize(float width, float height) {
    //if (_size.width == width && _size.height == height)
    //    return;
    //_size = scene2d::DimensionF(width, height);
    /* TODO: remove the code
    if (_popup_shadow) {
        float px = (float)_popup_shadow_data->padding ;
        float py = (float)_popup_shadow_data->padding;
        _popup_shadow->_size.Set(width + 2.0f * px, height + 2.0f * px);
    }
    */
    _size = scene2d::DimensionF(width, height);
    scene2d::DimensionF pixel_size = (_size * _dpi_scale).makeRound();
    RECT r = {};
    ::AdjustWindowRectEx(&r, GetWindowLongW(_hwnd, GWL_STYLE), FALSE, GetWindowLongW(_hwnd, GWL_EXSTYLE));
    SetWindowPos(_hwnd, NULL,
        0, 0,
        (int)pixel_size.width + (r.right - r.left),
        (int)pixel_size.height + (r.bottom - r.top),
        SWP_NOMOVE | SWP_NOACTIVATE);
}

void Dialog::setWindowPos(const scene2d::RectF& rect)
{
    SetWindowPos(_hwnd, NULL,
        (int)rect.left,
        (int)rect.top,
        (int)rect.width(),
        (int)rect.height(),
        SWP_NOACTIVATE);
}

void Dialog::SetTitle(const std::string& text) {
    // _title_label->SetText(text);
    auto utf16_text = EncodingManager::UTF8ToWide(text);
    SetWindowTextW(_hwnd, utf16_text.c_str());
}

void Dialog::InitWindow(HINSTANCE hInstance, const WCHAR* wnd_class_name, HICON icon) {
    auto title = EncodingManager::UTF8ToWide("");
    RegisterWindowClass(hInstance, wnd_class_name, icon, &Dialog::WndProcMain);
    DWORD style, ex_style;
    if (_flags & DIALOG_FLAG_MAIN) {
        if (_popup_shadow_data.has_value()) {
            style = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
            ex_style = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR
                | WS_EX_STATICEDGE | WS_EX_APPWINDOW;

        } else {
            style = WS_OVERLAPPEDWINDOW;
            ex_style = WS_EX_OVERLAPPEDWINDOW;
        }
    } else if (_flags & DIALOG_FLAG_POPUP) {
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
    if (_create_data.has_value()) {
        _dpi_scale = _create_data->dpi_scale;
        _pixel_size = (_size * _dpi_scale).makeRound();
        MONITORINFO info = {};
        info.cbSize = sizeof(info);
        if (GetMonitorInfoW(_create_data->monitor, &info)) {
            int monitor_width = (int)info.rcWork.right;
            int monitor_height = (int)info.rcWork.bottom;
            wnd_left = (monitor_width - (int)_pixel_size.width) / 2;
            wnd_top = (monitor_height - (int)_pixel_size.height) / 2;
        }
    } else {
        _dpi_scale = graphics::GraphicDevice::instance()->GetInitialDesktopDpiScale();
        _pixel_size = (_size * _dpi_scale).makeRound();
        RECT work_area;
        if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &work_area, 0)) {
            int monitor_width = (int)work_area.right;
            int monitor_height = (int)work_area.bottom;
            wnd_left = (monitor_width - (int)_pixel_size.width) / 2;
            wnd_top = (monitor_height - (int)_pixel_size.height) / 2;
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
    int mask = scene2d::NO_BUTTON;
    if (wParam & MK_LBUTTON) mask |= scene2d::LEFT_BUTTON;
    if (wParam & MK_MBUTTON) mask |= scene2d::MIDDLE_BUTTON;
    if (wParam & MK_RBUTTON) mask |= scene2d::RIGHT_BUTTON;
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

LRESULT Dialog::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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
        _pixel_size = scene2d::DimensionF((float)LOWORD(lParam), (float)HIWORD(lParam));
        _size = scene2d::DimensionF(_pixel_size.width / _dpi_scale, _pixel_size.height / _dpi_scale);
        OnResize();
        break;
    case WM_NCCALCSIZE: {
        if (!_popup_shadow_data.has_value())
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
        if (!_popup_shadow_data.has_value())
            return DefWindowProcW(hWnd, message, wParam, lParam);
        RECT rect;
        GetWindowRect(hWnd, &rect);
        float x = (float)(GET_X_LPARAM(lParam) - rect.left) / _dpi_scale;
        float y = (float)(GET_Y_LPARAM(lParam) - rect.top) / _dpi_scale;
        if (_scene->pickNode(scene2d::PointF(x, y), scene2d::NODE_FLAG_CLICKABLE, NULL)) {
            return HTCLIENT;
        } else {
            return HTCAPTION;
        }
    }
    //case WM_NCACTIVATE:
        //c2_log("Dialog HWND %p NCACTIVE wParam %d\n", hWnd, wParam);
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
        return DefWindowProcW(_hwnd, message, wParam, lParam);
    case WM_WINDOWPOSCHANGED: {
        OnWindowPosChanged((WINDOWPOS*)lParam);
        return DefWindowProcW(_hwnd, message, wParam, lParam);
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
        OnMouseDown(scene2d::LEFT_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_LBUTTONUP:
        OnMouseUp(scene2d::LEFT_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
        OnMouseDown(scene2d::MIDDLE_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_MBUTTONUP:
        OnMouseUp(scene2d::MIDDLE_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
        OnMouseDown(scene2d::RIGHT_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_RBUTTONUP:
        OnMouseUp(scene2d::RIGHT_BUTTON, MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_MOUSEMOVE:
        _mouse_position = scene2d::PointF((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam))
            / _dpi_scale;
        OnMouseMove(MakeButtonMask(wParam), GetModifiersState());
        break;
    case WM_NCMOUSEMOVE:
        if (!_popup_shadow_data.has_value())
            return DefWindowProcW(hWnd, message, wParam, lParam);
        _mouse_position = scene2d::PointF((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam))
            / _dpi_scale;
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
        _mouse_event_tracking = false;
        RequestUpdate();
        return 0;
    case WM_MOUSEACTIVATE:
        if (_flags & DIALOG_FLAG_POPUP) {
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
        _himc = ImmGetContext(hWnd);
        if (_himc) {
            if (lParam & GCS_COMPSTR) {
                std::wstring utf16_text = IME_GetCompositionString(_himc, GCS_COMPSTR);
                //c2_log("  GCS_COMPSTR=[%s]\n",
                //       EncodingManager::Instance()->WideToUTF8(utf16_text).GetCString());
                // Send Editing Text
                if (lParam & GCS_CURSORPOS) {
                    int caret_pos = IME_GetCaretPos(_himc);
                    //c2_log("  GCS_CURSORPOS=[%d]\n", caret_pos);
                    OnImeComposition(utf16_text, absl::make_optional(caret_pos));
                } else {
                    OnImeComposition(utf16_text, absl::nullopt);
                }
            }
            if (lParam & GCS_RESULTSTR) {
                std::wstring utf16_text = IME_GetCompositionString(_himc, GCS_RESULTSTR);
                //c2_log("  GCS_RESULTSTR=[%s]\n",
                //       EncodingManager::Instance()->WideToUTF8(utf16_text).GetCString());
                // Send Input Text
                OnImeCommit(utf16_text);
            }
            ImmReleaseContext(hWnd, _himc);
            _himc = NULL;
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
        if (auto node = _focused_node.upgrade()) {
            node->state_ |= scene2d::NODE_STATE_FOCUSED;
            scene2d::FocusEvent focus_in(node.get(), scene2d::FOCUS_IN);
            _scene->dispatchEvent(node.get(), focus_in, true);
        }
        return 0;
    case WM_KILLFOCUS:
        if (auto node = _focused_node.upgrade()) {
            node->state_ &= ~scene2d::NODE_STATE_FOCUSED;
            scene2d::FocusEvent focus_out(node.get(), scene2d::FOCUS_OUT);
            _scene->dispatchEvent(node.get(), focus_out, true);
        }
        return 0;
    case WM_SETCURSOR:
        if (auto node = _hovered_node.upgrade()) {
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
LRESULT CALLBACK Dialog::WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Dialog* me;
    if (message == WM_NCCREATE) {
        me = reinterpret_cast<Dialog*>(((LPCREATESTRUCTW)lParam)->lpCreateParams);
        me->_hwnd = hWnd;
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)me);
    } else {
        me = reinterpret_cast<Dialog*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }
    if (me)
        return me->WindowProc(hWnd, message, wParam, lParam);
    else
        return DefWindowProcW(hWnd, message, wParam, lParam);
}
void Dialog::OnCreate() {
    _scene = std::make_unique<scene2d::Scene>(*this);
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
    _close_button->SetClickedCallback(MakeCallback(this, &Dialog::OnCloseSysCommand));
    */
    if (_popup_shadow_data.has_value()) {
        float px = (float)_popup_shadow_data->padding;
        float py = (float)_popup_shadow_data->padding;
        _popup_shadow_data->monitor = _create_data.has_value()
            ? _create_data->monitor
            : NULL;
        _popup_shadow_data->dpi_scale = _dpi_scale;
        _popup_shadow = std::make_unique<PopupShadow>(
            _size.width + px * 2,
            _size.height + py * 2,
            _hwnd, L"kwui::PopupShadow", *_popup_shadow_data);
    }
    SetupHook();
}
void Dialog::OnPaint() {
    if (!_rt) {
        LOG(WARNING) << "OnPaint: No render target.";
        return;
    }

    _scene->resolveStyle();
    _scene->computeLayout(_size.width, _size.height);

    int tries = 0;
    while (tries < 2) {
        PAINTSTRUCT ps;
        BeginPaint(_hwnd, &ps);
        _rt->BeginDraw();

        graphics::Painter p(_rt.Get(), _mouse_position);
        p.Clear(theme::BACKGROUND_COLOR);
        graphics::PainterImpl pi(p);
        _scene->paint(&pi);

        HRESULT hr = _rt->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET) {
            LOG(INFO) << "Got D2DERR_RECREATE_TARGET";
            EndPaint(_hwnd, &ps);

            DiscardDeviceResources();
            RecreateRenderTarget();
            ++tries;

            continue;
        }
        if (FAILED(hr))
            LOG(INFO) << "OnPaint hr=" << std::hex << hr;
        EndPaint(_hwnd, &ps);
        break;
    }

    _scene->runPostRenderTasks();
}
void Dialog::OnResize() {
    LOG(INFO) << "OnResize pixelSize " << _pixel_size.width << "x" << _pixel_size.height << "px";
    ClosePopups();
    UpdateBorderAndRenderTarget();
    RequestPaint();
}
void Dialog::Close() {
    if (_animation_timer_id) {
        KillTimer(_hwnd, _animation_timer_id);
        _animation_timer_id = 0;
        _animating_nodes.clear();
    }
    if (_popup_shadow) {
        _popup_shadow->Close();
        _popup_shadow = nullptr;
    }
    SendMessageW(_hwnd, WM_CLOSE, 0, 0);
}
void Dialog::RequestPaint() {
    InvalidateRect(_hwnd, NULL, FALSE);
}
void Dialog::RequestUpdate() {
    _scene->resolveStyle();
    _scene->computeLayout(_size.width, _size.height);

    POINT point;
    if (GetCursorPos(&point) && ScreenToClient(_hwnd, &point)) {
        _mouse_position = scene2d::PointF((float)point.x, (float)point.y);
        _mouse_position /= _dpi_scale;
    } else {
        _mouse_position = scene2d::PointF::fromAll(-1);
    }
    UpdateHoveredNode();
    UpdateFocusedNode();
}
void Dialog::RequestAnimationFrame(scene2d::Node* node) {
    auto it = find_if(_animating_nodes.begin(), _animating_nodes.end(),
        [&](const auto& link) {
            return link.get() == node;
        });
    if (it == _animating_nodes.end())
        _animating_nodes.push_back(node->weakProxy());
    if (!_animating_nodes.empty() && !_animation_timer_id) {
        _animation_timer_id = SetTimer(_hwnd, ANIMATION_TIMER_EVENT, USER_TIMER_MINIMUM, NULL);
    }
}
void Dialog::UpdateCaretRect(const scene2d::PointF& origin, const scene2d::DimensionF& size) {
    scene2d::PointF top_left = (origin * _dpi_scale).makeRound();
    scene2d::PointF bottom_right = ((origin + scene2d::PointF(size.width, size.height)) * _dpi_scale).makeRound();

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

    if (_himc) {
        ImmSetCompositionWindow(_himc, &cof);
        ImmSetCandidateWindow(_himc, &caf);
    } else {
        _himc = ImmGetContext(_hwnd);
        if (_himc) {
            ImmSetCompositionWindow(_himc, &cof);
            ImmSetCandidateWindow(_himc, &caf);
            ImmReleaseContext(_hwnd, _himc);
        }
    }
}
void Dialog::OnKeyDown(int key, int modifiers, bool prev_down) {
    if (key == VK_RETURN)
        return OnEnterKeyDown(*this);
    if (key == VK_ESCAPE)
        return OnEscapeKeyDown(*this);
    if (kwui::Application::scriptReloadEnabled()) {
        if (key == VK_F5 && modifiers == 0)
            OnF5Down(*this);
    }

    base::object_refptr<scene2d::Node> node = _focused_node.upgrade();
    if (node) {
        scene2d::KeyEvent key_down(node.get(), scene2d::KEY_DOWN, key, modifiers);
        _scene->dispatchEvent(node.get(), key_down, true);
    }
}
void Dialog::OnKeyUp(int key, int modifiers) {
    if (key == VK_RETURN)
        return OnEnterKeyUp(*this);
    if (key == VK_ESCAPE)
        return OnEscapeKeyUp(*this);

    base::object_refptr<scene2d::Node> node = _focused_node.upgrade();
    if (node) {
        scene2d::KeyEvent key_up(node.get(), scene2d::KEY_UP, key, modifiers);
        _scene->dispatchEvent(node.get(), key_up, true);
    }
}
void Dialog::OnCharacter(wchar_t ch) {
    base::object_refptr<scene2d::Node> node = _focused_node.upgrade();
    if (node) {
        scene2d::ImeEvent chars(node.get(), scene2d::CHARS, std::wstring(1, ch));
        _scene->dispatchEvent(node.get(), chars, true);
    }
}
void Dialog::OnImeComposition(const std::wstring& text, absl::optional<int> caret_pos) {
    base::object_refptr<scene2d::Node> node = _focused_node.upgrade();
    if (node) {
        scene2d::ImeEvent composing(node.get(), scene2d::COMPOSING, text, caret_pos);
        _scene->dispatchEvent(node.get(), composing, true);
    }
}
void Dialog::OnImeCommit(const std::wstring& text) {
    base::object_refptr<scene2d::Node> node = _focused_node.upgrade();
    if (node) {
        scene2d::ImeEvent commit(node.get(), scene2d::COMMIT, text);
        _scene->dispatchEvent(node.get(), commit, true);
    }
}
void Dialog::OnImeStartComposition() {
    // Node2DRef node = _focused_node.lock();
    // if (node) {
    //     node->OnImeStartComposition(*this);
    //     scene2d::PointF origin, size;
    //     if (node->QueryImeCaretRect(origin, size))
    //         UpdateCaretRect(node->MapPointToRoot(origin), size);
    // }
    base::object_refptr<scene2d::Node> node = _focused_node.upgrade();
    if (node) {
        scene2d::ImeEvent start_compose(node.get(), scene2d::START_COMPOSE);
        _scene->dispatchEvent(node.get(), start_compose, true);
        if (start_compose.caret_rect_) {
            UpdateCaretRect(_scene->mapPointToScene(node.get(), start_compose.caret_rect_->origin()),
                start_compose.caret_rect_->size());
        }
    }
}
void Dialog::OnImeEndComposition() {
    base::object_refptr<scene2d::Node> node = _focused_node.upgrade();
    if (node) {
        scene2d::ImeEvent end_compose(node.get(), scene2d::END_COMPOSE);
        _scene->dispatchEvent(node.get(), end_compose, true);
    }
}
void Dialog::OnMouseDown(scene2d::ButtonState button, int buttons, int modifiers) {
    if (button == buttons && !_mouse_capture) {
        //c2_log("capture mouse\n");
        _mouse_capture = true;
        SetCapture(_hwnd);
    }

    if (button != scene2d::LEFT_BUTTON) return;
    scene2d::Node* node;
    scene2d::PointF local_pos;
    node = _scene->pickNode(_mouse_position, scene2d::NODE_FLAG_CLICKABLE, &local_pos);
    if (node) {
        //LOG(INFO) << "mouse down " << local_pos;
        _active_node = node->weaken();
        node->state_ |= scene2d::NODE_STATE_ACTIVE;
        scene2d::MouseEvent mouse_down(node, scene2d::MOUSE_DOWN, _mouse_position, local_pos, button, buttons, modifiers);
        _scene->dispatchEvent(node, mouse_down, true);
    }

    node = _scene->pickNode(_mouse_position, scene2d::NODE_FLAG_FOCUSABLE);
    base::object_refptr<scene2d::Node> old_focused = _focused_node.upgrade();
    if (node) {
        if (node != old_focused.get()) {
            if (old_focused) {
                old_focused->state_ &= ~scene2d::NODE_STATE_FOCUSED;
                scene2d::FocusEvent focus_out(old_focused.get(), scene2d::FOCUS_OUT);
                _scene->dispatchEvent(old_focused.get(), focus_out, true);
            }
        }
        _focused_node = node->weaken();
        node->state_ |= scene2d::NODE_STATE_FOCUSED;
        scene2d::FocusEvent focus_in(node, scene2d::FOCUS_IN);
        _scene->dispatchEvent(_focused_node.get(), focus_in, true);
    }
}
void Dialog::OnMouseUp(scene2d::ButtonState button, int buttons, int modifiers) {
    if (buttons == 0 && _mouse_capture) {
        _mouse_capture = false;
        //c2_log("release mouse\n");
        ReleaseCapture();
    }

    if (button != scene2d::LEFT_BUTTON) return;

    base::object_refptr<scene2d::Node> node = _active_node.upgrade();
    if (node) {
        _active_node = nullptr;
        node->state_ &= ~scene2d::NODE_STATE_ACTIVE;
        scene2d::PointF local_pos = _mouse_position - _scene->mapPointToScene(node.get(), scene2d::PointF());
        scene2d::MouseEvent mouse_up(node.get(), scene2d::MOUSE_UP, _mouse_position, local_pos, button, buttons, modifiers);
        _scene->dispatchEvent(node.get(), mouse_up, true);
    }
}
void Dialog::OnMouseMove(int buttons, int modifiers) {
    //c2_log("OnMouseMove %.0f, %.0f\n", _mouse_position.x, _mouse_position.y);
    UpdateMouseTracking();
    UpdateHoveredNode();
    base::object_refptr<scene2d::Node> node = _active_node ? _active_node.upgrade() : _hovered_node.upgrade();
    if (node) {
        scene2d::PointF local_pos = _mouse_position - _scene->mapPointToScene(node.get(), scene2d::PointF());
        scene2d::MouseEvent mouse_move(node.get(), scene2d::MOUSE_MOVE, _mouse_position, local_pos, scene2d::NO_BUTTON, buttons, modifiers);
        _scene->dispatchEvent(node.get(), mouse_move, true);
        RequestPaint();
    }
}
void Dialog::OnMouseWheel(int delta, int buttons, int modifiers, bool hwheel)
{
    scene2d::Node* node;
    scene2d::PointF local_pos;
    node = _scene->pickNode(_mouse_position, scene2d::NODE_FLAG_SCROLLABLE, &local_pos);
    if (node) {
        float wheel_delta = float(delta) / WHEEL_DELTA;
        //LOG(INFO) << "mouse wheel " << local_pos << ", delta=" << wheel_delta << ", hwheel=" << hwheel;
        scene2d::MouseCommand cmd = hwheel ? scene2d::MOUSE_HWHEEL : scene2d::MOUSE_WHEEL;
        scene2d::MouseEvent mouse_wheel(node, cmd, _mouse_position, local_pos, wheel_delta, buttons, modifiers);
        _scene->dispatchEvent(node, mouse_wheel, true);
    }
}
void Dialog::UpdateHoveredNode() {
    scene2d::PointF local_pos;
    scene2d::Node* node = _scene->pickNode(_mouse_position, scene2d::NODE_FLAG_HOVERABLE, &local_pos);
    base::object_refptr<scene2d::Node> old_hovered = _hovered_node.upgrade();
    if (node != old_hovered.get()) {
        if (old_hovered) {
            old_hovered->state_ &= ~scene2d::NODE_STATE_HOVER;
            scene2d::MouseEvent hover_leave(old_hovered.get(), scene2d::MOUSE_OUT, _mouse_position, scene2d::PointF());
            _scene->dispatchEvent(old_hovered.get(), hover_leave, true);
        }
        _hovered_node = node ? node->weaken() : base::object_weakptr<scene2d::Node>();
        if (node) {
            node->state_ |= scene2d::NODE_STATE_HOVER;
            scene2d::MouseEvent hover_enter(node, scene2d::MOUSE_OVER, _mouse_position, local_pos);
            _scene->dispatchEvent(node, hover_enter, true);
        }
        RequestPaint();
    }
}
void Dialog::UpdateFocusedNode() {
    // Focused node become invisible
    if (auto node = _focused_node.upgrade()) {
        if (!node->visibleInHierarchy()) {
            node->state_ &= ~scene2d::NODE_STATE_FOCUSED;
            scene2d::FocusEvent focus_out(node.get(), scene2d::FOCUS_OUT);
            _scene->dispatchEvent(node.get(), focus_out, true);
            _focused_node = nullptr;
        }
    }
}
void Dialog::RecreateRenderTarget() {
    _rt = graphics::GraphicDevice::instance()->CreateHwndRenderTarget(
        _hwnd, _size, _dpi_scale);
    LOG(INFO) << "Dialog::RecreateRenderTarget() hwnd=" << std::hex << _hwnd
        << " size=(" << _size.width << "x" << _size.height << ")"
        << " dpi_scale=" << _dpi_scale
        << " rt=" << std::hex << _rt.Get();
}
void Dialog::OnDpiChanged(UINT dpi, const RECT* rect) {
    LOG(INFO) << "Dialog dpi changed to " << dpi
        << "size " << rect->right - rect->left << "x" << rect->bottom - rect->top << "px";
    float new_dpi_scale = (float)dpi / USER_DEFAULT_SCREEN_DPI;
    if (new_dpi_scale == _dpi_scale)
        return;

    _dpi_scale = new_dpi_scale;
    if (_popup_shadow) {
        _popup_shadow->_render_dpi_scale = _dpi_scale;
        float padding_px = roundf(_popup_shadow_data->padding * _dpi_scale);
        float double_padding_px = roundf(2.0f * _popup_shadow_data->padding * _dpi_scale);
        SetWindowPos(_popup_shadow->GetHwnd(), _hwnd,
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
    SetWindowPos(_hwnd, NULL,
        rect->left,
        rect->top,
        rect->right - rect->left,
        rect->bottom - rect->top,
        SWP_NOZORDER | SWP_NOACTIVATE);

    DiscardDeviceResources();
    UpdateBorderAndRenderTarget();
    RequestUpdate();
}
void Dialog::OnCloseSysCommand(EventContext& ctx) {
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
void Dialog::UpdateMouseTracking() {
    if (!_mouse_event_tracking) {
        _mouse_event_tracking = true;
        TRACKMOUSEEVENT tme = {};
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = _hwnd;
        tme.dwFlags = TME_LEAVE;
        TrackMouseEvent(&tme);
    }
}
void Dialog::OnDestroy() {
    ReleaseHook();
    ClosePopups();
    _scene = nullptr;
    // _title_label = nullptr;
    // _close_button = nullptr;
    _rt = nullptr;
    if ((_flags & DIALOG_FLAG_MAIN) && g_dialog_map.size() == 1)
        PostQuitMessage(0);
    if (_hwnd_parent && _visible)
        EnableWindow(_hwnd_parent, true);
}

void Dialog::OnActivate(bool active) {
    //c2_log("%p OnActive %d\n", _hwnd, active);
    if (active && _popup_shadow) {
        SetWindowPos(_popup_shadow->GetHwnd(),
            _hwnd,
            0, 0,
            0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOCOPYBITS);
    }
    if (!active) {
        ClosePopups();
    }
}
void Dialog::OnWindowPosChanged(WINDOWPOS* wnd_pos) {
    if (_on_window_pos_changed)
        return;
    if (!_popup_shadow)
        return;
    _on_window_pos_changed = true;
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
        float padding_px = roundf(_popup_shadow_data->padding * _dpi_scale);
        float double_padding_px = roundf(2.0f * _popup_shadow_data->padding * _dpi_scale);
        //c2_log("BeginDeferWindowPos flags=%08x padding_px=%.0f\n", wnd_pos->flags, padding_px);
        //c2_log("  popup DeferWindowPos cx=%d, cy=%d\n",
        //       wnd_pos->cx + (int)double_padding_px,
        //       wnd_pos->cy + (int)double_padding_px);
        hdwp = DeferWindowPos(hdwp, _popup_shadow->GetHwnd(), _hwnd,
            wnd_pos->x - (int)padding_px,
            wnd_pos->y - (int)padding_px,
            wnd_pos->cx + (int)double_padding_px,
            wnd_pos->cy + (int)double_padding_px,
            flags);
        success = EndDeferWindowPos(hdwp);
        //c2_log("EndDeferWindowPos success=%d\n", success);
    }
    _on_window_pos_changed = false;
}
void Dialog::DiscardDeviceResources() {
    // DiscardNodeDeviceResources(_root.get());
}
void Dialog::OnEnterKeyDown(EventContext& ctx)
{
    kwui::ScriptEngine::get()->postEvent("dialog:enter-key-down", id_);
}
void Dialog::OnEnterKeyUp(EventContext& ctx)
{
    kwui::ScriptEngine::get()->postEvent("dialog:enter-key-up", id_);
}
void Dialog::OnEscapeKeyDown(EventContext& ctx)
{
    kwui::ScriptEngine::get()->postEvent("dialog:escape-key-down", id_);
}
void Dialog::OnEscapeKeyUp(EventContext& ctx)
{
    kwui::ScriptEngine::get()->postEvent("dialog:escape-key-up", id_);
}
void Dialog::OnF5Down(EventContext& ctx)
{
    if (kwui::Application::scriptReloadEnabled()) {
        _scene->reloadScriptModule();
        OnPaint();
    }
}
void Dialog::DiscardNodeDeviceResources(scene2d::Node* node) {
    // node->DiscardDeviceResources();
    // for (auto& child : node->GetChildren())
    //     DiscardNodeDeviceResources(child.get());
}

void Dialog::UpdateBorderAndRenderTarget() {
    if (_popup_shadow_data.has_value()) {
        float border_radius = roundf(theme::DIALOG_BORDER_RADIUS * _dpi_scale);
        HRGN rgn = CreateRoundRectRgn(
            0, 0,
            (int)_pixel_size.width + 1, (int)_pixel_size.height + 1,
            (int)border_radius, (int)border_radius);
        SetWindowRgn(_hwnd, rgn, FALSE);
    }

    if (!_rt) {
        RecreateRenderTarget();
    } else {
        HRESULT hr = _rt->Resize(
            D2D1::SizeU((UINT32)_pixel_size.width, (UINT32)_pixel_size.height));
        //LOG(INFO) << "RT resize " << (UINT32)_pixel_size.width << "x" << (UINT32)_pixel_size.height << "px";
        _rt->SetDpi(_dpi_scale * USER_DEFAULT_SCREEN_DPI,
            _dpi_scale * USER_DEFAULT_SCREEN_DPI);
        if (hr == D2DERR_RECREATE_TARGET)
            RecreateRenderTarget();
    }
}
void Dialog::OnAnimationTimerEvent() {
    absl::Time timestamp = absl::Now();
    auto nodes = move(_animating_nodes);
    _animating_nodes.clear();
    for (auto& link : nodes) {
        auto node = link.upgrade();
        if (node)
            node->onAnimationFrame(timestamp);
    }
    if (_animating_nodes.empty() && _animation_timer_id) {
        KillTimer(_hwnd, _animation_timer_id);
        _animation_timer_id = 0;
    }
}

LRESULT CALLBACK Dialog::hookProc(int code, WPARAM wParam, LPARAM lParam) {
    MSG* msg = (MSG*)lParam;
    if (wParam == PM_REMOVE) {
        if (msg->message == WM_LBUTTONDOWN || msg->message == WM_RBUTTONDOWN
            || msg->message == WM_NCLBUTTONDOWN || msg->message == WM_NCRBUTTONDOWN
            || msg->message == WM_SYSCHAR) {
            Dialog* dlg = nullptr;
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
void Dialog::ClosePopups()
{
    bool found;
    do {
        found = false;
        for (auto it = g_dialog_map.begin(); it != g_dialog_map.end(); ++it) {
            if (it->second->_hwnd_anchor == _hwnd) {
                auto dlg = it->second;
                g_dialog_map.erase(it);
                dlg->Close();
                found = true;
                break;
            }
        }
    } while (found);
}
void Dialog::SetupHook()
{
    _hook = ::SetWindowsHookExW(WH_GETMESSAGE, &Dialog::hookProc, NULL, GetCurrentThreadId());
}

void Dialog::ReleaseHook()
{
    if (_hook) {
        ::UnhookWindowsHookEx(_hook);
        _hook = NULL;
    }
}

} // namespace windows