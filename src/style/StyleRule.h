#pragma once
#include "base/string_intern.h"
#include "StyleValue.h"

namespace style {

enum class SelectorDependency {
    None,
    DirectParent,
    Ancestor,
};

class Selector {
public:
    base::string_atom tag;
    base::string_atom id;
    base::string_atom klass;
    std::unique_ptr<Selector> dep_selector;
    SelectorDependency dep_type = SelectorDependency::None;

    static absl::StatusOr<std::unique_ptr<Selector>> parse(absl::string_view input); 
    static absl::StatusOr<std::vector<std::unique_ptr<Selector>>> parseGroup(absl::string_view input); 
    int specificity() const;
};

class StyleRule {
public:
    std::unique_ptr<Selector> selector;
    StyleSpec spec;

    StyleRule(std::unique_ptr<Selector> sel, const StyleSpec &spe)
        : selector(std::move(sel)), spec(spe) {}
    inline int specificity() const
    {
        return selector->specificity();
    }
};

}