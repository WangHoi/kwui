#include "PaintSurfaceD2D.h"
#include "windows_header.h"

namespace windows
{
namespace graphics
{

void PaintSurfaceD2D::resize(int pixel_width, int pixel_height, float dpi_scale) {

}
std::unique_ptr<graph2d::PainterInterface> PaintSurfaceD2D::beginPaint() {
	return nullptr;
}
bool PaintSurfaceD2D::endPaint() {
	return false;
}

} // namespace graphics

} // namespace windows
