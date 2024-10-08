#pragma once
#include <string>
#include <unordered_set>
#include "absl/strings/string_view.h"
#include "absl/strings/str_format.h"

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
	inline bool operator!=(const string_atom &o) const {
		return !(*this == o);
	}
	inline bool operator==(const char* s) const {
		return (strcmp(atom_text_, s) == 0);
	}
	inline bool operator!=(const char* s) const {
		return !(*this == 0);
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
	template <typename Sink>
	friend void AbslStringify(Sink& sink, const string_atom& p) {
		absl::Format(&sink, "%s", p.atom_text_);
	}
};

inline string_atom string_intern(absl::string_view s)
{
	return string_atom(std::string(s));
}

}

template<>
struct std::hash<base::string_atom>
{
	std::size_t operator()(const base::string_atom& s) const noexcept
	{
		return std::hash<std::string>{}(s.c_str());
	}
};
