#include "Control.h"

namespace scene2d {
ControlRegistry* ControlRegistry::get()
{
    static ControlRegistry r;
    return &r;
}

void ControlRegistry::registerControl(base::string_atom name, ControlFactoryFn fn)
{
    control_factories_[name] = fn;
}

void ControlRegistry::unregisterControl(base::string_atom name)
{
    control_factories_.erase(name);
}

Control* ControlRegistry::createControl(base::string_atom name)
{
    auto it = control_factories_.find(name);
    if (it == control_factories_.end())
        return nullptr;
    return (it->second)();
}

}