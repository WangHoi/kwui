#include "NativeGLStateX.h"

#include "../../tests/custom_control/glad/gl.h"
#include "src/gpu/ganesh/gl/GrGLContext.h"
#include "src/core/SkLeanWindows.h"
#include "base/log.h"

#ifndef GR_GL_VERTEX_ARRAY_BINDING
#define GR_GL_VERTEX_ARRAY_BINDING 0x85B5
#endif
#ifndef GR_GL_BLEND_EQUATION_ALPHA
#define GR_GL_BLEND_EQUATION_ALPHA 0x883D
#endif
#ifndef GR_GL_BLEND_EQUATION_RGB
#define GR_GL_BLEND_EQUATION_RGB 0x8009
#endif
#ifndef GR_GL_SAMPLER_BINDING
#define GR_GL_SAMPLER_BINDING 0x8919
#endif

#define GL_CALL(IFACE, X)                                       \
    do {                                                        \
        X;                                                      \
        GR_GL_CHECK_ERROR_IMPL(IFACE, X);                       \
    } while (false)

namespace xskia
{
#ifdef _WIN32
typedef void (__stdcall *PFNGLGETVERTEXATTRIBIVPROC)(GrGLuint index, GrGLenum pname, GrGLint* params);

GrGLFuncPtr get_gl_proc(void* ctx, const char name[])
{
    // SkASSERT(wglGetCurrentContext());
    if (GrGLFuncPtr p = (GrGLFuncPtr)GetProcAddress((HMODULE)ctx, name))
        return p;
    if (GrGLFuncPtr p = (GrGLFuncPtr)wglGetProcAddress(name))
        return p;
    return nullptr;
};
#endif

ScopedNativeGLStateX::ScopedNativeGLStateX(const GrGLContext& ctx)
    : gl_(ctx.glInterface()), caps_(ctx.caps())
{
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_VERTEX_ARRAY_BINDING, &vao_));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_ARRAY_BUFFER_BINDING, &vbo_));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_ELEMENT_ARRAY_BUFFER_BINDING, &ibo_));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &fbo_));

    LOG(INFO) << ">--- beginNativeRendering " << vao_ << ", " << (void*)&ctx;

    if (vao_ == 0)
    {
        int kk = 1;
    }

    if (caps_->srgbWriteControl())
    {
        GR_GL_CALL(gl_, GetIntegerv(GR_GL_FRAMEBUFFER_SRGB, &srb_framebuffer_));
        GR_GL_CALL(gl_, Enable(GR_GL_FRAMEBUFFER_SRGB));
    }

    // Viewport
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_VIEWPORT, viewport_));

    // Blend states
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_BLEND, &blend_));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_BLEND_SRC_RGB, &src_blend_func_[0]));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_BLEND_SRC_ALPHA, &src_blend_func_[1]));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_BLEND_DST_RGB, &dst_blend_func_[0]));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_BLEND_DST_ALPHA, &dst_blend_func_[1]));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_BLEND_EQUATION_RGB, &blend_equation_[0]));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_BLEND_EQUATION_ALPHA, &blend_equation_[1]));
    GR_GL_CALL(gl_, GetFloatv(GR_GL_BLEND_COLOR, blend_color_));

    // Cull mode
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_CULL_FACE, &cull_face_));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_CULL_FACE_MODE, &cull_face_mode_));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_FRONT_FACE, &front_face_));

    // Depth
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_DEPTH_TEST, &depth_test_));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_DEPTH_WRITEMASK, &depth_mask_));

    // Stencil
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_STENCIL_TEST, &stencil_test_));

    // Textures
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_ACTIVE_TEXTURE, &active_texture_));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_TEXTURE_BINDING_2D, &texture2d_));
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_SAMPLER_BINDING, &sampler_));

    // Program
    GR_GL_CALL(gl_, GetIntegerv(GR_GL_CURRENT_PROGRAM, &current_program_));

    /*
    struct FreeModule
    {
        void operator()(HMODULE m) { (void)FreeLibrary(m); }
    };
    std::unique_ptr<typename std::remove_pointer<HMODULE>::type, FreeModule> module(
        LoadLibraryA("opengl32.dll"));

    auto fGetVertexAttribi = reinterpret_cast<PFNGLGETVERTEXATTRIBIVPROC>(get_gl_proc(
        module.get(), "glGetVertexAttribiv"));
    if (fGetVertexAttribi)
    {
        int num_vertex_array_attribs = std::min(8, caps_->maxVertexAttributes());
        vertex_array_attribs_.resize(num_vertex_array_attribs);
        for (int i = 0; i < num_vertex_array_attribs; ++i)
        {
            auto& va = vertex_array_attribs_[i];
            GL_CALL(gl_, fGetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &va.buffer_binding));
            GL_CALL(gl_, fGetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_ENABLED, &va.enabled));
            GL_CALL(gl_, fGetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_SIZE, &va.size));
            GL_CALL(gl_, fGetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_STRIDE, &va.stride));
            GL_CALL(gl_, fGetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_TYPE, &va.type));
            GL_CALL(gl_, fGetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &va.normalized));
        }
    }
    else
    {
        LOG(ERROR) << "resolve 'glGetVertexAttribiv' failed.";
    }
    */
}

