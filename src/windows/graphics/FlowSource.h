#pragma once
#include "windows/windows_header.h"

class FlowLayoutSource {
public:
    enum FlowShape
    {
        FlowShapeCircle,
        FlowShapeFunnel
    };

    struct RectF
    {
        float left;
        float top;
        float right;
        float bottom;
    };

    FlowLayoutSource()
    :   flowShape_(FlowShapeCircle),
        width_(300),
        height_(300)
    {
        Reset();
    }

    HRESULT Reset();
    HRESULT SetShape(FlowShape flowShape);
    HRESULT SetSize(float width, float height);
    HRESULT GetNextRect(float fontHeight, OUT RectF* nextRect);

protected:
    FlowShape flowShape_;
    float width_;
    float height_;

    float currentY_;
};
