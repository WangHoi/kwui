#pragma once
#include "base/base.h"
#include "scene2d/scene2d.h"
#include "graph2d/PaintSurface.h"
#include "script/Dialog.h"
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

class DialogWin32 : public script::DialogInterface {
public:
    struct CreateData {
        float dpi_scale;
        HMONITOR monitor;
    };
    DialogWin32(const WCHAR* wnd_class_name, HICON icon, int flags,
        absl::optional<PopupShadowData> popup_shadow,
        absl::optional<CreateData> create_data);
    virtual ~DialogWin32();

    static float getMonitorDpiScale(HMONITOR monitor);
    static DialogWin32* findDialogById(const std::string& id);

    inline void SetParent(HWND parent) { hwnd_parent_ = parent; }
    inline void SetParent(DialogWin32* parent) { SetParent(parent ? parent->hwnd_ : nullptr); }
    void SetPopupAnchor(DialogWin32* anchor);
    inline void Show() {
        SetVisible(true);
    }
    inline void Hide() {
        SetVisible(false);
    }
    void SetVisible(bool visible);
    inline bool IsVisible() const {
        return visible_;
    }
    static scene2d::RectF adjustWindowRect(const scene2d::RectF& rect,
        float dpi_scale, bool customFrame, bool popup);
    void Resize(float width, float height);
    
    void setWindowPos(const scene2d::RectF& rect);
    
    void SetTitle(const std::string& text);
    scene2d::Scene* GetScene() const override { return scene_.get(); }
    const scene2d::DimensionF& GetSize() const { return size_; }
    inline float GetDpiScale() const { return dpi_scale_; }
    HWND GetHwnd() const { return hwnd_; }
    void Close();

    // implements EventContext
    std::string eventContextId() const override { return id_; }
    scene2d::PointF GetMousePosition() const override { return mouse_position_; }
    void RequestPaint() override;
    void RequestUpdate() override;
    void RequestAnimationFrame(scene2d::Node* node) override;

    void OnCloseSysCommand(EventContext& ctx);
    virtual void OnDestroy();
    virtual void OnActivate(bool active);
    virtual void OnWindowPosChanged(WINDOWPOS* wnd_pos);
    virtual void DiscardDeviceResources();
    virtual void OnEnterKeyDown(EventContext& ctx);
    virtual void OnEnterKeyUp(EventContext& ctx);
    virtual void OnEscapeKeyDown(EventContext& ctx);
    virtual void OnEscapeKeyUp(EventContext& ctx);
    virtual void OnF5Down(EventContext& ctx);

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
    void OnMouseWheel(int wheel_delta, int buttons, int modifiers, bool hwheel = false);
    void UpdateHoveredNode();
    void UpdateFocusedNode();
    void UpdateMouseTracking();
    void recreateSurface();
    void OnDpiChanged(UINT dpi, const RECT* rect);
    void DiscardNodeDeviceResources(scene2d::Node* node);
    void UpdateBorderAndRenderTarget();
    void OnAnimationTimerEvent();
    void SetupHook();
    void ReleaseHook();
    static LRESULT CALLBACK hookProc(int code, WPARAM wParam, LPARAM lParam);
    void ClosePopups();

    HWND hwnd_parent_;
    HWND hwnd_anchor_;
    HWND hwnd_;
    int flags_;
    absl::optional<CreateData> create_data_;
    bool visible_;
    bool first_show_window_;
    scene2d::DimensionF size_;
    scene2d::DimensionF pixel_size_;
    scene2d::PointF mouse_position_;
    std::unique_ptr<graph2d::PaintSurfaceInterface> surface_;
    std::unique_ptr<scene2d::Scene> scene_;
    base::object_weakptr<scene2d::Node> hovered_node_;
    base::object_weakptr<scene2d::Node> active_node_;
    base::object_weakptr<scene2d::Node> focused_node_;
    bool mouse_event_tracking_;
    bool mouse_capture_;
    absl::optional<PopupShadowData> popup_shadow_data_;
    std::unique_ptr<PopupShadow> popup_shadow_;
    bool on_window_pos_changed_;
    float dpi_scale_;
    absl::optional<scene2d::RectF> screen_rect_;
    HIMC himc_;
    std::vector<base::object_weakptr<scene2d::Node>>  animating_nodes_;
    UINT_PTR animation_timer_id_;
    HHOOK hook_ = NULL;

    std::string id_;
};

} // namespace windows
