#include "PopupShadow.h"
#include "EncodingManager.h"
#include "graphics/GraphicDevice.h"
#include "graphics/Painter.h"
#include "graphics/Color.h"
#include "Dialog.h"
#include "theme.h"
#include <dwmapi.h>

namespace windows {

#pragma comment(lib, "dwmapi.lib")

typedef LRESULT(CALLBACK *WndProc)(HWND, UINT, WPARAM, LPARAM);
static ATOM RegisterWindowClass(HINSTANCE hInstance,
								const wchar_t* class_name,
								HICON icon,
								WndProc wnd_proc);

PopupShadow::PopupShadow(float width, float height,
						 HWND owner,
						 const WCHAR* wnd_class_name,
						 const PopupShadowData& data)
	: _owner_hwnd(owner), _hwnd(NULL)
	, _visible(false), _first_show_window(true)
	, _data(data) {
	_render_dpi_scale = data.dpi_scale;
	_pixel_size = (scene2d::DimensionF(width, height) * _render_dpi_scale).makeRound();
	InitWindow(GetModuleHandle(NULL), wnd_class_name, NULL);
}
PopupShadow::~PopupShadow() {
}
void PopupShadow::SetVisible(bool visible) {
	if (_visible != visible) {
		_visible = visible;
	}
	ShowWindow(_hwnd, _visible ? SW_SHOWNORMAL : SW_HIDE);
	if (_visible && _first_show_window) {
		_first_show_window = false;
		UpdateWindow(_hwnd);
	}
}
void PopupShadow::InitWindow(HINSTANCE hInstance, const WCHAR* wnd_class_name, HICON icon) {
	auto title = EncodingManager::UTF8ToWide("");
	RegisterWindowClass(hInstance, wnd_class_name, icon, &PopupShadow::WndProcMain);
	DWORD style, ex_style;

	style = WS_POPUP
		| WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	ex_style = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR
		| WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT;

	RECT rect;
    int wnd_left = 360;
    int wnd_top = 240;

	RECT work_area;
    if (_data.monitor) {
        MONITORINFO info = {};
        info.cbSize = sizeof(info);
        if (GetMonitorInfoW(_data.monitor, &info)) {
            int monitor_width = (int)info.rcWork.right;
            int monitor_height = (int)info.rcWork.bottom;
            wnd_left = (monitor_width - (int)_pixel_size.width) / 2;
            wnd_top = (monitor_height - (int)_pixel_size.height) / 2;
        }
    } else if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &work_area, 0)) {
		int work_area_width = (int)work_area.right;
		int work_area_height = (int)work_area.bottom;
		wnd_left = (work_area_width - (int)_pixel_size.width) / 2;
		wnd_top = (work_area_height - (int)_pixel_size.height) / 2;
	}

	SetRect(&rect, wnd_left, wnd_top,
			wnd_left + (int)_pixel_size.width,
			wnd_top + (int)_pixel_size.height);
	CreateWindowExW(ex_style, wnd_class_name, title.c_str(), style,
					rect.left, rect.top,
					rect.right - rect.left,
					rect.bottom - rect.top,
					NULL, NULL, hInstance, this);
}

ATOM RegisterWindowClass(HINSTANCE hInstance, const wchar_t* class_name, HICON icon, WndProc wnd_proc) {
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_DBLCLKS;
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
	int mask = 0;
	if (wParam & MK_LBUTTON) mask |= (1 << LEFT_BUTTON);
	if (wParam & MK_MBUTTON) mask |= (1 << MIDDLE_BUTTON);
	if (wParam & MK_RBUTTON) mask |= (1 << RIGHT_BUTTON);
	return mask;
}

