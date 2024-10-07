#include "CustomElementControl.h"
#include "Scene.h"
#include "graph2d/graph2d.h"
#include "api/kwui/CustomElementPaint.h"
#include "api/kwui/CustomElementPaint_private.h"

#if WITH_SKIA
#include <include/gpu/GrBackendSurface.h>
#include <src/gpu/ganesh/gl/GrGLDefines_impl.h>
#include "xskia/PainterX.h"
#endif

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

bool CustomElementControl::hitTest(const scene2d::PointF& pos, int flags) const
{
    return (NODE_FLAG_CLICKABLE | NODE_FLAG_HOVERABLE | NODE_FLAG_SCROLLABLE) & flags;
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
        p.save();
        p.clipRect(rect);
        kwui::CustomElementPaintOption po;
        po.left = rect.left;
        po.top = rect.top;
        po.width = rect.width();
        po.height = rect.height();
        custom_->onPaint(*this, po);
        p.restore();
        tmp_fill_bitmap_ = nullptr;
    }
}

void CustomElementControl::onMouseEvent(Node* node, MouseEvent& evt)
{
    if (custom_) {
        if (evt.cmd == MouseCommand::MOUSE_WHEEL) {
            custom_->onWheel(evt.wheel_delta);
        } else if (evt.cmd == MouseCommand::MOUSE_DOWN) {
            custom_->onMouseDown(evt.button, evt.buttons, evt.pos.x, evt.pos.y);
        } else if (evt.cmd == MouseCommand::MOUSE_MOVE) {
            custom_->onMouseMove(evt.button, evt.buttons, evt.pos.x, evt.pos.y);
        } else if (evt.cmd == MouseCommand::MOUSE_UP) {
            custom_->onMouseUp(evt.button, evt.buttons, evt.pos.x, evt.pos.y);
        }
    }
}

void CustomElementControl::setFillBitmap(void* native_bitmap, float dpi_scale)
{
    if (!cur_painter_)
        return;
#if WITH_SKIA
    // OpenGL
    float pixel_width = cur_rect_.width() * dpi_scale;
    float pixel_height = cur_rect_.height() * dpi_scale;
    GrGLTextureInfo gl_info;
    gl_info.fTarget = GR_GL_TEXTURE_2D;
    gl_info.fID = (uintptr_t)native_bitmap;
    gl_info.fFormat = GR_GL_RGBA8;
    GrBackendTexture tex(pixel_width, pixel_height, GrMipmapped::kNo, gl_info);
    auto xp = (xskia::PainterX*)cur_painter_;
    tmp_fill_bitmap_ = xp->adoptBackendTexture(tex);
#else
#ifdef _WIN32
    do {
        auto srv = static_cast<ID3D11ShaderResourceView*>(native_bitmap);
        if (!srv)
            break;
        ComPtr<ID3D11Resource> tex;
        srv->GetResource(tex.GetAddressOf());
        if (!tex)
            break;
        ComPtr<IDXGISurface> surface;
        (void)tex.As(&surface);
        if (!surface)
            break;
        auto& wp = windows::graphics::PainterImpl::unwrap(*cur_painter_);
        auto bitmap = wp.CreateSharedBitmap(surface.Get(),
                                            dpi_scale,
                                            D2D1_ALPHA_MODE_PREMULTIPLIED);
        if (!bitmap)
            break;
        auto brush = wp.CreateBitmapBrush(bitmap.Get());
        wp.SetBrush(brush);
    } while(false);
#endif
#endif
}

void CustomElementControl::drawRoundedRect(float left, float top, float width, float height, float radius)
{
    if (!cur_painter_)
        return;

#if WITH_SKIA
    // TODO: drawRoundedRect()
    auto rrect = RRectF::fromRectRadius(RectF::fromXYWH(left, top, width, height), CornerRadiusF(radius));
    graph2d::PaintBrush paint;
    paint.setShader(tmp_fill_bitmap_.get());
    cur_painter_->drawRRect(rrect, paint);
#else
#ifdef _WIN32
    auto& wp = windows::graphics::PainterImpl::unwrap(*cur_painter_);
    wp.DrawRoundedRect(left, top, width, height, radius);
#endif
#endif
}

void CustomElementControl::drawPath(const kwui::CustomElementPaintPath& path,
                                    const kwui::CustomElementPaintBrush& brush)
{
    if (!cur_painter_)
        return;
    cur_painter_->drawPath(path.d->path.get(), brush.d->brush);
}

void CustomElementControl::drawText(const std::string& text, float x, float y,
                                    const kwui::CustomElementPaintBrush& brush,
                                    const kwui::CustomElementPaintFont& font)
{
    if (!cur_painter_)
        return;
    auto layout = TextLayoutBuilder(text)
                  .SetFontFamily(font.d->font_name)
                  .FontSize(font.d->point_size)
                  .Build();
    cur_painter_->drawTextLayout({x, y}, *layout, brush.d->brush.color());
}

float CustomElementControl::getDpiScale() const
{
    return cur_painter_ ? cur_painter_->getDpiScale() : 1.0f;
}

void CustomElementControl::writePixels(const void* pixels, size_t src_width, size_t src_height, size_t src_stride,
                                       float dst_x, float dst_y, kwui::ColorType color_type)
{
    if (!cur_painter_)
        return;
    auto bmp = graph2d::createBitmap(pixels, src_width, src_height, src_stride, kwui::COLOR_TYPE_RGBA8888);
    cur_painter_->drawBitmap(bmp.get(),
                             PointF(dst_x, dst_y),
                             DimensionF(src_width, src_height));
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
