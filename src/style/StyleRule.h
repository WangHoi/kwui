#pragma once
#include "base/string_intern.h"
#include "StyleValue.h"
#include "StyleClass.h"
#include "StylePseudoClass.h"
#include "absl/strings/str_format.h"

namespace style {

enum class SelectorDependency {
    None,
    DirectParent,
    Ancestor,
    DirectPrecedent,
    Precedent,
};

class Selector {
public:
    base::string_atom tag;
    base::string_atom id;
    Classes klass;
    PseudoClasses pseudo_classes;
    std::unique_ptr<Selector> dep_selector;
    SelectorDependency dep_type = SelectorDependency::None;

    static absl::StatusOr<std::unique_ptr<Selector>> parse(absl::string_view input); 
    static absl::StatusOr<std::vector<std::unique_ptr<Selector>>> parseGroup(absl::string_view input); 
    int specificity() const;

    template <typename Sink>
    friend void AbslStringify(Sink& sink, const Selector& p) {
        if (p.dep_selector) {
            AbslStringify(sink, *p.dep_selector);
            if (p.dep_type == SelectorDependency::DirectParent) {
                absl::Format(&sink, ">");
            } else if (p.dep_type == SelectorDependency::Ancestor) {
                absl::Format(&sink, " ");
            }
        }
        absl::Format(&sink, "%v", p.tag);
        absl::Format(&sink, "%v", p.klass);
        absl::Format(&sink, "%v", p.pseudo_classes);
        if (p.id != base::string_atom()) {
            absl::Format(&sink, "#%v", p.id);
        }
    }
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

absl::StatusOr<std::vector<std::unique_ptr<StyleRule>>> parse_css(absl::string_view input);
absl::StatusOr<StyleSpec> parse_inline_style(absl::string_view input);

}
