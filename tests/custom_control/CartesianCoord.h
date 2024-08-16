#pragma once
#include <tuple>

class CartesianCoord {
public:
    void setSceneBounds(float min_x, float min_y, float max_x, float max_y);
    void setSceneScale(float scale_y);
    std::tuple<float, float> mapFromScene(float x, float y) const;
    float mapXFromScene(float x) const;
    float mapYFromScene(float y) const;
    std::tuple<float, float> mapToScene(float x, float y) const;
    float mapXToScene(float x) const;
    float mapYToScene(float y) const;

private:
    float scene_min_x_ = 0, scene_min_y_ = 0;
    float scene_max_x_ = 0, scene_max_y_ = 0;
    float scene_scale_x_ = 10.0f, scene_scale_y_ = 10.0f;
    float mid_factor_ = 0.5f;
};
