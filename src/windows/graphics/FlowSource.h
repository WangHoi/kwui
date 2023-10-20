#pragma once
#include "windows/windows_header.h"
#include "scene2d/geom_types.h"

class FlowLayoutSourceInterface {
public:
    virtual HRESULT GetNextRect(float fontHeight, OUT scene2d::RectF* nextRect) = 0;
};

class FlowLayoutSource : public FlowLayoutSourceInterface
{
public:
    enum FlowShape
    {
        FlowShapeCircle,
        FlowShapeFunnel
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
    HRESULT GetNextRect(float fontHeight, OUT scene2d::RectF* nextRect) override;

protected:
    FlowShape flowShape_;
    float width_;
    float height_;

    float currentY_;
};
