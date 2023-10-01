#pragma once
#include "base/Object.h"
#include "geom_types.h"
#include <string>

namespace scene2d {

enum EventGroups {
    NO_GROUP = 0,
    HANDLE_MOUSE = 1,
    HANDLE_KEY = 2,
    HANDLE_FOCUS = 4,
    HANDLE_IME = 8,
};

class Node;
struct Event {
    base::object_refptr<Node> target;
    int cmd;

    Event(Node* t, int c)
        : target(t), cmd(c) {}
    virtual EventGroups group() const
    {
        return NO_GROUP;
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

    MOUSE_ENTER,
    MOUSE_LEAVE,
};

struct MouseEvent : public Event {
    PointF view_pos;
    PointF pos;
    int buttons;
    int modifiers;

    MouseEvent(Node* t, int c, const PointF& vpos, int btns, int modi)
        : Event(t, c)
        , view_pos(vpos)
        , pos(vpos)
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
    ImeEvent(Node* t, int c, const std::wstring &text)
        : Event(t, c), wtext_(text) {}
    static const EventGroups EVENT_GROUP = HANDLE_IME;
    EventGroups group() const override
    {
        return EVENT_GROUP;
    }

    std::wstring wtext_;
};

}
