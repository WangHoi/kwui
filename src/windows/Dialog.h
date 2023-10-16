#pragma once
#include "base/base.h"
#include "scene2d/scene2d.h"
#include "windows_header.h"
#include "absl/types/optional.h"
#include "PopupShadow.h"
#include <vector>

namespace scene2d {
class Scene;
class Node;
}

namespace windows {

namespace graphics {
class Painter;
}

enum CursorType {
    CURSOR_ARROW,
    CURSOR_CROSS,
    CURSOR_HAND,
    CURSOR_IBEAM,
    CURSOR_WAIT,
    NUM_CURSOR_TYPES,
};

enum DialogFlag {
    DIALOG_FLAG_MAIN = 1,
    DIALOG_FLAG_POPUP = 2,
};

class Dialog : public scene2d::EventContext {
public:
    struct CreateData {
        float dpi_scale;
        HMONITOR monitor;
    };
    Dialog(float width, float height,
        const WCHAR* wnd_class_name, HICON icon, int flags,
        absl::optional<PopupShadowData> popup_shadow,
        absl::optional<CreateData> create_data);
    virtual ~Dialog();

    inline void SetParent(HWND parent) { _hwnd_parent = parent; }
    inline void SetParent(Dialog* parent) { SetParent(parent->_hwnd); }
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
    void Resize(float width, float height);
    void SetTitle(const std::string& text);
    scene2d::Scene* GetScene() const { return _scene.get(); }
    const scene2d::DimensionF& GetSize() const { return _size; }
    inline float GetDpiScale() const { return _dpi_scale; }
    HWND GetHwnd() const { return _hwnd; }
    void Close();

    // implements EventContext
    scene2d::PointF GetMousePosition() const override { return _mouse_position; }
    void RequestPaint() override;
    void RequestUpdate() override;
    void RequestAnimationFrame(scene2d::Node* node) override;

    virtual void OnCloseButtonClicked(EventContext& ctx);
    virtual void OnDestroy();
    virtual void OnActivate(bool active);
    virtual void OnWindowPosChanged(WINDOWPOS* wnd_pos);
    virtual void DiscardDeviceResources();
    virtual void OnEnterKeyDown(EventContext& ctx) {}
    virtual void OnEnterKeyUp(EventContext& ctx) {}
    virtual void OnEscapeKeyDown(EventContext& ctx) {}
    virtual void OnEscapeKeyUp(EventContext& ctx) {}

private:
    void InitWindow(HINSTANCE hInstance, const WCHAR* wnd_class_name, HICON icon);
    static LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void OnCreate();
    void OnPaint();
    void OnResize();
    void OnKeyDown(int key, int modifiers, bool prev_down = true);
    void OnKeyUp(int key, int modifiers);
    void OnCharacter(wchar_t ch);
    void OnImeCommit(const std::wstring& text);
    void OnImeStartComposition();
    void OnImeComposition(const std::wstring& text, absl::optional<int> caret_pos);
    void OnImeEndComposition();
    void UpdateCaretRect(const scene2d::PointF& origin, const scene2d::DimensionF& size);
    void OnMouseDown(scene2d::ButtonState button, int buttons, int modifiers);
    void OnMouseUp(scene2d::ButtonState button, int buttons, int modifiers);
    void OnMouseMove(int buttons, int modifiers);
    void UpdateHoveredNode();
    void UpdateFocusedNode();
    void UpdateMouseTracking();
    void RecreateRenderTarget();
    void OnDpiChanged(UINT dpi, const RECT* rect);
    void DiscardNodeDeviceResources(scene2d::Node* node);
    void UpdateBorderAndRenderTarget();
    void OnAnimationTimerEvent();

    HWND _hwnd_parent;
    HWND _hwnd;
    int _flags;
    absl::optional<CreateData> _create_data;
    bool _visible;
    bool _first_show_window;
    scene2d::DimensionF _size;
    scene2d::DimensionF _pixel_size;
    scene2d::PointF _mouse_position;
    ComPtr<ID2D1HwndRenderTarget> _rt;
    std::unique_ptr<scene2d::Scene> _scene;
    // Node2DRef _root;
    // LabelNodeRef _title_label;
    // ImageButtonNodeRef _close_button;
    base::object_weakptr<scene2d::Node> _hovered_node;   // under mouse node
    base::object_weakptr<scene2d::Node> _focused_node;
    bool _mouse_event_tracking;
    bool _mouse_capture;
    absl::optional<PopupShadowData> _popup_shadow_data;
    std::unique_ptr<PopupShadow> _popup_shadow;
    bool _on_window_pos_changed;
    float _dpi_scale;
    HIMC _himc;
    std::vector<base::object_weakptr<scene2d::Node>>  _animating_nodes;
    UINT_PTR _animation_timer_id;
};

} // namespace windows
