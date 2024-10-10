#pragma once

#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include <vector>

class GrGLCaps;
class GrGLContext;

namespace xskia
{

class ScopedGLStateSaver
{
public:
    ScopedGLStateSaver(const GrGLContext*);
    ~ScopedGLStateSaver();

private:
    const GrGLInterface* gl_;
    const GrGLCaps* caps_;
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
