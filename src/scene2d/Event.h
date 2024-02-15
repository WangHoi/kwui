#pragma once
#include "base/Object.h"
#include "geom_types.h"
#include "absl/types/optional.h"
#include <string>

namespace scene2d {

enum EventGroups {
    NO_GROUP = 0,
    HANDLE_MOUSE = 1,
    HANDLE_KEY = 2,
    HANDLE_FOCUS = 4,
    HANDLE_IME = 8,
};
enum EventFlag {
    EVENT_FLAG_HANDLED = 1,
    EVENT_FLAG_SINKING = 2,
};

class Node;
struct Event {
    Node* target;
    int cmd;
    int flags;

    Event(Node* t, int c)
        : target(t), cmd(c), flags(0) {}
    virtual EventGroups group() const
    {
        return NO_GROUP;
    }
    inline bool isHandled() const
    {
        return (flags & EVENT_FLAG_HANDLED) != 0;
    }
    inline void setHandled()
    {
        flags |= EVENT_FLAG_HANDLED;
    }
};

enum ModifierState {
    NO_MODIFILER = 0,
    LCTRL_MODIFIER = 1,
    RCTRL_MODIFIER = 2,
    CTRL_MODIFIER = LCTRL_MODIFIER | RCTRL_MODIFIER,
    LSHIFT_MODIFIER = 4,
    RSHIFT_MODIFIER = 8,
    SHIFT_MODIFIER = LSHIFT_MODIFIER | RSHIFT_MODIFIER,
    LALT_MODIFIER = 16,
    RALT_MODIFIER = 32,
    ALT_MODIFIER = LALT_MODIFIER | RALT_MODIFIER,
};

enum ButtonState {
    NO_BUTTON = 0,
    LEFT_BUTTON = 1,
    RIGHT_BUTTON = 2,
    MIDDLE_BUTTON = 4,
};

enum MouseCommand {
    MOUSE_DOWN,
    MOUSE_UP,
    MOUSE_MOVE,

    MOUSE_OVER,
    MOUSE_OUT,

    MOUSE_WHEEL,
    MOUSE_HWHEEL,
};

struct MouseEvent : public Event {
    PointF view_pos;
    PointF pos;
    float wheel_delta;
    ButtonState button = NO_BUTTON;
    int buttons = NO_BUTTON;
    int modifiers = NO_MODIFILER;

    MouseEvent(Node* t, int c)
        : Event(t, c) {}
    MouseEvent(Node* t, int c, const PointF& vpos, const PointF& lpos)
        : Event(t, c)
        , view_pos(vpos)
        , pos(lpos) {}
    MouseEvent(Node* t, int c, const PointF& vpos, const PointF& lpos, ButtonState btn, int btns, int modi)
        : Event(t, c)
        , view_pos(vpos)
        , pos(lpos)
        , button(btn)
        , buttons(btns)
        , modifiers(modi) {}
    MouseEvent(Node* t, int c, const PointF& vpos, const PointF& lpos, float delta, int btns, int modi)
        : Event(t, c)
        , view_pos(vpos)
        , pos(lpos)
        , wheel_delta(delta)
        , buttons(btns)
        , modifiers(modi) {}

    static const EventGroups EVENT_GROUP = HANDLE_MOUSE;
    EventGroups group() const override
    {
        return EVENT_GROUP;
    }
};

enum KeyCommand {
    KEY_DOWN,
    KEY_UP,
};

struct KeyEvent : public Event {
    int key;
    int modifiers;

    KeyEvent(Node* t, int c, int k, int modi)
        : Event(t, c)
        , key(k)
        , modifiers(modi) {}
    static const EventGroups EVENT_GROUP = HANDLE_KEY;
    EventGroups group() const override
    {
        return EVENT_GROUP;
    }
};

enum FocusCommand {
    FOCUS_IN,
    FOCUS_OUT,
};

struct FocusEvent : public Event {
    FocusEvent(Node* t, int c)
        : Event(t, c) {}
    static const EventGroups EVENT_GROUP = HANDLE_FOCUS;
    EventGroups group() const override
    {
        return EVENT_GROUP;
    }
};

enum ImeCommand {
    CHARS,
    START_COMPOSE,
    COMPOSING,
    END_COMPOSE,
    COMMIT,
};

struct ImeEvent : public Event {
    ImeEvent(Node* t, int c)
        : Event(t, c) {}
    ImeEvent(Node* t, int c, const std::wstring& text, absl::optional<int> caret_pos = absl::nullopt)
        : Event(t, c), wtext_(text), caret_pos_(caret_pos) {}
    static const EventGroups EVENT_GROUP = HANDLE_IME;
    EventGroups group() const override
    {
        return EVENT_GROUP;
    }

    std::wstring wtext_;
    absl::optional<int> caret_pos_;
    absl::optional<RectF> caret_rect_;
};

}
