//
// Created by wangh on 2024/8/16.
//

#include "CartesianCoord.h"

static float lerp(float x, float y, float s)
{
    return x + (y - x) * s;
}

void CartesianCoord::setSceneBounds(float min_x, float min_y, float max_x, float max_y)
{
    scene_min_x_ = min_x;
    scene_min_y_ = min_y;
    scene_max_x_ = max_x;
    scene_max_y_ = max_y;
}

void CartesianCoord::setSceneScale(float scale_y)
{
    scene_scale_y_ = scale_y;
    scene_scale_x_ = scale_y;
    /*
    if (scene_max_y_ - scene_min_y_ > 1e-3f) {
        scene_scale_x_ = scale_y * (scene_max_x_ - scene_min_x_) / (scene_max_y_ - scene_min_y_);
        scene_scale_x_ = std::max(scene_max_x_, 0.01f);
    }
    */
}

std::tuple<float, float> CartesianCoord::mapFromScene(float x, float y) const
{
    return {mapXFromScene(x), mapYFromScene(y)};
}

float CartesianCoord::mapXFromScene(float x) const
{
    return (x - lerp(scene_min_x_, scene_max_x_, mid_factor_)) / scene_scale_x_;
}

float CartesianCoord::mapYFromScene(float y) const
{
    return -(y - lerp(scene_min_y_, scene_max_y_, mid_factor_)) / scene_scale_y_;
}

std::tuple<float, float> CartesianCoord::mapToScene(float x, float y) const
{
    return {mapXToScene(x), mapYToScene(y)};
}

float CartesianCoord::mapXToScene(float x) const
{
    return x * scene_scale_x_ + lerp(scene_min_x_, scene_max_x_, mid_factor_);;
}

float CartesianCoord::mapYToScene(float y) const
{
    return -y * scene_scale_y_ + lerp(scene_min_y_, scene_max_y_, mid_factor_);
}
