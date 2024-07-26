#include "NativeViewControl.h"
#include "Scene.h"
#include "api/kwui/ScriptEngine.h"
#include "graph2d/graph2d.h"

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

void NativeViewControl::onAttach(scene2d::Node* node)
{
    node->requestAnimationFrame(node);
}

void NativeViewControl::onDetach(Node* node)
{
}

void NativeViewControl::onSetAttribute(base::string_atom name, const NodeAttributeValue& value)
{
    if (g_native_view_handler) {
        if (value.isNull()) {
            g_native_view_handler->onSetAttribute(name.c_str(), nullptr);
        } else {
            std::string str = value.toString();
            g_native_view_handler->onSetAttribute(name.c_str(), &str);
        }
    }
}

void NativeViewControl::onAnimationFrame(Node* node, absl::Time timestamp)
{
    // const std::string id = node->scene()->eventContextId();
    // kwui::ScriptValue arg = kwui::ScriptValue::newObject();
    // arg["id"] = id;
    // arg["timestamp"] = kwui::ScriptValue((double)absl::ToUnixMillis(timestamp));
    // kwui::ScriptEngine::get()->postEvent("dialog:animation-event", arg);
    node->requestPaint();
    node->requestAnimationFrame(node);
}

void NativeViewControl::onPaint(graph2d::PainterInterface& p, const scene2d::RectF& rect)
{
    if (rect.width() <= 0 && rect.height() <= 0)
        return;

    if (g_native_view_handler) {
        auto& wp = windows::graphics::PainterImpl::unwrap(p);
        if (native_.width != rect.width() || native_.height != rect.height() || native_.dpi_scale != p.getDpiScale()) {
            native_.width = rect.width();
            native_.height = rect.height();
            native_.dpi_scale = p.getDpiScale();
            native_.d2d_bitmap = windows::graphics::GraphicDeviceD2D::instance()->createBitmap(
                rect.width() * native_.dpi_scale, rect.height() * native_.dpi_scale);
        }
        if (native_.d2d_bitmap) {
            ComPtr<IDXGISurface> surface;
            native_.d2d_bitmap->GetSurface(surface.GetAddressOf());
            ComPtr<ID3D11Texture2D> tex;
            surface.As(&tex);
            if (tex) {
                g_native_view_handler->onPaint(tex.Get(), native_.width, native_.height, native_.dpi_scale);
                wp.DrawBitmap(native_.d2d_bitmap.Get(), rect.origin(), rect.size());
            }
        }
    }
}

void NativeViewControl::setNativeViewHandler(kwui::NativeViewHandler* handler)
{
    g_native_view_handler = handler;
}
}
