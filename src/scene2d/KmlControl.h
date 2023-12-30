#pragma once

#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "style/StyleColor.h"
#include "windows/windows_header.h"

namespace scene2d {

class KmlControl : public scene2d::Control {
public:
    static const char* CONTROL_NAME;
    KmlControl();
    base::string_atom name() override;
    void onAnimationFrame(Node* node, absl::Time timestamp) override;
};

}
