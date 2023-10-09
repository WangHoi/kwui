#pragma once
#include <string>
#include <unordered_set>
#include "absl/strings/string_view.h"

namespace base {

class string_atom {
public:
	string_atom()
		: string_atom("") {}
	inline operator const char*() const {
		return atom_text_;
	}
	inline bool operator<(const string_atom &o) const {
		return atom_text_ < o.atom_text_;
	}
	inline bool operator==(const string_atom &o) const {
		return atom_text_ == o.atom_text_;
	}
	inline const char* c_str() const { return atom_text_; }

private:
	string_atom(const std::string& text)
	{
		static std::unordered_set<std::string> tbl;
		auto p = tbl.insert(text);
		atom_text_ = p.first->c_str();
	}

	const char* atom_text_;

	friend string_atom string_intern(absl::string_view s);
};

inline string_atom string_intern(absl::string_view s)
{
	return string_atom(std::string(s));
}

}
