#pragma once

#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include <vector>

class GrGLCaps;
class GrGLContext;

namespace xskia
{

class ScopedNativeGLStateX
{
public:
    ScopedNativeGLStateX(const GrGLContext&);
    ~ScopedNativeGLStateX();

private:
    const GrGLInterface* gl_;
    const GrGLCaps* caps_;
    GrGLint vao_;
    GrGLint vbo_;
    GrGLint ibo_;
    GrGLint fbo_;
    GrGLint rbo_;
    GrGLint srb_framebuffer_;
    GrGLint viewport_[4];
    GrGLint blend_;
    GrGLint src_blend_func_[2]; // color, alpha
    GrGLint dst_blend_func_[2];
    GrGLint blend_equation_[2];
    GrGLfloat blend_color_[4]; // RGBA
    GrGLint cull_face_;
    GrGLint cull_face_mode_;
    GrGLint front_face_;
    GrGLint depth_test_;
    GrGLint depth_mask_;
    GrGLint stencil_test_;
    GrGLint active_texture_;
    GrGLint texture2d_;
    GrGLint current_program_;

    struct VertexArrayAttrib
    {
        GrGLint buffer_binding;
        GrGLint enabled;
        GrGLint size;
        GrGLint stride;
        GrGLint type;
        GrGLint normalized;
    };
    std::vector<VertexArrayAttrib> vertex_array_attribs_;
};
}
