#pragma once
#include "base/base.h"
#include "scene2d/scene2d.h"
#include "windows_header.h"
#include "absl/types/optional.h"
#include "PopupShadow.h"
#include <vector>

namespace windows {
class WindowMsgListener
{
public:
	virtual void onAppMessage(WPARAM wParam, LPARAM lParam) = 0;
};
class HiddenMsgWindow
{
public:
	HiddenMsgWindow();
	~HiddenMsgWindow() = default;

	inline HWND getHwnd() const { return _hwnd; }
	inline void setListener(WindowMsgListener *l) { _listener = l; }
	static const UINT MESSAGE_TYPE;

private:
	void initWindow(HINSTANCE hInstance, const WCHAR* wnd_class_name);
	static LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	WindowMsgListener *_listener;
	HWND _hwnd;
};
}
