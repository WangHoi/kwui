#include "CustomElementControl.h"
#include "Scene.h"
#include "graph2d/graph2d.h"
#include "api/kwui/CustomElementPaint.h"
#include "api/kwui/CustomElementPaint_private.h"

namespace scene2d::control
{
static std::unordered_map<base::string_atom, kwui::CustomElementFactoryFn> g_factory_map;

static Control* custom_element_control_factory(base::string_atom name);

CustomElementControl::CustomElementControl(base::string_atom name)
    : name_(name)
{
}

base::string_atom CustomElementControl::name()
{
    return name_;
}

void CustomElementControl::onAttach(scene2d::Node* node)
{
    auto it = g_factory_map.find(name_);
    if (it != g_factory_map.end()) {
        custom_.reset(it->second());
    } else {
        custom_ = nullptr;
    }
    if (custom_)
        node->requestAnimationFrame(node);
}

void CustomElementControl::onDetach(Node* node)
{
    custom_ = nullptr;
}

void CustomElementControl::onAnimationFrame(Node* node, absl::Time timestamp)
{
    // const std::string id = node->scene()->eventContextId();
    // kwui::ScriptValue arg = kwui::ScriptValue::newObject();
    // arg["id"] = id;
    // arg["timestamp"] = kwui::ScriptValue((double)absl::ToUnixMillis(timestamp));
    // kwui::ScriptEngine::get()->postEvent("dialog:animation-event", arg);
    node->requestPaint();
    node->requestAnimationFrame(node);
}

void CustomElementControl::onPaint(graph2d::PaintContextInterface& p, const scene2d::RectF& rect)
{
    if (rect.width() <= 0 || rect.height() <= 0)
        return;

    base::scoped_setter _1(cur_painter_, &p);
    base::scoped_setter _2(cur_rect_, rect);

    if (custom_) {
        kwui::CustomElementPaintOption po;
        po.left = rect.left;
        po.top = rect.top;
        po.width = rect.width();
        po.height = rect.height();
        custom_->onPaint(*this, po);
    }
}

void* CustomElementControl::getNativeBitmap(float& out_pixel_width, float& out_pixel_height)
{
    if (!cur_painter_)
        return nullptr;

    auto& wp = windows::graphics::PainterImpl::unwrap(*cur_painter_);
    if (native_.width != cur_rect_.width()
        || native_.height != cur_rect_.height()
        || native_.dpi_scale != wp.GetDpiScale()) {
        native_.width = cur_rect_.width();
        native_.height = cur_rect_.height();
        native_.dpi_scale = wp.GetDpiScale();
        native_.d2d_bitmap = windows::graphics::GraphicDeviceD2D::instance()
            ->createBitmap(cur_rect_.width() * native_.dpi_scale,
                           cur_rect_.height() * native_.dpi_scale,
                           native_.dpi_scale);
        ComPtr<IDXGISurface> surface;
        native_.d2d_bitmap->GetSurface(surface.GetAddressOf());
        surface.As(&native_.d3d_tex);
    }
    out_pixel_width = native_.width * native_.dpi_scale;
    out_pixel_height = native_.height * native_.dpi_scale;
    return native_.d3d_tex.Get();
}

void CustomElementControl::setFillBitmap(void* native_bitmap)
{
    if (!cur_painter_)
        return;

    if (native_.d3d_tex.Get() == native_bitmap && native_.d2d_bitmap) {
        auto& wp = windows::graphics::PainterImpl::unwrap(*cur_painter_);
        auto brush = wp.CreateBitmapBrush(native_.d2d_bitmap.Get());
        wp.SetBrush(brush);
    }
}

void CustomElementControl::drawRoundedRect(float left, float top, float width, float height, float radius)
{
    if (!cur_painter_)
        return;

    auto& wp = windows::graphics::PainterImpl::unwrap(*cur_painter_);
    wp.DrawRoundedRect(left, top, width, height, radius);
}

void CustomElementControl::drawPath(const kwui::CustomElementPaintPath* path)
{
    if (!cur_painter_)
        return;
    graph2d::PaintBrush brush;
    brush.setStyle(graph2d::PAINT_STYLE_STROKE);
    brush.setColor(style::named_color::blue);
    brush.setStrokeWidth(2.0f);
    cur_painter_->drawPath(path->d->path.get(), brush);
}

ControlFactoryFn CustomElementContrlFactory(base::string_atom name, kwui::CustomElementFactoryFn factory_fn)
{
    g_factory_map[name] = factory_fn;
    return &custom_element_control_factory;
}

Control* custom_element_control_factory(base::string_atom name)
{
    return new CustomElementControl(name);
}
}
