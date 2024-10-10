#include "GLStateSaver.h"

#include "src/gpu/ganesh/gl/GrGLContext.h"
#include "src/core/SkLeanWindows.h"
#include "base/log.h"

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

ScopedGLStateSaver::ScopedGLStateSaver(const GrGLContext* ctx)
    : gl_(ctx->glInterface()), caps_(ctx->caps())
{
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
}

ScopedGLStateSaver::~ScopedGLStateSaver()
{
    struct FreeModule
    {
        void operator()(HMODULE m) { (void)FreeLibrary(m); }
    };
    std::unique_ptr<typename std::remove_pointer<HMODULE>::type, FreeModule> module(
        LoadLibraryA("opengl32.dll"));

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
