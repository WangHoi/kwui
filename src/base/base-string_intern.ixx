module;
#include <string>
#include <unordered_set>
export module base:string_intern;

namespace base {

export class string_atom {
public:
	string_atom()
		: string_atom("") {}
	inline operator const char*() const {
		return atom_text_;
	}

private:
	string_atom(const std::string& text)
	{
		static std::unordered_set<std::string> tbl;
		auto p = tbl.insert(text);
		atom_text_ = p.first->c_str();
	}

	const char* atom_text_;

	friend string_atom string_intern(const std::string& s);
};

export inline string_atom string_intern(const std::string& s)
{
	return string_atom(s);
}

}
