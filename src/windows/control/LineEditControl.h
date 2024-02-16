#pragma once
#include "absl/types/optional.h"
#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "style/StyleColor.h"
#include "windows/graphics/TextLayout.h"
#include <string>
#include <deque>
#include <memory>
#include <functional>

namespace windows {

namespace graphics {
class Painter;
}

namespace control {

class Command;
class TextEditModel {
public:
    struct State {
        absl::optional<int> sel_anchor;
        int sel_active;
        std::wstring text;
        State() : sel_anchor(absl::nullopt), sel_active(0) {}
        inline bool HasSelection() const { return sel_anchor.has_value(); }
    };
    class Command {
    public:
        virtual ~Command() = default;
        virtual void Apply(State& state) = 0;
        virtual void Rollback(State& state) = 0;
        virtual bool Merge(const Command& cmd) { return false; }
    };
    void ResetWithText(const std::wstring& text);
    //void InsertChar(int pos, char32 ch);
    void InsertText(int pos, const std::wstring& text, bool from_ime);
    // n 可以为负数
    void DeleteText(int pos, int n);
    void DeleteSelection(int sel_anchor, int sel_active);
    void ReplaceSelection(int sel_anchor, int sel_active,
                          const std::wstring& text);
    bool Undo();
    bool Redo();
    void ClearUndoHistory();
    inline const State& GetState() const { return _state; }

private:
    void AddCommand(std::unique_ptr<Command>&& cmd, bool try_merge = true);

    State _state;
    std::deque<std::unique_ptr<Command>> _cmds;
    size_t _cmd_top = 0;
};

class LineEditControl : public scene2d::Control {
public:
    static const char* CONTROL_NAME;
    LineEditControl();
    ~LineEditControl();
    base::string_atom name() override;
    void onAttach(scene2d::Node* node) override;
    void onDetach(scene2d::Node* node) override;
    void onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value) override;
    void onSetEventHandler(base::string_atom name, const script::Value& func) override;
    bool hitTest(const scene2d::PointF& pos, int flags) const override;
    void onLayout(scene2d::Node* node, const scene2d::RectF& rect) override;
    void onPaint(graph2d::PainterInterface& p, const scene2d::RectF& rect) override;
    void onFocusEvent(scene2d::Node* node, scene2d::FocusEvent& evt) override;
    void onMouseEvent(scene2d::Node* node, scene2d::MouseEvent& evt) override;
    void onKeyEvent(scene2d::Node* node, scene2d::KeyEvent& evt) override;
    void onImeEvent(scene2d::Node* node, scene2d::ImeEvent& evt) override;
    void onAnimationFrame(scene2d::Node* node, absl::Time timestamp) override;
    void SetColor(const style::Color& c) { _color = c; }
    void SetBackgroundColor(const style::Color& c) { _bg_color = c; }
    void SetBackgroundPadding(float padding) { _padding = padding; }
    void SetBorderRadius(float radius) { _border_radius = radius; }
    inline void SetSelectionColor(const style::Color& bg_color, const style::Color& text_color) {
        _selection_bg_color = bg_color;
        _selection_text_color = text_color;
    }
    inline void SetCaretColor(const style::Color& color) { _caret_color = color; }
    std::string GetText() const;
    void SetText(const std::string& text);
    void SetFontSize(float font_size);
    using TextChangedCallback = std::function<void(const std::wstring&)>;
    inline void SetTextChangedCallback(TextChangedCallback&& callback) {
        _text_changed_callback = callback;
    }

private:
    void OnFocusIn(scene2d::Node* node, scene2d::FocusEvent& evt);
    void OnFocusOut(scene2d::Node* node, scene2d::FocusEvent& evt);
    void OnCharacter(std::wstring ch);
    void OnImeStartComposition(scene2d::Node* node, scene2d::ImeEvent& evt);
    void OnImeComposition(const std::wstring& text,
                          absl::optional<int> caret_pos);
    void OnImeEndComposition();
    void OnImeCommit(const std::wstring& text);
    bool QueryImeCaretRect(scene2d::PointF& origin, scene2d::DimensionF& size);

    void OnKeyDown(scene2d::Node* node, int key, int modifiers);
    void OnKeyUp(scene2d::Node* node, int key, int modifiers) {}
    void OnMouseDown(scene2d::Node* node, const scene2d::PointF& local_pos);
    void OnSizeChanged();

    void UpdateTextLayout();
    // 更新文本偏移：插入文字、光标移动、插入输入法Composing文本
    void UpdateCaretAndScroll();
    void InsertText(const std::wstring& text, bool ime_commit);
    void DeleteLeftChar();
    void DeleteRightChar();
    // TODO: 重构：将 MoveCaret() 拆开为MoveCaretTo(make_selection) 和 DoMoveCaret(pos, force, selection)
    void MoveCaret(int pos, bool force, bool keep_selection);
    void MoveCaretLeft(bool make_selection);
    void MoveCaretRight(bool make_selection);
    void MoveCaretToStart(bool make_selection);
    void MoveCaretToEnd(bool make_selection);
    std::wstring GetDisplayText() const;
    int GetDisplayCaretPos() const;
    void ClearComposingText();
    void SyncStateFromModel();
    void Undo();
    void Redo();
    void SelectAll();
    void ClipboardCopy();
    void ClipboardCut();
    void ClipboardPaste();
    inline bool HasSelection() const { return _sel_anchor.has_value(); }
    std::wstring GetSelectionText() const;
    void ResetCaretBlink();

    scene2d::Node* _node = nullptr;
    scene2d::RectF _rect;
    std::wstring _text;
    float _font_size;
    std::unique_ptr<windows::graphics::TextLayout> _layout;
    style::Color _color;
    style::Color _bg_color;
    style::Color _selection_bg_color;
    style::Color _selection_text_color;
    style::Color _caret_color;
    bool _is_focused;
    int _caret_pos;
    scene2d::RectF _caret_rect; // 坐标相对于文字左上角，不考虑scroll_offset
    scene2d::PointF _scroll_offset;
    float _padding;
    float _border_radius;
    // 输入法的临时文本：如拼音
    struct ComposeState {
        std::wstring text;
        int caret_pos = 0;	// 内部光标位置
    };
    absl::optional<ComposeState> _composing;
    absl::optional<int> _sel_anchor;	// 选择区域的另一端位置
    TextEditModel _model;
    class CaretBlinkHelper;
    std::unique_ptr<CaretBlinkHelper> _caret_blink_helper;
    TextChangedCallback _text_changed_callback;
    script::Value onchange_func_;
};

}
}