static int GetModifiersState() {
	int state = NO_MODIFILER;
	if (GetKeyState(VK_LCONTROL) & 0x8000) state |= LCTRL_MODIFIER;
	if (GetKeyState(VK_RCONTROL) & 0x8000) state |= RCTRL_MODIFIER;
	if (GetKeyState(VK_LSHIFT) & 0x8000) state |= LSHIFT_MODIFIER;
	if (GetKeyState(VK_RSHIFT) & 0x8000) state |= RSHIFT_MODIFIER;
	if (GetKeyState(VK_LMENU) & 0x8000) state |= LALT_MODIFIER;
	if (GetKeyState(VK_RMENU) & 0x8000) state |= RALT_MODIFIER;
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

LRESULT PopupShadow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		OnCreate();
		{
			MARGINS margins{ 0, 0, 1, 0 };
			DwmExtendFrameIntoClientArea(hWnd, &margins);
		}
		break;
	case WM_DESTROY:
		OnDestroy();
		break;
	case WM_SHOWWINDOW:
		if (wParam) {
			OnPaint();
		}
		break;
	case WM_DWMCOMPOSITIONCHANGED:
		//c2_log("PopupShadow recv WM_DISPLAYCHANGE\n");
		DiscardDeviceResources();
		OnPaint();
		break;
	case WM_DPICHANGED:
		DiscardDeviceResources();
		OnPaint();
		return 0;
	case WM_DISPLAYCHANGE:
		//c2_log("PopupShadow recv WM_DISPLAYCHANGE\n");
		DiscardDeviceResources();
		OnPaint();
		break;
	case WM_PAINT:
		//c2_log("PopupShadow recv WM_PAINT\n");
		OnPaint();
		return 0;
	case WM_SIZE:
		_pixel_size = scene2d::DimensionF((float)LOWORD(lParam), (float)HIWORD(lParam));
		OnResize();
		break;
	case WM_NCCALCSIZE: {
		WINDOWPLACEMENT placement;
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
	case WM_NCHITTEST:
		return HTTRANSPARENT;
	case WM_NCACTIVATE:
		return TRUE;
	case WM_ERASEBKGND: // don't want flicker
		return true;
	case WM_ACTIVATE:
        return 0;
		//if (wParam == WA_INACTIVE) {
		//    //ReleaseCapture();
		//} else {
		//	//SetActiveWindow(_hwnd_owner);
		//}
  //      c2_log("PopupShadow got WM_ACTIVATE %d\n", wParam == WA_INACTIVE ? 0 : 1);
		//return DefWindowProcW(hWnd, message, wParam, lParam);
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	return 0;
}
LRESULT CALLBACK PopupShadow::WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PopupShadow *me;
	if (message == WM_CREATE) {
		me = reinterpret_cast<PopupShadow*>(((LPCREATESTRUCTW)lParam)->lpCreateParams);
		me->_hwnd = hWnd;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG)me);
	} else {
		me = reinterpret_cast<PopupShadow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
	}
	return me->WindowProc(hWnd, message, wParam, lParam);
}
void PopupShadow::OnCreate() {
}
void PopupShadow::OnPaint() {
	if (!_rt.target)
		return;
	HRESULT hr;
	PAINTSTRUCT ps;
	BeginPaint(_hwnd, &ps);
	_rt.target->BeginDraw();
	
	graphics::Painter p(_rt.target.Get(), scene2d::PointF::fromAll(-1));
	p.Clear(NO_COLOR);
	if (!_bitmap) {
		graphics::BitmapSubItem item = graphics::GraphicDevice::get()
			->GetBitmap(_data.image_name, _render_dpi_scale);
		if (item)
			_bitmap = p.CreateBitmap(item);
	}
	if (_bitmap) {
		float src_margin = theme::DIALOG_SHADOW_MARGIN_PIXELS * 2;
		float dst_margin = _render_dpi_scale * src_margin;
		p.DrawScale9Bitmap(_bitmap.Get(), 0, 0, _pixel_size.width, _pixel_size.height,
						   src_margin, dst_margin);
	}
	HDC hdc = NULL;
	RECT rect = {};
	hr = _rt.interop_target->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hdc);

	UPDATELAYEREDWINDOWINFO info = {};
	POINT pt = {};
	SIZE size = { (LONG)_pixel_size.width, (LONG)_pixel_size.height };
	RECT wrect;
	GetWindowRect(_hwnd, &wrect);
	info.cbSize = sizeof(UPDATELAYEREDWINDOWINFO);
	info.psize = &size;
	info.pptSrc = &pt;
	//info.pptDst = &pt;
	BLENDFUNCTION m_blend = {};
	m_blend.BlendOp = AC_SRC_OVER;
	m_blend.SourceConstantAlpha = 255;
	m_blend.AlphaFormat = AC_SRC_ALPHA;
	_rt.interop_target->ReleaseDC(&rect);
	info.pblend = &m_blend;
	info.dwFlags = ULW_ALPHA;// | ULW_EX_NORESIZE;
	info.hdcSrc = hdc;
	hr = UpdateLayeredWindowIndirect(_hwnd, &info);
	//c2_log("UpdateLayeredWindow size=(%d,%d) res=%d\n",
	//	   size.cx, size.cy,
	//	   SUCCEEDED(hr));

	hr = _rt.target->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET) {
		DiscardDeviceResources();
		RecreateRenderTarget();
	}
	EndPaint(_hwnd, &ps);
}
void PopupShadow::OnResize() {
	//c2_log("PopupShadow OnResize %.0fx%.0f px\n", _pixel_size.width, _pixel_size.height);
	_bitmap = nullptr;
	RecreateRenderTarget();
	OnPaint();
}
void PopupShadow::Close() {
	SendMessageW(_hwnd, WM_CLOSE, 0, 0);
}
void PopupShadow::RecreateRenderTarget() {
	_rt = graphics::GraphicDevice::get()
		->CreateWicBitmapRenderTarget(_pixel_size.width, _pixel_size.height, 1.0f);
}
void PopupShadow::OnDestroy() {
	_bitmap = nullptr;
	_rt = graphics::WicBitmapRenderTarget();// nullptr;
}
void PopupShadow::DiscardDeviceResources() {
	_bitmap = nullptr;
}

}
