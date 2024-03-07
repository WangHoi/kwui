#include "HiddenMsgWindow.h"
#include "base/log.h"

namespace windows {

typedef LRESULT(CALLBACK *WndProc)(HWND, UINT, WPARAM, LPARAM);
static ATOM RegisterWindowClass(HINSTANCE hInstance, const wchar_t* class_name, WndProc wnd_proc);

const UINT HiddenMsgWindow::MESSAGE_TYPE = WM_USER + 77;

HiddenMsgWindow::HiddenMsgWindow()
	: _listener(NULL), _hwnd(NULL)
{
	initWindow(GetModuleHandle(NULL), L"kwui::HiddenMsgWindow");
}

void HiddenMsgWindow::initWindow(HINSTANCE hInstance, const WCHAR* wnd_class_name) {
	RegisterWindowClass(hInstance, wnd_class_name, &HiddenMsgWindow::WndProcMain);
	CreateWindowExW(0, wnd_class_name, L"", 0,
		0, 0,
		0, 0,
		HWND_MESSAGE, NULL, hInstance, this);
}

ATOM RegisterWindowClass(HINSTANCE hInstance, const wchar_t* class_name, WndProc wnd_proc) {
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_DBLCLKS;
	wcex.lpfnWndProc = wnd_proc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = class_name;

	return RegisterClassExW(&wcex);
}
LRESULT HiddenMsgWindow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == HiddenMsgWindow::MESSAGE_TYPE) {
		if (_listener) {
			_listener->onAppMessage(wParam, lParam);
		} else {
			LOG(INFO) << "HiddenMsgWindow::WindowProc(): ignore user message";
		}
		return 0;
	} else {
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
LRESULT CALLBACK HiddenMsgWindow::WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	HiddenMsgWindow *me;
	if (message == WM_NCCREATE) {
		me = reinterpret_cast<HiddenMsgWindow*>(((LPCREATESTRUCTW)lParam)->lpCreateParams);
		me->_hwnd = hWnd;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)me);
	} else {
		me = reinterpret_cast<HiddenMsgWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
	}
	if (me)
		return me->WindowProc(hWnd, message, wParam, lParam);
	else
		return DefWindowProc(hWnd, message, wParam, lParam);
}

}
