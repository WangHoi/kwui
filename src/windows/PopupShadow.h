#pragma once

#include "windows_header.h"
#include "graphics/GraphicDevice.h"
#include "scene2d/geom_types.h"

namespace windows {

struct PopupShadowData {
	std::string image_name;
	int padding; // in DIP
    HMONITOR monitor;
    float dpi_scale;
};

class DialogWin32;
class PopupShadow {
public:
	PopupShadow(float width, float height, HWND owner,
				const WCHAR* wnd_class_name,
				const PopupShadowData& data);
	virtual ~PopupShadow();

	inline void Show() {
		SetVisible(true);
	}
	inline void Hide() {
		SetVisible(false);
	}
	void SetVisible(bool visible);
	inline bool IsVisible() const {
		return _visible;
	}
	HWND GetHwnd() const { return _hwnd; }
	void Close();

	virtual void OnDestroy();

private:
	void InitWindow(HINSTANCE hInstance, const WCHAR* wnd_class_name, HICON icon);
	static LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void OnCreate();
	void OnPaint();
	void OnResize();
	void RecreateRenderTarget();
	void DiscardDeviceResources();

	HWND _owner_hwnd;
	HWND _hwnd;
	bool _visible;
	bool _first_show_window;
	scene2d::DimensionF _pixel_size;
	graphics::WicBitmapRenderTarget _rt;
	PopupShadowData _data;
	ComPtr<ID2D1Bitmap> _bitmap;
	float _render_dpi_scale;
	friend class DialogWin32;
};

}
