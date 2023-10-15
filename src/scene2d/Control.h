#pragma once
#include "base/string_intern.h"
#include "Event.h"
#include "Node.h"
#include <map>

namespace windows {
namespace graphics {
class Painter;
}
}

namespace scene2d {

class Node;
class Control {
public:
    virtual base::string_atom name() = 0;
    virtual int eventGroups() const { return NO_GROUP; }
    virtual void onAttach(Node *node) {}
    virtual void onDetach(Node *node) {}
    virtual bool testFlags(int flags) const { return false; }
    virtual void onPaint(windows::graphics::Painter &p, const scene2d::RectF& rect) {}
    virtual void onMouseEvent(MouseEvent &evt) {}
    virtual void onKeyEvent(KeyEvent &evt) {}
    virtual void onFocusEvent(FocusEvent &evt) {}
    virtual void onImeEvent(ImeEvent &evt) {}
    virtual void onSetAttribute(base::string_atom name, const NodeAttributeValue &value) {}
    virtual void onSetEventHandler(base::string_atom name, JSValue func) {}
};

typedef Control* (*ControlFactoryFn)();

class ControlRegistry {
public:
    static ControlRegistry* get();
    void registerControl(base::string_atom name, ControlFactoryFn fn);
    void unregisterControl(base::string_atom name);
    Control* createControl(base::string_atom name);

private:
    std::map<base::string_atom, ControlFactoryFn> control_factories_;
};

}