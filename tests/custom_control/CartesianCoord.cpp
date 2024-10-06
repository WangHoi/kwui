//
// Created by wangh on 2024/8/16.
//

#include "CartesianCoord.h"
#include <math.h>

static float lerp(float x, float y, float s)
{
    return x + (y - x) * s;
}

static constexpr float S = 1.1f;

void CartesianCoord::setSceneBounds(float min_x, float min_y, float max_x, float max_y)
{
    scene_min_x_ = min_x;
    scene_min_y_ = min_y;
    scene_max_x_ = max_x;
    scene_max_y_ = max_y;

    if (max_y - min_y > 1e-3f) {
        scene_scale_x_ = scene_scale_y_ = (max_y - min_y) / 13.f;
    }
}

void CartesianCoord::zoomIn()
{
    scale_factor_ += 1;
}

void CartesianCoord::zoomOut()
{
    scale_factor_ -= 1;
}

void CartesianCoord::pan(float scene_dx, float scene_dy)
{
    auto sx = powf(S, scale_factor_)  * scene_scale_x_;
    auto sy = powf(S, scale_factor_)  * scene_scale_y_;
    dx_ += scene_dx / sx;
    dy_ -= scene_dy / sy;
}

std::tuple<float, float> CartesianCoord::mapFromScene(float x, float y) const
{
    return {mapXFromScene(x), mapYFromScene(y)};
}

float CartesianCoord::mapXFromScene(float x) const
{
    auto scale = powf(S, scale_factor_)  * scene_scale_x_;
    return (x - lerp(scene_min_x_, scene_max_x_, mid_factor_)) / scale - dx_;
}

float CartesianCoord::mapYFromScene(float y) const
{
    auto scale = powf(S, scale_factor_)  * scene_scale_y_;
    return -(y - lerp(scene_min_y_, scene_max_y_, mid_factor_)) / scale - dy_;
}

std::tuple<float, float> CartesianCoord::mapToScene(float x, float y) const
{
    return {mapXToScene(x), mapYToScene(y)};
}

float CartesianCoord::mapXToScene(float x) const
{
    auto scale = powf(S, scale_factor_)  * scene_scale_x_;
    return (x + dx_) * scale + lerp(scene_min_x_, scene_max_x_, mid_factor_);
}

float CartesianCoord::mapYToScene(float y) const
{
    auto scale = powf(S, scale_factor_)  * scene_scale_y_;
    return -(y + dy_) * scale + lerp(scene_min_y_, scene_max_y_, mid_factor_);
}

float CartesianCoord::getScaleFactor() const
{
    return scale_factor_;
}
