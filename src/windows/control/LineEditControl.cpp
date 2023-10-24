#include "LineEditControl.h"
#include "windows/graphics/Painter.h"
#include "base/log.h"
#include "windows/EncodingManager.h"
#include "windows/encoding_helpers.h"
#include "absl/time/clock.h"

namespace windows {
namespace control {

static constexpr size_t MAX_UNDO_HISTORY = 20;
const char* LineEditControl::CONTROL_NAME = "line_edit";

class ReplaceSelectionCommand : public TextEditModel::Command {
public:
    ReplaceSelectionCommand(int caret_pos, const std::wstring& text)
        : _sel_active(caret_pos), _text(text) {}
    ReplaceSelectionCommand(int caret_anchor, int caret_pos, const std::wstring& text)
        : _sel_anchor(caret_anchor), _sel_active(caret_pos), _text(text) {}
    inline bool HasSelection() const {
        return _sel_anchor.has_value();
    }
    void Apply(TextEditModel::State& state) override {
        state.sel_anchor = absl::nullopt;
        if (_sel_anchor.has_value()) {
            int sel_start = _sel_anchor.value();
            int sel_end = _sel_active;
            if (sel_start > sel_end)
                std::swap(sel_start, sel_end);
            _deleted_text = state.text.substr(sel_start, sel_end - sel_start);
            state.text.erase(sel_start, sel_end - sel_start);
            state.sel_active = sel_start + (int)_text.length();
            state.text.insert(sel_start, _text);
        } else {
            state.sel_active = _sel_active + (int)_text.length();
            state.text.insert(_sel_active, _text);
        }
    }
    void Rollback(TextEditModel::State& state) override {
        state.sel_anchor = _sel_anchor;
        state.sel_active = _sel_active;
        if (_sel_anchor.has_value()) {
            int sel_start = _sel_anchor.value();
            int sel_end = _sel_active;
            if (sel_start > sel_end)
                std::swap(sel_start, sel_end);
            state.text.erase(sel_start, _text.length());
            state.text.insert(sel_start, _deleted_text);
        } else {
            state.text.erase(_sel_active, _text.length());
        }
    }
    bool Merge(const Command& cmd_) {
        if (HasSelection())
            return false;
        auto cmd = dynamic_cast<const ReplaceSelectionCommand*>(&cmd_);
        if (cmd) {
            if (cmd->HasSelection())
                return false;
            if (_sel_active + _text.length() == cmd->_sel_active) {
                _text += cmd->_text;
                return true;
            }
        }
        return false;
    }
private:
    absl::optional<int> _sel_anchor;
    int _sel_active;
    std::wstring _text;
    std::wstring _deleted_text; // HasSelection() 时才有效
};
class DeleteTextCommand : public TextEditModel::Command {
public:
    // 向前删除时，n 为负数
    DeleteTextCommand(int caret_pos, int n)
        : _sel_active(caret_pos), _n(n) {}
    void Apply(TextEditModel::State& state) override {
        if (_n >= 0) {
            _deleted_text = state.text.substr(_sel_active, _n);
            DCHECK(_deleted_text.length() == _n);
            state.sel_anchor = absl::nullopt;
            state.sel_active = _sel_active;
            state.text.erase(_sel_active, _n);
        } else {
            _deleted_text = state.text.substr(_sel_active + _n, -_n);
            DCHECK(_deleted_text.length() == -_n);
            state.sel_anchor = absl::nullopt;
            state.sel_active = _sel_active + _n; // 需要移动光标
            state.text.erase(_sel_active + _n, -_n);
        }
    }
    void Rollback(TextEditModel::State& state) override {
        if (_n >= 0) {
            state.sel_anchor = absl::nullopt;
            state.sel_active = _sel_active;
            state.text.insert(_sel_active, _deleted_text);
        } else {
            state.sel_anchor = absl::nullopt;
            state.sel_active = _sel_active;
            state.text.insert(_sel_active + _n, _deleted_text);
        }
        _deleted_text.clear();
    }
    bool Merge(const Command& cmd_) {
        auto cmd = dynamic_cast<const DeleteTextCommand*>(&cmd_);
        if (cmd && _n * cmd->_n > 0) {
            if (_n >= 0 && _sel_active == cmd->_sel_active) {
                // Delete
                _n += cmd->_n;
                _deleted_text += cmd->_deleted_text;
                return true;
            } else if (_n < 0 && _sel_active + _n == cmd->_sel_active) {
                // Backspace
                _n += cmd->_n;
                _deleted_text = cmd->_deleted_text + _deleted_text;
                return true;
            }
        }
        return false;
    }
private:
    int _sel_active;
    int _n;
    std::wstring _deleted_text;
};

class LineEditControl::CaretBlinkHelper {
public:
    CaretBlinkHelper() {
        _blink_time_ms = GetCaretBlinkTime();
        Reset();
    }
    void Reset() {
        _reset_time = absl::Now();
        _visible = absl::nullopt;
    }
    // true is state changed
    bool Update(absl::Time timestamp) {
        if (_blink_time_ms == INFINITY)
            return false;
        absl::Duration rem;
        int64_t i = absl::IDivDuration(timestamp - _reset_time, absl::Milliseconds(_blink_time_ms), &rem);
        bool new_visible = (i % 2 == 0);
        if (_visible.has_value() && _visible.value() == new_visible)
            return false;
        _visible.emplace(new_visible);
        return true;
    }
    bool IsCaretVisible() const {
        return _visible.value_or(false);
    }

private:
    // The time required to invert the caret's pixels.
    // INFINITE indicates that the caret does not blink.
    UINT _blink_time_ms;
    absl::Time _reset_time;
    absl::optional<bool> _visible;
};
/*
static std::wstring char32_to_wstring(std::char32_t ch) {
    static_assert(sizeof(wchar_t) == sizeof(char16));

    char16 c16[3] = {};
    int len = put_unicode(c16, ch);
    return std::wstring((wchar_t*)c16);
}
*/
void TextEditModel::ResetWithText(const std::wstring& text) {
    _cmds.clear();
    _cmd_top = 0;
    _state.sel_anchor = absl::nullopt;
    _state.sel_active = text.length();
    _state.text = text;
}
/*
void TextEditModel::InsertChar(int pos, char32 ch) {
    AddCommand(std::make_unique<ReplaceSelectionCommand>(
        pos, char32_to_wstring(ch)));
}
*/
void TextEditModel::InsertText(int pos, const std::wstring& text, bool from_ime) {
    AddCommand(std::make_unique<ReplaceSelectionCommand>(
        pos, text), !from_ime);
}
void TextEditModel::DeleteText(int pos, int n) {
    AddCommand(std::make_unique<DeleteTextCommand>(
        pos, n));
}
void TextEditModel::DeleteSelection(int sel_anchor, int sel_active) {
    AddCommand(std::make_unique<ReplaceSelectionCommand>(
        sel_anchor, sel_active, std::wstring()));
}
void TextEditModel::ReplaceSelection(int sel_anchor,
    int sel_active,
    const std::wstring& text) {
    AddCommand(std::make_unique<ReplaceSelectionCommand>(
        sel_anchor, sel_active, text));
}
bool TextEditModel::Undo() {
    if (_cmds.empty())
        return false;
    if (_cmd_top == 0)
        return false;
    if (_cmd_top > _cmds.size()) {
        LOG(INFO) << "TextEditModel::Undo(): Invalid cmd top " << _cmd_top;
        ClearUndoHistory();
        return false;
    }
    --_cmd_top;
    _cmds[_cmd_top]->Rollback(_state);
    return true;
}
bool TextEditModel::Redo() {
    if (_cmds.empty())
        return false;
    if (_cmd_top == _cmds.size())
        return false;
    if (_cmd_top > _cmds.size()) {
        LOG(INFO) << "TextEditModel::Redo(): Invalid cmd top " << _cmd_top;
        ClearUndoHistory();
        return false;
    }
    _cmds[_cmd_top]->Apply(_state);
    ++_cmd_top;
    return true;
}
void TextEditModel::ClearUndoHistory() {
    _cmds.clear();
    _cmd_top = 0;
}
void TextEditModel::AddCommand(std::unique_ptr<TextEditModel::Command>&& cmd, bool try_merge) {
    cmd->Apply(_state);
    if (try_merge && _cmd_top > 0 && _cmd_top <= (int)_cmds.size()) {
        if (_cmds[_cmd_top - 1]->Merge(*cmd)) {
            _cmds.resize(_cmd_top);
            return;
        }
    }
    _cmds.resize(_cmd_top);
    _cmds.push_back(move(cmd));
    ++_cmd_top;
    if (_cmds.size() > MAX_UNDO_HISTORY) {
        _cmds.pop_front();
        --_cmd_top;
    }
}
LineEditControl::LineEditControl()
    : _font_size(16/*DEFAULT_FONT_SIZE*/)
    , _color(BLACK), _bg_color(WHITE)
    , _selection_bg_color(BLUE), _selection_text_color(WHITE)
    , _caret_color(BLACK), _is_focused(false), _caret_pos(-1)
    , _scroll_offset(0, 0)
    , _padding(0)
    , _border_radius(0)
    , _sel_anchor(0) {
    // _flags = NODE_FLAG_CLICKABLE | NODE_FLAG_FOCUSABLE;
    _caret_blink_helper = std::make_unique<CaretBlinkHelper>();
    UpdateTextLayout();
    MoveCaret(0, false, false);
}
LineEditControl::~LineEditControl() = default;
base::string_atom LineEditControl::name()
{
    return base::string_intern(CONTROL_NAME);
}
void LineEditControl::onAttach(scene2d::Node* node)
{
    _node = node;
}
void LineEditControl::onDetach(scene2d::Node* node)
{
    _node = nullptr;
}
bool LineEditControl::testFlags(int flags) const
{
    return true;
}
void LineEditControl::onPaint(windows::graphics::Painter &p, const scene2d::RectF& rect) {
    p.Save();
    p.Translate(rect.origin());

    scene2d::DimensionF node_size = rect.size();
    p.SetColor(_bg_color);
    p.DrawRoundedRect(scene2d::PointF(), node_size, _border_radius);

    scene2d::PointF clip_origin(_padding, 0);
    scene2d::DimensionF clip_size(node_size.width - 2 * _padding, node_size.height);
    p.PushClipRect(clip_origin, clip_size);
    {
        // Draw selection
        if (HasSelection() && !_composing.has_value()) {
            p.SetColor(_selection_bg_color);
            scene2d::RectF rect = _layout->rangeRect(_sel_anchor.value(),
                _caret_pos);
            p.DrawRect(_scroll_offset.x + _padding + rect.left,
                _scroll_offset.y + _padding + rect.top,
                rect.width(),
                rect.height());
        }

        // Draw text
        p.SetColor(_color);
        p.DrawTextLayout(_scroll_offset + _padding, *_layout);
    }
    p.PopClipRect();

    if (_composing.has_value()) {
        // Draw underline
        float baseline = _layout->baseline();
        auto pos = HasSelection()
            ? std::min(_sel_anchor.value(), _caret_pos)
            : _caret_pos;
        scene2d::RectF rect = _layout->rangeRect(pos,
            pos + _composing->text.length());
        p.SetColor(_caret_color);
        p.DrawRect(_scroll_offset.x + _padding + rect.left,
            _scroll_offset.y + _padding + baseline - 0.5f,
            rect.width(),
            1.0f);
    }
    if (_is_focused && _caret_blink_helper->IsCaretVisible()) {
        p.SetColor(_caret_color);
        p.DrawRect(_scroll_offset + _padding + _caret_rect.origin(), _caret_rect.size());
    }

    p.Restore();
}
void LineEditControl::onFocusEvent(scene2d::Node* node, scene2d::FocusEvent& evt)
{
    if (evt.cmd == scene2d::FOCUS_IN)
        OnFocusIn(node, evt);
    else if (evt.cmd == scene2d::FOCUS_OUT)
        OnFocusOut(node, evt);
}
void LineEditControl::onMouseEvent(scene2d::Node* node, scene2d::MouseEvent& evt)
{
    if (evt.cmd == scene2d::MOUSE_DOWN)
        OnMouseDown(node, evt.pos);
}
void LineEditControl::onKeyEvent(scene2d::Node* node, scene2d::KeyEvent& evt)
{
    if (evt.cmd == scene2d::KEY_DOWN)
        OnKeyDown(node, evt.key, evt.modifiers);
    else if (evt.cmd == scene2d::KEY_UP)
        OnKeyUp(node, evt.key, evt.modifiers);
}
void LineEditControl::onImeEvent(scene2d::Node* node, scene2d::ImeEvent& evt)
{
    if (evt.cmd == scene2d::CHARS)
        OnCharacter(evt.wtext_);
    else if (evt.cmd == scene2d::START_COMPOSE)
        OnImeStartComposition(node, evt);
    else if (evt.cmd == scene2d::COMPOSING)
        OnImeComposition(evt.wtext_, evt.caret_pos_);
    else if (evt.cmd == scene2d::END_COMPOSE)
        OnImeEndComposition();
    else if (evt.cmd == scene2d::COMMIT)
        OnImeCommit(evt.wtext_);
    node->requestPaint();
}
void LineEditControl::onAnimationFrame(scene2d::Node* node, absl::Time timestamp) {
    if (!_is_focused)
        return;
    if (_caret_blink_helper->Update(timestamp))
        node->requestPaint();
    node->requestAnimationFrame(node);
}
void LineEditControl::UpdateTextLayout() {
    std::wstring disp_text = GetDisplayText();
    _layout = windows::graphics::TextLayoutBuilder(disp_text)
        .FontSize(_font_size)
        .Build();
}
void LineEditControl::OnFocusIn(scene2d::Node* node, scene2d::FocusEvent& evt) {
    _is_focused = true;
    ResetCaretBlink();
    node->requestAnimationFrame(node);
    node->requestPaint();
}
void LineEditControl::OnFocusOut(scene2d::Node* node, scene2d::FocusEvent& ctx) {
    _is_focused = false;
    node->requestPaint();
}
void LineEditControl::OnCharacter(std::wstring ch) {
    InsertText(ch, false);
}
void LineEditControl::OnImeStartComposition(scene2d::Node* node, scene2d::ImeEvent& evt)
{
    scene2d::PointF o;
    scene2d::DimensionF s;
    QueryImeCaretRect(o, s);
    evt.caret_rect_ = scene2d::RectF::fromOriginSize(o, s);
}
void LineEditControl::OnImeComposition(const std::wstring& text,
    absl::optional<int> caret_pos) {
    if (_composing.has_value()) {
        _composing->text = text;
        _composing->caret_pos = caret_pos.value_or(_composing->caret_pos);
    } else {
        ComposeState e;
        e.text = text;
        e.caret_pos = caret_pos.value_or((int)text.length());
        _composing.emplace(e);
    }

    UpdateTextLayout();
    ResetCaretBlink();
    UpdateCaretAndScroll();
}
void LineEditControl::OnImeEndComposition() {
    _composing = absl::nullopt;
    UpdateTextLayout();
    ResetCaretBlink();
    UpdateCaretAndScroll();
}
void LineEditControl::OnImeCommit(const std::wstring& text) {
    InsertText(text, true);
}
bool LineEditControl::QueryImeCaretRect(scene2d::PointF& origin, scene2d::DimensionF& size) {
    //UpdateTextLayout();
    //UpdateScrollOffset();
    origin = _scroll_offset + _padding + _caret_rect.origin();
    size = _caret_rect.size();
    return true;
}
void LineEditControl::OnKeyDown(scene2d::Node* node, int key, int modifiers) {
    switch (key) {
    case VK_LEFT:
        if (modifiers) {
            if ((modifiers & scene2d::SHIFT_MODIFIER) == modifiers) {
                MoveCaretLeft(true);
            }
        } else {
            MoveCaretLeft(false);
        }
        break;
    case VK_RIGHT:
        if (modifiers) {
            if ((modifiers & scene2d::SHIFT_MODIFIER) == modifiers) {
                MoveCaretRight(true);
            }
        } else {
            MoveCaretRight(false);
        }
        break;
    case VK_HOME:
        if (modifiers) {
            if ((modifiers & scene2d::SHIFT_MODIFIER) == modifiers) {
                MoveCaretToStart(true);
            }
        } else {
            MoveCaretToStart(false);
        }
        break;
    case VK_END:
        if (modifiers) {
            if ((modifiers & scene2d::SHIFT_MODIFIER) == modifiers) {
                MoveCaretToEnd(true);
            }
        } else {
            MoveCaretToEnd(false);
        }
        break;
    case VK_BACK:
        DeleteLeftChar();
        break;
    case VK_DELETE:
        DeleteRightChar();
        break;
    case 'C':
        ClipboardCopy();
        break;
    case 'X':
        ClipboardCut();
        break;
    case 'V':
        ClipboardPaste();
        break;
    case 'A':
        if (modifiers) {
            if ((modifiers & scene2d::CTRL_MODIFIER) == modifiers) {
                SelectAll();
            }
        }
        break;
    case 'Z':
        if (modifiers) {
            if ((modifiers & scene2d::CTRL_MODIFIER) == modifiers) {
                // Ctrl-Z
                Undo();
            } else if ((modifiers & scene2d::CTRL_MODIFIER) && (modifiers & scene2d::SHIFT_MODIFIER)
                && (modifiers & (scene2d::CTRL_MODIFIER | scene2d::SHIFT_MODIFIER)) == modifiers) {
                // Ctrl-Shift-Z
                Redo();
            }
        }
        break;
    case 'Y':
        if (modifiers) {
            if ((modifiers & scene2d::CTRL_MODIFIER) == modifiers) {
                // Ctrl-Y
                Redo();
            }
        }
        break;
    }
    ResetCaretBlink();
    node->requestPaint();
}
void LineEditControl::InsertText(const std::wstring& text, bool from_ime) {
    ClearComposingText();
    if (HasSelection()) {
        _model.ReplaceSelection(_sel_anchor.value(),
            _caret_pos,
            text);
    } else {
        _model.InsertText(_caret_pos, text, from_ime);
    }
    SyncStateFromModel();
    UpdateTextLayout();
    UpdateCaretAndScroll();
}
void LineEditControl::DeleteLeftChar() {
    if (HasSelection()) {
        _model.DeleteSelection(_sel_anchor.value(), _caret_pos);
        SyncStateFromModel();
    } else {
        if (_caret_pos > 0) {
            int n = 1;
            if (_caret_pos > 1) {
                wchar_t char_back_one = _text[_caret_pos - 1];
                wchar_t char_back_two = _text[_caret_pos - 2];
                if ((is_low_surrogate(char_back_one) && is_high_surrogate(char_back_two))
                    || (char_back_one == '\n' && char_back_two == '\r')) {
                    n = 2;
                }
            }
            _model.DeleteText(_caret_pos, -n);
            SyncStateFromModel();
        }
    }
    UpdateTextLayout();
    UpdateCaretAndScroll();
}
void LineEditControl::DeleteRightChar() {
    if (HasSelection()) {
        _model.DeleteSelection(_sel_anchor.value(), _caret_pos);
        SyncStateFromModel();
    } else {
        if (_caret_pos < (int)_text.length()) {
            int n = 1;
            if (_caret_pos + 1 < (int)_text.length()) {
                wchar_t char_after_one = _text[_caret_pos];
                wchar_t char_after_two = _text[_caret_pos + 1];
                if ((is_high_surrogate(char_after_one) && is_low_surrogate(char_after_two))
                    || (char_after_one == '\r' && char_after_two == '\n')) {
                    n = 2;
                }
            }
            _model.DeleteText(_caret_pos, n);
            SyncStateFromModel();
        }
    }
    UpdateTextLayout();
    UpdateCaretAndScroll();
}
void LineEditControl::MoveCaretLeft(bool make_selection) {
    int n = 1;
    if (_caret_pos > 1) {
        wchar_t char_back_one = _text[_caret_pos - 1];
        wchar_t char_back_two = _text[_caret_pos - 2];
        if ((is_low_surrogate(char_back_one) && is_high_surrogate(char_back_two))
            || (char_back_one == '\n' && char_back_two == '\r')) {
            n = 2;
        }
    }
    if (make_selection && !HasSelection()) {
        if (_caret_pos > 0) {
            _sel_anchor.emplace(_caret_pos);
        }
    }
    MoveCaret(_caret_pos - n, false, make_selection);
}
void LineEditControl::MoveCaretRight(bool make_selection) {
    int n = 1;
    if (_caret_pos + 1 < (int)_text.length()) {
        wchar_t char_after_one = _text[_caret_pos];
        wchar_t char_after_two = _text[_caret_pos + 1];
        if ((is_high_surrogate(char_after_one) && is_low_surrogate(char_after_two))
            || (char_after_one == '\r' && char_after_two == '\n')) {
            n = 2;
        }
    }
    if (make_selection && !HasSelection()) {
        if (_caret_pos < (int)_text.length()) {
            _sel_anchor.emplace(_caret_pos);
        }
    }
    MoveCaret(_caret_pos + n, false, make_selection);
}
void LineEditControl::MoveCaret(int pos, bool force, bool keep_selection) {
    if (!keep_selection)
        _sel_anchor = absl::nullopt;

    pos = std::clamp(pos, 0, (int)_text.length());
    if (pos != _caret_pos || force) {
        _caret_pos = pos;
        UpdateCaretAndScroll();
    }
}
void LineEditControl::MoveCaretToStart(bool make_selection) {
    if (make_selection && !HasSelection()) {
        if (_caret_pos != 0)
            _sel_anchor.emplace(_caret_pos);
    }
    MoveCaret(0, false, make_selection);
}
void LineEditControl::MoveCaretToEnd(bool make_selection) {
    if (make_selection && !HasSelection()) {
        if (_caret_pos != _text.length())
            _sel_anchor.emplace(_caret_pos);
    }
    MoveCaret((int)_text.length(), false, make_selection);
}
void LineEditControl::UpdateCaretAndScroll() {
    int disp_caret_pos = GetDisplayCaretPos();
    _caret_rect = _layout->caretRect(disp_caret_pos);
    scene2d::RectF content_rect = _layout->rect();
    // TODO: _node为空
    float avail_width = 100;//_node->size().width - _padding * 2;

    float caret_x = _caret_rect.left + 0.5f * _caret_rect.width() + _scroll_offset.x;
    if (caret_x < 0) {
        _scroll_offset.x += -caret_x;
        if (_scroll_offset.x > 0)
            _scroll_offset.x = 0;
    } else {
        if (caret_x > avail_width) {
            _scroll_offset.x -= caret_x - avail_width;
        }
    }

    if (avail_width >= content_rect.width() && _scroll_offset.x < 0) {
        _scroll_offset.x = 0;
        return;
    }
    if (avail_width < content_rect.width() && _scroll_offset.x < avail_width - content_rect.width()) {
        _scroll_offset.x = avail_width - content_rect.width();
        return;
    }
}
void LineEditControl::OnMouseDown(scene2d::Node* node, const scene2d::PointF& local_pos) {
    int pos = _layout->hitTest(local_pos - _scroll_offset - _padding);
    if (pos != -1)
        MoveCaret(pos, false, false);
    ResetCaretBlink();
    node->requestPaint();
}
void LineEditControl::OnSizeChanged() {
    MoveCaret(_caret_pos, true, true);
}
std::string LineEditControl::GetText() const {
    return EncodingManager::WideToUTF8(_text);
}
void LineEditControl::SetText(const std::string& text) {
    std::wstring utf16_text = EncodingManager::UTF8ToWide(text);
    if (utf16_text != _text) {
        _model.ResetWithText(utf16_text);
        SyncStateFromModel();
        UpdateTextLayout();
        MoveCaret(_text.length(), true, true);
    }
}
void LineEditControl::SetFontSize(float font_size) {
    if (_font_size != font_size) {
        _font_size = font_size;
        UpdateTextLayout();
        MoveCaret(_text.length(), true, true);
    }
}
std::wstring LineEditControl::GetDisplayText() const {
    std::wstring display_text = _text;
    if (HasSelection()) {
        if (_composing.has_value()) {
            auto pos = std::min(_sel_anchor.value(), _caret_pos);
            auto sel_len = std::max(_sel_anchor.value(), _caret_pos) - pos;
            if (pos >= 0 && pos <= (int)display_text.length()) {
                display_text.erase(pos, sel_len);
                display_text.insert(pos, _composing->text);
            }
        }
    } else {
        if (_composing.has_value()) {
            if (_caret_pos >= 0 && _caret_pos <= (int)display_text.length())
                display_text.insert(_caret_pos, _composing->text);
        }
    }
    return display_text;
}
int LineEditControl::GetDisplayCaretPos() const {
    if (HasSelection()) {
        if (_composing.has_value()) {
            return std::min(_sel_anchor.value(), _caret_pos)
                + _composing->caret_pos;
        } else {
            return _caret_pos;
        }
    } else {
        return _composing.has_value()
            ? _caret_pos + _composing->caret_pos
            : _caret_pos;
    }
}
void LineEditControl::ClearComposingText() {
    _composing = absl::nullopt;
}
void LineEditControl::SyncStateFromModel() {
    auto& state = _model.GetState();
    bool changed = _text != state.text;
    _text = state.text;
    _sel_anchor = state.sel_anchor;
    _caret_pos = state.sel_active;
    if (changed && _text_changed_callback)
        _text_changed_callback(_text);
}
void LineEditControl::Undo() {
    if (_model.Undo()) {
        SyncStateFromModel();
        UpdateTextLayout();
        UpdateCaretAndScroll();
    }
}
void LineEditControl::Redo() {
    if (_model.Redo()) {
        SyncStateFromModel();
        UpdateTextLayout();
        UpdateCaretAndScroll();
    }
}
void LineEditControl::SelectAll() {
    if (!_text.empty()) {
        _sel_anchor.emplace(0);
        MoveCaret(_text.length(), false, true);
    }
}
void LineEditControl::ClipboardCopy() {
    if (HasSelection()) {
        auto text = EncodingManager::WideToUTF8(GetSelectionText());
        // set_clipboard_text(text);
    }
}
void LineEditControl::ClipboardCut() {
    if (HasSelection()) {
        auto text = EncodingManager::WideToUTF8(GetSelectionText());
        // set_clipboard_text(text);

        _model.DeleteSelection(_sel_anchor.value(), _caret_pos);
        SyncStateFromModel();
        UpdateTextLayout();
        UpdateCaretAndScroll();
    }
}
void LineEditControl::ClipboardPaste() {
#if 0
    absl::optional<std::string> text = get_clipboard_text();
    if (text.has_value()) {
        std::wstring utf16_text = EncodingManager::UTF8ToWide(text.value());
        InsertText(utf16_text, true/*from_ime*/); // from_ime=true 标识可以能单独撤销
    }
#endif
}

std::wstring LineEditControl::GetSelectionText() const {
    if (!_sel_anchor.has_value())
        return std::wstring();
    int sel_start = std::min(_sel_anchor.value(), _caret_pos);
    int sel_end = std::max(_sel_anchor.value(), _caret_pos);
    return _text.substr(sel_start, sel_end - sel_start);
}
void LineEditControl::ResetCaretBlink() {
    _caret_blink_helper->Reset();
}

}
}
