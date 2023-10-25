#pragma once
#include "base/string_intern.h"
#include "absl/strings/str_split.h"
#include <set>
#include <vector>

namespace style {

class PseudoClasses {
public:
    inline bool empty() const
    {
        return set_.empty();
    }
    inline size_t size() const
    {
        return set_.size();
    }
    inline void addClass(base::string_atom klass)
    {
        set_.insert(klass);
    }
    void removeClass(base::string_atom klass)
    {
        set_.erase(klass);
    }
    void toggleClass(base::string_atom klass)
    {
        auto it = set_.find(klass);
        if (it != set_.end()) {
            set_.erase(it);
        } else {
            set_.insert(klass);
        }
    }
    bool containsClass(base::string_atom klass) const
    {
        return (set_.find(klass) != set_.end());
    }
    bool containsClass(const PseudoClasses& o) const
    {
        if (this == &o)
            return true;

        for (auto& klass : o.set_) {
            if (set_.find(klass) == set_.end())
                return false;
        }
        return true;
    }
    std::set<base::string_atom>::const_iterator begin() const
    {
        return set_.begin();
    }
    std::set<base::string_atom>::const_iterator end() const
    {
        return set_.end();
    }

private:
    std::set<base::string_atom> set_;

    template <typename Sink>
    friend void AbslStringify(Sink& sink, const PseudoClasses& p) {
        for (auto& atom : p.set_) {
            absl::Format(&sink, ":.%v", atom);
        }
    }
};

}