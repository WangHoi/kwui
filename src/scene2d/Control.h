#pragma once
#include "base/string_intern.h"
#include "Event.h"
#include "Node.h"
#include <map>

namespace graph2d {
class PainterInterface;
}

namespace scene2d {

class EventContext {
public:
    virtual std::string eventContextId() const = 0;
    virtual scene2d::PointF GetMousePosition() const = 0;

    virtual void RequestPaint() = 0;
    virtual void RequestUpdate() = 0;
    virtual void RequestAnimationFrame(scene2d::Node* node) = 0;
};

class Node;
class Control {
public:
    virtual ~Control() {}
    virtual base::string_atom name() = 0;
    virtual int eventGroups() const { return NO_GROUP; }
    virtual void onAttach(Node *node) {}
    virtual void onDetach(Node *node) {}
    virtual bool hitTest(const PointF& pos, int flags) const { return false; }
    virtual void onLayout(Node* node, const scene2d::RectF& rect) {}
    virtual void onPaint(graph2d::PainterInterface &p, const scene2d::RectF& rect) {}
    virtual void onMouseEvent(Node* node, MouseEvent &evt) {}
    virtual void onKeyEvent(Node* node, KeyEvent &evt) {}
    virtual void onFocusEvent(Node* node, FocusEvent &evt) {}
    virtual void onImeEvent(Node* node, ImeEvent &evt) {}
    virtual void onAnimationFrame(Node* node, absl::Time timestamp) {}
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
    template <typename T>
    void registerControl()
    {
        registerControl(base::string_intern(T::CONTROL_NAME),
            []() -> scene2d::Control* {
                return new T();
            });
    }

private:
    std::map<base::string_atom, ControlFactoryFn> control_factories_;
};

}