ScopedNativeGLStateX::~ScopedNativeGLStateX()
{
    LOG(INFO) << "<--- endNativeRendering " << vao_;
    GR_GL_CALL(gl_, BindVertexArray(vao_));
    GR_GL_CALL(gl_, BindBuffer(GR_GL_ARRAY_BUFFER, vbo_));
    GR_GL_CALL(gl_, BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, ibo_));
    GR_GL_CALL(gl_, BindFramebuffer(GR_GL_FRAMEBUFFER, fbo_));

    if (caps_->srgbWriteControl())
    {
        if (srb_framebuffer_)
            GR_GL_CALL(gl_, Enable(GR_GL_FRAMEBUFFER_SRGB));
        else
            GR_GL_CALL(gl_, Disable(GR_GL_FRAMEBUFFER_SRGB));
    }
    // GR_GL_CALL(gl_, BindRenderbuffer(GR_GL_RENDERBUFFER, rbo_));

    GR_GL_CALL(gl_, Viewport(viewport_[0], viewport_[1], viewport_[2], viewport_[3]));
    if (blend_)
    {
        GR_GL_CALL(gl_, Enable(GR_GL_BLEND));
        GR_GL_CALL(gl_, BlendFunc(src_blend_func_[0], dst_blend_func_[0]));
        GR_GL_CALL(gl_, BlendEquation(blend_equation_[0]));
        GR_GL_CALL(gl_, BlendColor(blend_color_[0], blend_color_[1], blend_color_[2], blend_color_[3]));
    }
    else
    {
        GR_GL_CALL(gl_, Disable(GR_GL_BLEND));
    }

    if (cull_face_)
        GR_GL_CALL(gl_, Enable(GR_GL_CULL_FACE));
    else
        GR_GL_CALL(gl_, Disable(GR_GL_CULL_FACE));
    GR_GL_CALL(gl_, CullFace(cull_face_mode_));
    GR_GL_CALL(gl_, FrontFace(front_face_));

    if (depth_test_)
        GR_GL_CALL(gl_, Enable(GR_GL_DEPTH_TEST));
    else
        GR_GL_CALL(gl_, Disable(GR_GL_DEPTH_TEST));
    GR_GL_CALL(gl_, DepthMask(depth_mask_));

    if (stencil_test_)
        GR_GL_CALL(gl_, Enable(GR_GL_STENCIL_TEST));
    else
        GR_GL_CALL(gl_, Disable(GR_GL_STENCIL_TEST));

    GR_GL_CALL(gl_, ActiveTexture(active_texture_));
    GR_GL_CALL(gl_, BindTexture(GR_GL_TEXTURE_2D, texture2d_));
    GR_GL_CALL(gl_, BindSampler(active_texture_ - GR_GL_TEXTURE0, sampler_));

    GR_GL_CALL(gl_, UseProgram(current_program_));

    // struct FreeModule
    // {
    //     void operator()(HMODULE m) { (void)FreeLibrary(m); }
    // };
    // std::unique_ptr<typename std::remove_pointer<HMODULE>::type, FreeModule> module(
    //     LoadLibraryA("opengl32.dll"));

    // auto fSetVertexAttribi = reinterpret_cast<PFNGLGETVERTEXATTRIBIVPROC>(get_gl_proc(module.get(), "glSetVertexAttribiv"));
    // if (fSetVertexAttribi)
    // {
    // for (size_t i = 0; i < vertex_array_attribs_.size(); ++i)
    // {
    //     gl_->fFunctions.fBindVertexArray
    //     auto& va = vertex_array_attribs_[i];
    //     GL_CALL(gl_, fSetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &va.buffer_binding));
    //     GL_CALL(gl_, fGetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_ENABLED, &va.enabled));
    //     GL_CALL(gl_, fGetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_SIZE, &va.size));
    //     GL_CALL(gl_, fGetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_STRIDE, &va.stride));
    //     GL_CALL(gl_, fGetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_TYPE, &va.type));
    //     GL_CALL(gl_, fGetVertexAttribi(0, GR_GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &va.normalized));
    // }
    // }
}
}
