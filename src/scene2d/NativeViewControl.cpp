#include "NativeViewControl.h"
#include "Scene.h"
#include "api/kwui/ScriptEngine.h"

namespace scene2d::control
{
const char* NativeViewControl::CONTROL_NAME = "native_view";

static kwui::NativeViewHandler* g_native_view_handler = nullptr;

NativeViewControl::NativeViewControl()
{
}

base::string_atom NativeViewControl::name()
{
    return base::string_intern(CONTROL_NAME);
}

void NativeViewControl::onAnimationFrame(Node* node, absl::Time timestamp)
{
    // const std::string id = node->scene()->eventContextId();
    // kwui::ScriptValue arg = kwui::ScriptValue::newObject();
    // arg["id"] = id;
    // arg["timestamp"] = kwui::ScriptValue((double)absl::ToUnixMillis(timestamp));
    // kwui::ScriptEngine::get()->postEvent("dialog:animation-event", arg);
    node->requestAnimationFrame(node);
}

void NativeViewControl::onPaint(graph2d::PainterInterface& p, const scene2d::RectF& rect)
{
    if (g_native_view_handler)
        g_native_view_handler->onPaint(rect.width(), rect.height());
}

void NativeViewControl::setNativeViewHandler(kwui::NativeViewHandler* handler)
{
    g_native_view_handler = handler;
}
}
